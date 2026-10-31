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
    return {entry['symbol']: entry['abi'] for entry in data}


def get_exported(libpath):
    if not libpath.exists():
        print(f'Library not found: {libpath}', file=sys.stderr)
        return {}
    out = subprocess.check_output(['nm', '-D', '--defined-only', str(libpath)], text=True)
    syms = {}
    for line in out.splitlines():
        parts = line.strip().split()
        if not parts:
            continue
        name = parts[-1]
        if name.startswith('IMGENGINE_'):
            continue
        if '@@' in name:
            symbol, version = name.rsplit('@@', 1)
        elif '@' in name:
            symbol, version = name.rsplit('@', 1)
        else:
            symbol, version = name, None
        syms[symbol] = version
    return syms


def main():
    parser = argparse.ArgumentParser(description='Verify the exact public libimgengine ABI export set and versions.')
    parser.add_argument('--library', type=Path, default=LIB)
    args = parser.parse_args()
    required = load_required()
    exported = get_exported(args.library)

    missing = sorted(set(required) - set(exported))
    unexpected = sorted(set(exported) - set(required))
    wrong_version = sorted(
        (symbol, required[symbol], exported[symbol])
        for symbol in set(required) & set(exported)
        if exported[symbol] != required[symbol]
    )

    print(f'Checked library: {args.library}')
    print(f'Required symbols: {len(required)}')
    print(f'Exported symbols: {len(exported)}')
    if missing or unexpected or wrong_version:
        if missing:
            print('\nMissing required symbols:')
            for symbol in missing:
                print(' -', symbol)
        if unexpected:
            print('\nUnexpected exported symbols:')
            for symbol in unexpected:
                print(' -', symbol)
        if wrong_version:
            print('\nIncorrect symbol versions:')
            for symbol, expected, actual in wrong_version:
                print(f' - {symbol}: expected {expected}, got {actual}')
        print('\nABI check failed.')
        sys.exit(2)
    print('\nExact public ABI export set and symbol versions verified.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
