import sys
import subprocess
import os
import re
import shutil

def run_cmd(cmd, cwd=None, env=None, check=True):
    try:
        result = subprocess.run(cmd, cwd=cwd, env=env, text=True, capture_output=True)
    except FileNotFoundError:
        if check: sys.exit(1)
        return type('DummyResult', (), {'returncode': 127})()
    if check and result.returncode != 0:
        sys.exit(result.returncode)
    return result

def is_tool(name):
    return shutil.which(name) is not None

def main():
    print("Updating generic shields (with vendor exclusion)...")
    doc_cov = 0.0
    total_decls = 0
    doc_decls = 0
    if os.path.exists("include"):
        for root, _, files in os.walk("include"):
            for file in files:
                if file.endswith(".h"):
                    filepath = os.path.join(root, file)
                    # Note: We actually only scan include/, and we moved stb to vendor/,
                    # so they are automatically excluded from doc coverage now!
                    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                        lines = f.readlines()
                    for i, line in enumerate(lines):
                        if (
                            "_API " in line
                            or line.startswith("void ")
                            or line.startswith("int ")
                            or line.startswith("char ")
                            or line.startswith("ui_error_t ")
                            or line.startswith("bool ")
                            or line.startswith("ui_bool_t ")
                        ):
                            total_decls += 1
                            j = i - 1
                            while j >= 0 and lines[j].strip() == "":
                                j -= 1
                            if j >= 0 and ("*/" in lines[j] or "//" in lines[j]):
                                doc_decls += 1

    if total_decls > 0:
        doc_cov = (doc_decls / total_decls) * 100.0

    test_cov = None
    if os.name != "nt" and is_tool("gcovr") and os.path.exists("build_gcc"):
        res = subprocess.run(
            [
                "gcovr",
                "-r",
                "..",
                ".",
                "--gcov-ignore-parse-errors=negative_hits.warn",
                "--print-summary",
                "-e", ".*vendor.*"
            ],
            cwd="build_gcc",
            capture_output=True,
            text=True,
        )
        if res.returncode == 0:
            match = re.search(r"lines:\s+([0-9.]+)%", res.stdout)
            if match:
                test_cov = float(match.group(1))

    def get_color(pct):
        if pct >= 90: return "brightgreen"
        if pct >= 80: return "green"
        if pct >= 70: return "yellowgreen"
        if pct >= 60: return "yellow"
        return "red"

    doc_color = get_color(doc_cov)
    doc_shield = f"[![Doc Coverage](https://img.shields.io/badge/docs-{doc_cov:.0f}%25-{doc_color}.svg)](#)"

    test_shield = ""
    if test_cov is not None:
        test_color = get_color(test_cov)
        test_shield = f"[![Test Coverage](https://img.shields.io/badge/coverage-{test_cov:.0f}%25-{test_color}.svg)](#)"

    if os.path.exists("README.md"):
        with open("README.md", "r", encoding="utf-8") as f:
            readme = f.read()

        readme = re.sub(r"\[!\[Doc Coverage\]\(.*?\)\]\(.*?\)\n?", "", readme)
        readme = re.sub(r"\[!\[Test Coverage\]\(.*?\)\]\(.*?\)\n?", "", readme)

        license_regex = r"(\[!\[License\].*?\]\(.*?\)\n?)"
        insert_str = r"\1" + doc_shield + "\n"
        if test_shield:
            insert_str += test_shield + "\n"

        readme = re.sub(license_regex, insert_str, readme, count=1)

        with open("README.md", "w", encoding="utf-8") as f:
            f.write(readme)

    print(f"Doc Coverage: {doc_cov:.2f}%")
    if test_cov is not None:
        print(f"Test Coverage: {test_cov:.2f}%")

if __name__ == '__main__':
    main()
