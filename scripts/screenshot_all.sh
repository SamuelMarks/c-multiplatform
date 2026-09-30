#!/usr/bin/env bash
#
# screenshot_all.sh
# Automated multi-design-system screenshot generator.
# Supports Material 3, Cupertino (iOS HIG), Fluent 2, and future design languages.
# Outputs to ../cc0-assets/c-multiplatform/<design_system>
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
DEFAULT_BASE_DIR="${ROOT_DIR}/../cc0-assets/c-multiplatform"

echo "==> Building screenshot_tool..."
if [ ! -d "${BUILD_DIR}" ]; then
  cmake -B "${BUILD_DIR}" -S "${ROOT_DIR}"
fi
cmake --build "${BUILD_DIR}" --target screenshot_tool

echo "==> Running screenshot_tool..."
"${BUILD_DIR}/bin/screenshot_tool" --output-base "${DEFAULT_BASE_DIR}" "$@"

echo "==> Successfully generated all design system screenshots in: ${DEFAULT_BASE_DIR}"
