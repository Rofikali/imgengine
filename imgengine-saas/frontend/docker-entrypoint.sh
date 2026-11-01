#!/bin/sh
# Keep the Rust API credential server-side. The Nuxt process consumes it from
# a Docker secret at startup; neither the image nor browser runtime config
# contains the value.
set -eu

if [ -n "${IMGENGINE_RENDER_API_KEY_FILE:-}" ]; then
    if [ ! -r "$IMGENGINE_RENDER_API_KEY_FILE" ] || [ ! -s "$IMGENGINE_RENDER_API_KEY_FILE" ]; then
        echo "IMGENGINE_RENDER_API_KEY_FILE must name a readable, non-empty secret" >&2
        exit 64
    fi
    NUXT_IMGENGINE_RENDER_API_KEY="$(cat "$IMGENGINE_RENDER_API_KEY_FILE")"
    export NUXT_IMGENGINE_RENDER_API_KEY
fi

exec node .output/server/index.mjs "$@"
