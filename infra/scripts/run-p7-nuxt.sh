#!/usr/bin/env bash
set -euo pipefail

# The named volume masks any Windows node_modules directory. `pnpm install` is only
# needed when the Linux dependency tree is absent; remove the volume for a
# deliberately fresh dependency installation.
corepack enable
if [[ ! -x node_modules/.bin/nuxt ]]; then
  # Foreground lifecycle scripts avoid concurrent binary validation races on a
  # freshly-created Docker Desktop named volume (observed as esbuild ETXTBSY).
  pnpm install --frozen-lockfile --reporter=append-only
fi

exec pnpm run dev -- --host 0.0.0.0 --port 3000
