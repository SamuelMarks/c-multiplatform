#!/usr/bin/env python3
"""
Update documentation and test coverage shields in README.md.
"""

import os
import re
import shutil
import subprocess
import sys


def run_cmd(cmd, cwd=None, env=None, check=True):
    try:
        result = subprocess.run(cmd, cwd=cwd, env=env, text=True, capture_output=True)
    except FileNotFoundError:
        if check:
            sys.exit(1)
        return type("DummyResult", (), {"returncode": 127, "stdout": "", "stderr": ""})()
    if check and result.returncode != 0:
        sys.exit(result.returncode)
    return result


def is_tool(name):
    return shutil.which(name) is not None


def get_color(pct):
    if pct >= 90:
        return "brightgreen"
    if pct >= 80:
        return "green"
    if pct >= 70:
        return "yellowgreen"
    if pct >= 60:
        return "yellow"
    return "red"


def get_doc_coverage():
    doc_cov = 0.0
    total_decls = 0
    doc_decls = 0
    if os.path.exists("include"):
        for root, _, files in os.walk("include"):
            for file in sorted(files):
                if file.endswith(".h"):
                    filepath = os.path.join(root, file)
                    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                        lines = f.readlines()
                    for i, line in enumerate(lines):
                        if not line or line[0].isspace():
                            continue
                        s = line.strip()
                        if (
                            not s
                            or s.startswith("#")
                            or s.startswith("/*")
                            or s.startswith("*")
                            or s.startswith("//")
                        ):
                            continue

                        is_decl = False
                        if "_API" in s or "_EXPORT" in s:
                            is_decl = True
                        elif s.startswith("extern ") and "(" in s:
                            is_decl = True
                        elif (
                            s.startswith("void ")
                            or s.startswith("int ")
                            or s.startswith("char ")
                            or s.startswith("ui_error_t ")
                            or s.startswith("bool ")
                            or s.startswith("ui_bool_t ")
                        ) and "(" in s:
                            is_decl = True
                        elif re.match(r"^(?:typedef\s+)?struct\s+[a-zA-Z0-9_]+\s*\{", s):
                            is_decl = True
                        elif re.match(r"^(?:typedef\s+)?enum\s+[a-zA-Z0-9_]+\s*\{", s):
                            is_decl = True

                        if is_decl:
                            total_decls += 1
                            j = i - 1
                            while j >= 0 and lines[j].strip() == "":
                                j -= 1
                            if j >= 0 and ("*/" in lines[j] or "//" in lines[j]):
                                doc_decls += 1

    if total_decls > 0:
        doc_cov = (doc_decls / total_decls) * 100.0
    return doc_cov, total_decls, doc_decls


def get_test_coverage():
    test_cov = None

    if os.name != "nt" and is_tool("gcovr"):
        # Check if build_gcc has .gcda coverage data
        has_gcda = False
        if os.path.exists("build_gcc"):
            for root, _, files in os.walk("build_gcc"):
                if any(f.endswith(".gcda") for f in files):
                    has_gcda = True
                    break

        if not has_gcda and is_tool("cmake") and is_tool("ctest"):
            print("Coverage data not found. Building and running tests with coverage in build_gcc...")
            os.makedirs("build_gcc", exist_ok=True)
            env = os.environ.copy()
            env["CC"] = "gcc"
            run_cmd(
                [
                    "cmake",
                    "..",
                    "-DCMAKE_BUILD_TYPE=Debug",
                    "-DCMAKE_C_FLAGS=--coverage",
                    "-DCMAKE_EXE_LINKER_FLAGS=--coverage",
                    "-DBUILD_TESTING=ON",
                ],
                cwd="build_gcc",
                env=env,
                check=False,
            )
            run_cmd(["cmake", "--build", "."], cwd="build_gcc", env=env, check=False)
            run_cmd(["ctest", "-C", "Debug", "--output-on-failure"], cwd="build_gcc", env=env, check=False)

        if os.path.exists("build_gcc"):
            cmd = [
                "gcovr",
                "-r",
                "..",
                ".",
                "--gcov-ignore-parse-errors=all",
                "--gcov-ignore-errors=all",
                "--print-summary",
                "-e",
                ".*vendor.*",
            ]
            if os.path.exists("gcovr.cfg"):
                cmd.extend(["--config", "../gcovr.cfg"])
            res = subprocess.run(
                cmd,
                cwd="build_gcc",
                capture_output=True,
                text=True,
            )
            if res.returncode == 0:
                match = re.search(r"lines:\s+([0-9.]+)%", res.stdout)
                if match:
                    test_cov = float(match.group(1))

    # If test coverage couldn't be calculated dynamically, preserve existing badge
    if test_cov is None and os.path.exists("README.md"):
        with open("README.md", "r", encoding="utf-8") as f:
            existing = f.read()
        m = re.search(r"coverage-([0-9.]+)%25", existing)
        if m:
            test_cov = float(m.group(1))

    return test_cov


def main():
    print("Updating documentation and test coverage shields...")
    doc_cov, total_decls, doc_decls = get_doc_coverage()
    test_cov = get_test_coverage()

    doc_color = get_color(doc_cov)
    doc_shield = f"[![Doc Coverage](https://img.shields.io/badge/docs-{doc_cov:.0f}%25-{doc_color}.svg)](#)"

    test_shield = ""
    if test_cov is not None:
        test_color = get_color(test_cov)
        test_shield = f"[![Test Coverage](https://img.shields.io/badge/coverage-{test_cov:.0f}%25-{test_color}.svg)](#)"

    if os.path.exists("README.md"):
        with open("README.md", "r", encoding="utf-8") as f:
            readme = f.read()

        readme = re.sub(
            r"\[?!\[doc[ _]?coverage\]\(.*?\)\]?(?:\(.*?\))?\n?",
            "",
            readme,
            flags=re.IGNORECASE,
        )
        readme = re.sub(
            r"\[?!\[test[ _]?coverage\]\(.*?\)\]?(?:\(.*?\))?\n?",
            "",
            readme,
            flags=re.IGNORECASE,
        )

        license_regex = r"(\[!\[License\].*?\]\(.*?\)\n?)"
        insert_str = r"\1" + doc_shield + "\n"
        if test_shield:
            insert_str += test_shield + "\n"

        readme = re.sub(license_regex, insert_str, readme, count=1)

        with open("README.md", "w", encoding="utf-8") as f:
            f.write(readme)

    print(f"Doc Coverage: {doc_cov:.2f}% ({doc_decls}/{total_decls})")
    if test_cov is not None:
        print(f"Test Coverage: {test_cov:.2f}%")
    else:
        print("Test Coverage: N/A")


if __name__ == "__main__":
    main()
