#!/bin/sh
set -eu

script_dir=$(cd "$(dirname "$0")" && pwd)
root_dir=$(cd "${script_dir}/.." && pwd)
build_dir="${CMP_COVERAGE_BUILD_DIR:-${root_dir}/build-coverage}"
badge_dir="${root_dir}/docs/badges"
coverage_xml="${build_dir}/coverage.xml"

mkdir -p "${badge_dir}"

cmake -S "${root_dir}" -B "${build_dir}" \
  -DCMP_ENABLE_COVERAGE=ON \
  -DCMP_REQUIRE_DOXYGEN=ON

cmake --build "${build_dir}"
cmake --build "${build_dir}" --target cmp_coverage
cmake --build "${build_dir}" --target cmp_docs

if [ ! -f "${coverage_xml}" ]; then
  echo "coverage.xml not found at ${coverage_xml}" >&2
  exit 1
fi

line_rate=$(sed -n 's/.*line-rate="\([^"]*\)".*/\1/p' "${coverage_xml}" | head -n 1)
if [ -z "${line_rate}" ]; then
  echo "Unable to parse line-rate from ${coverage_xml}" >&2
  exit 1
fi

line_pct=$(awk -v rate="${line_rate}" 'BEGIN { printf "%.0f", rate * 100 }')

if [ "${line_pct}" -lt 100 ]; then
  echo "Test coverage is below 100% (${line_pct}%). Cannot commit." >&2
  exit 1
fi

color="e05d44"
if [ "${line_pct}" -ge 100 ]; then
  color="4c1"
elif [ "${line_pct}" -ge 90 ]; then
  color="97ca00"
elif [ "${line_pct}" -ge 80 ]; then
  color="dfb317"
fi

"${script_dir}/make_badge.sh" \
  "test coverage" \
  "${line_pct}%" \
  "${color}" \
  "${badge_dir}/test-coverage.svg"

doxy_warn="${build_dir}/doxygen_warnings.log"
if [ -f "${doxy_warn}" ] && [ -s "${doxy_warn}" ]; then
  echo "Doxygen warnings found. Doc coverage is below 100%. Cannot commit." >&2
  cat "${doxy_warn}" >&2
  exit 1
fi

"${script_dir}/make_badge.sh" \
  "doc coverage" \
  "100%" \
  "4c1" \
  "${badge_dir}/doc-coverage.svg"
