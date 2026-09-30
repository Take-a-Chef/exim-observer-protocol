#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mode=${1:-check}
c_files=$(find include c fuzz/c -type f \( -name '*.c' -o -name '*.h' -o -name '*.inc' \))
if [ "$mode" = --write ]; then
  ./.tools/bin/golangci-lint fmt ./go
  clang-format -i $c_files
  ./node_modules/.bin/prettier --write '**/*.{md,yaml,yml,json}'
  ./node_modules/.bin/prettier --parser yaml --write .clang-format .clang-tidy
else
  go_diff=$(./.tools/bin/golangci-lint fmt --diff ./go)
  test -z "$go_diff"
  clang-format --dry-run --Werror $c_files
  ./node_modules/.bin/prettier --check '**/*.{md,yaml,yml,json}'
  ./node_modules/.bin/prettier --parser yaml --check .clang-format .clang-tidy
fi
