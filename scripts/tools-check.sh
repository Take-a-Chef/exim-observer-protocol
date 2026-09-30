#!/bin/sh
set -eu
for tool in gcc clang clang-format clang-tidy cppcheck yamllint python3 node npm go; do
  command -v "$tool"
done
clang-format --version | grep 'version 18\.1\.8'
python3 -c 'import yaml; print("PyYAML " + yaml.__version__)'
./node_modules/.bin/prettier --version
./node_modules/.bin/markdownlint-cli2 --version
