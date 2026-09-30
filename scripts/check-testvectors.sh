#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
python3 tools/vectors.py
make test-c
