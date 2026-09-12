#!/usr/bin/env python3
import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

ENGINE_ROOT = Path(__file__).resolve().parents[1]
BUILD_DIR = Path(os.environ.get('IMGENGINE_BUILD_DIR', ENGINE_ROOT / 'build'))
LIB = BUILD_DIR / 'libimgengine.so'
JSON = ENGINE_ROOT / 'abi' / 'exported_symbols.json'

def load_required():
    data = json.loads(JSON.read_text())
    return {entry['symbol'] for entry in data}

def get_exported(libpath):
    if not libpath.exists():
        print(f'Library not found: {libpath}', file=sys.stderr)
        return set()
    out = subprocess.check_output(['nm', '-D', '--defined-only', str(libpath)], text=True)
    syms = set()
    for line in out.splitlines():
        parts = line.strip().split()
        if not parts:
            continue
        # symbol name is typically last column
        name = parts[-1].split('@@', 1)[0]
        if name == 'IMGENGINE_1.0':
            continue
        syms.add(name)
    return syms

def main():
    parser = argparse.ArgumentParser(description='Verify the exact public libimgengine ABI export set.')
    parser.add_argument('--library', type=Path, default=LIB)
    args = parser.parse_args()
    required = load_required()
    exported = get_exported(args.library)

    missing = sorted(required - exported)
    unexpected = sorted(exported - required)

    print(f'Checked library: {args.library}')
    print(f'Required symbols: {len(required)}')
    print(f'Exported symbols: {len(exported)}')
    if missing or unexpected:
        print('\nMissing required symbols:')
        for s in missing:
            print(' -', s)
        print('\nUnexpected exported symbols:')
        for s in unexpected:
            print(' -', s)
        print('\nABI check failed.')
        sys.exit(2)
    print('\nExact public ABI export set verified.')
    return 0

if __name__ == '__main__':
    sys.exit(main())
