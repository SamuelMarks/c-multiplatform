#!/usr/bin/env bash
#
# screenshot_material3.sh
# Automated screenshot generator for all Material 3 components.
# Outputs to ../cc0-assets/c-multiplatform/material3
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
TARGET_OUTPUT_DIR="${1:-${ROOT_DIR}/../cc0-assets/c-multiplatform/material3}"

echo "==> Building screenshot_material3 tool..."
if [ ! -d "${BUILD_DIR}" ]; then
  cmake -B "${BUILD_DIR}" -S "${ROOT_DIR}"
fi
cmake --build "${BUILD_DIR}" --target screenshot_material3

echo "==> Ensuring target output directory exists: ${TARGET_OUTPUT_DIR}"
mkdir -p "${TARGET_OUTPUT_DIR}"
mkdir -p "${TARGET_OUTPUT_DIR}/2x"

echo "==> Generating 1x screenshots for all Material 3 components..."
"${BUILD_DIR}/bin/screenshot_material3" --output-dir "${TARGET_OUTPUT_DIR}" --dpi-scale 1.0

echo "==> Generating 2x Retina screenshots for all Material 3 components..."
"${BUILD_DIR}/bin/screenshot_material3" --output-dir "${TARGET_OUTPUT_DIR}/2x" --dpi-scale 2.0

echo "==> Successfully generated Material 3 screenshots in: ${TARGET_OUTPUT_DIR}"
