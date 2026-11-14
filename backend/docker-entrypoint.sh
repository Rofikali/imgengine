#!/bin/sh
# Load the API-key set from a Docker secret at process start without printing
# its value or baking it into the image. The Rust API intentionally keeps the
# existing IMGENGINE_API_KEYS configuration contract.
set -eu

if [ -n "${IMGENGINE_API_KEYS_FILE:-}" ]; then
    if [ ! -r "$IMGENGINE_API_KEYS_FILE" ] || [ ! -s "$IMGENGINE_API_KEYS_FILE" ]; then
        echo "IMGENGINE_API_KEYS_FILE must name a readable, non-empty secret" >&2
        exit 64
    fi
    IMGENGINE_API_KEYS="$(cat "$IMGENGINE_API_KEYS_FILE")"
    export IMGENGINE_API_KEYS
fi

exec /usr/local/bin/imgengine-api-server "$@"
