const fs = require('fs');
const path = require('path');

function validateTree(cTree, jsTree) {
    let mismatches = [];

    function compareNodes(cNode, jsNode, path = 'root') {
        if (!cNode && !jsNode) return;
        if (!cNode || !jsNode) {
            mismatches.push(`${path}: Node existence mismatch (C: ${!!cNode}, JS: ${!!jsNode})`);
            return;
        }

        // Compare bounds (allow 1px rounding difference)
        const tol = 1.0;
        if (Math.abs(cNode.x - jsNode.x) > tol ||
            Math.abs(cNode.y - jsNode.y) > tol ||
            Math.abs(cNode.w - jsNode.w) > tol ||
            Math.abs(cNode.h - jsNode.h) > tol) {
            mismatches.push(`${path}: Bounds mismatch. C:[${cNode.x},${cNode.y},${cNode.w},${cNode.h}] JS:[${jsNode.x},${jsNode.y},${jsNode.w},${jsNode.h}]`);
        }

        const cChildren = cNode.children || [];
        const jsChildren = jsNode.children || [];

        if (cChildren.length !== jsChildren.length) {
            mismatches.push(`${path}: Children count mismatch (C: ${cChildren.length}, JS: ${jsChildren.length})`);
        }

        const minLen = Math.min(cChildren.length, jsChildren.length);
        for (let i = 0; i < minLen; i++) {
            compareNodes(cChildren[i], jsChildren[i], `${path}.children[${i}]`);
        }
    }

    compareNodes(cTree, jsTree);
    return mismatches;
}

function extractJsTree() {
    // This code gets evaluated in the browser context via Playwright
    function traverse(element) {
        const rect = element.getBoundingClientRect();
        const node = {
            x: rect.x,
            y: rect.y,
            w: rect.width,
            h: rect.height,
            tag: element.tagName.toLowerCase(),
            children: []
        };
        for (let child of element.children) {
            node.children.push(traverse(child));
        }
        return node;
    }
    return traverse(document.getElementById('app-root') || document.body);
}

module.exports = {
    validateTree,
    extractJsTree
};
