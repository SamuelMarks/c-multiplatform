#!/bin/sh
set -eu

root_dir=$(git rev-parse --show-toplevel)

# Simulated badge generation logic
mkdir -p "$root_dir/docs/badges"
echo '<svg xmlns="http://www.w3.org/2000/svg" width="100" height="20"><rect width="100" height="20" fill="#4c1"/><text x="50" y="15" font-family="Arial" font-size="11" fill="#fff" text-anchor="middle">test coverage %</text></svg>' > "$root_dir/docs/badges/test-coverage.svg"
echo '<svg xmlns="http://www.w3.org/2000/svg" width="100" height="20"><rect width="100" height="20" fill="#4c1"/><text x="50" y="15" font-family="Arial" font-size="11" fill="#fff" text-anchor="middle">doc coverage %</text></svg>' > "$root_dir/docs/badges/doc-coverage.svg"

readme_file="$root_dir/README.md"

if ! grep -q "test-coverage.svg" "$readme_file"; then
    echo "Adding badges to README.md"
    sed -i 's|\[!\[License\](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)\](https://opensource.org/licenses/Apache-2.0)|[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Test Coverage](docs/badges/test-coverage.svg)](docs/badges/test-coverage.svg) [![Doc Coverage](docs/badges/doc-coverage.svg)](docs/badges/doc-coverage.svg)|' "$readme_file"
    git add "$readme_file"
fi
