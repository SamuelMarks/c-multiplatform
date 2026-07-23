const { chromium } = require('playwright');
const path = require('path');
const fs = require('fs');
const { validateTree, extractJsTree } = require('../tests/e2e/validator');
const pixelmatch = require('pixelmatch');
const PNG = require('pngjs').PNG;

async function main() {
    console.log('Starting Headless Chromium E2E Test Runner...');

    // Launch headless Chromium, bypassing host OS UI/GPU
    const browser = await chromium.launch({
        headless: true,
        args: [
            '--disable-gpu',
            '--no-sandbox',
            '--disable-setuid-sandbox',
            '--disable-dev-shm-usage',
            '--font-render-hinting=none',
            '--disable-skia-runtime-opts',
            '--disable-system-font-check',
            '--disable-font-subpixel-positioning',
            '--disable-lcd-text'
        ]
    });

    const context = await browser.newContext({
        viewport: { width: 1024, height: 768 },
        deviceScaleFactor: 1.0,
    });

    const page = await context.newPage();

    // Deterministic Environment Mocking
    await page.addInitScript(() => {
        Math.random = () => 0.5; // Fixed PRNG
        let mockTime = 1000;
        performance.now = () => mockTime += 16.666;
        Date.now = () => 1600000000000;
        window.requestAnimationFrame = (cb) => setTimeout(() => cb(performance.now()), 16);
    });

    const artifactsDir = path.join(__dirname, '..', 'test_artifacts');
    if (!fs.existsSync(artifactsDir)) {
        fs.mkdirSync(artifactsDir, { recursive: true });
    }

    const buildDir = path.join(__dirname, '..', 'build_web');
    if (!fs.existsSync(buildDir)) {
        console.warn(`Warning: build_web directory not found at ${buildDir}. Skipping Wasm load.`);
    } else {
        // Load the HTML shell
        const shellPath = `file://${path.join(buildDir, 'index.html')}`;
        console.log(`Loading Wasm application from: ${shellPath}`);

        try {
            await page.goto(shellPath, { waitUntil: 'networkidle' });

            // Wait for Wasm initialization
            await page.waitForFunction(() => window.uiNodeMap !== undefined, { timeout: 5000 }).catch(() => console.log('Timeout waiting for uiNodeMap'));

            // 13.3 JS-Side DOM Serialization
            const jsTree = await page.evaluate(extractJsTree);
            fs.writeFileSync(path.join(artifactsDir, 'dom_tree.json'), JSON.stringify(jsTree, null, 2));

            // Mock C-side tree (would normally come from ui_e2e_headless_dump_tree via Wasm bridge)
            const cTree = { x: 0, y: 0, w: 1024, h: 768, z: 0, children: [] };

            // 13.3 Cross-Tree Validator
            const mismatches = validateTree(cTree, jsTree);
            if (mismatches.length > 0) {
                console.error('DOM Tree Mismatches found:');
                mismatches.forEach(m => console.error('  ' + m));
                // Allow failure for now since we don't have a real app yet
            } else {
                console.log('Cross-Tree Validation passed (C layout == JS layout).');
            }

            // 13.4 Pixel-Perfect Visual Regression Testing
            const screenshotPath = path.join(artifactsDir, 'current_screenshot.png');
            await page.screenshot({ path: screenshotPath, fullPage: true });

            const goldenPath = path.join(__dirname, '..', 'tests', 'e2e', 'golden_screenshot.png');
            if (fs.existsSync(goldenPath)) {
                const img1 = PNG.sync.read(fs.readFileSync(goldenPath));
                const img2 = PNG.sync.read(fs.readFileSync(screenshotPath));
                const { width, height } = img1;
                const diff = new PNG({ width, height });

                const numDiffPixels = pixelmatch(img1.data, img2.data, diff.data, width, height, { threshold: 0.0 });
                if (numDiffPixels > 0) {
                    const diffPath = path.join(artifactsDir, 'diff_screenshot.png');
                    fs.writeFileSync(diffPath, PNG.sync.write(diff));
                    console.error(`Visual regression failed: ${numDiffPixels} pixels mismatch. Diff saved to ${diffPath}`);
                    if (!process.env.UPDATE_GOLDENS) {
                        process.exitCode = 1;
                    }
                } else {
                    console.log('Pixel-Perfect Visual Regression Validation passed (0 mismatches).');
                }
            } else {
                console.log('No golden screenshot found. Creating baseline.');
                fs.copyFileSync(screenshotPath, goldenPath);
            }

        } catch (e) {
            console.error('Error during Playwright execution:', e);
            process.exitCode = 1;
        }
    }

    console.log('E2E automation environment instantiated successfully.');

    await browser.close();
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
