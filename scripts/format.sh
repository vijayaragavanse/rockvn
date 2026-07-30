#!/usr/bin/env bash
# Formats every tracked C++ source file in place, or verifies formatting
# with --check (used by CI). The tracked-file list comes from git so build
# trees and vcpkg output are never touched.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

mapfile -t files < <(git ls-files '*.cpp' '*.hpp' '*.cc' '*.h')
if [[ ${#files[@]} -eq 0 ]]; then
  echo "No tracked C++ sources found."
  exit 0
fi

if [[ "${1:-}" == "--check" ]]; then
  clang-format --dry-run -Werror "${files[@]}"
else
  clang-format -i "${files[@]}"
fi
