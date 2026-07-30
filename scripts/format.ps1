#!/usr/bin/env pwsh
# Formats every tracked C++ source file in place, or verifies formatting
# with -Check. Windows counterpart of format.sh; both defer to git for the
# file list so build trees and vcpkg output are never touched.
param([switch]$Check)

$ErrorActionPreference = 'Stop'
Set-Location (git rev-parse --show-toplevel)

$files = @(git ls-files '*.cpp' '*.hpp' '*.cc' '*.h')
if ($files.Count -eq 0) {
  Write-Output 'No tracked C++ sources found.'
  exit 0
}

if ($Check) {
  clang-format --dry-run -Werror @files
} else {
  clang-format -i @files
}
exit $LASTEXITCODE
