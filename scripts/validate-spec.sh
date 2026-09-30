#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
python3 tools/generate.py --validate
python3 -m unittest discover -s tools -p 'test_*.py'
