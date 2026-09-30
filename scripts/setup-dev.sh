#!/bin/sh
# Explicit opt-in, networked development tool installation. Never called by make.
set -eu
cd "$(dirname "$0")/.."
python3 -m venv .tools/venv
.tools/venv/bin/python -m pip install -r tools/requirements.txt
npm ci --ignore-scripts --no-audit --no-fund
