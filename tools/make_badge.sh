#!/bin/sh
set -eu

if [ "$#" -ne 4 ]; then
  echo "Usage: $0 <label> <value> <color> <output>" >&2
  exit 2
fi

label=$1
value=$2
color=$3
output=$4

label_len=${#label}
value_len=${#value}

label_width=$((label_len * 6 + 10))
value_width=$((value_len * 6 + 10))
total_width=$((label_width + value_width))

label_x=$((label_width / 2))
value_x=$((label_width + (value_width / 2)))

cat > "${output}" <<EOF
<svg xmlns="http://www.w3.org/2000/svg" width="${total_width}" height="20" role="img" aria-label="${label}: ${value}">
  <linearGradient id="s" x2="0" y2="100%">
    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
    <stop offset="1" stop-opacity=".1"/>
  </linearGradient>
  <clipPath id="r">
    <rect width="${total_width}" height="20" rx="3" fill="#fff"/>
  </clipPath>
  <g clip-path="url(#r)">
    <rect width="${label_width}" height="20" fill="#555"/>
    <rect x="${label_width}" width="${value_width}" height="20" fill="#${color}"/>
    <rect width="${total_width}" height="20" fill="url(#s)"/>
  </g>
  <g fill="#fff" text-anchor="middle" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="11">
    <text x="${label_x}" y="14">${label}</text>
    <text x="${value_x}" y="14">${value}</text>
  </g>
</svg>
EOF
