#!/usr/bin/env python3
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD_DIR = ROOT / 'imgengine' / 'build'
LIB = BUILD_DIR / 'libimgengine.so'
JSON = ROOT / 'docs' / 'abi' / 'exported_symbols.json'

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
        name = parts[-1]
        syms.add(name)
    return syms

def main():
    required = load_required()
    exported = get_exported(LIB)

    missing = sorted(required - exported)
    extra = sorted(exported & set(required))

    print(f'Checked library: {LIB}')
    print(f'Required symbols: {len(required)}')
    print(f'Exported matching required: {len(extra)}')
    if missing:
        print('\nMissing required symbols:')
        for s in missing:
            print(' -', s)
        print('\nABI check failed (missing symbols).')
        sys.exit(2)
    print('\nAll required symbols are present.')
    return 0

if __name__ == '__main__':
    sys.exit(main())
