#!/usr/bin/env python3
"""Fail closed if API or Nuxt host ports are published beyond loopback."""

import json
import sys
from pathlib import Path


EXPECTED = {
    "api": (8000, 8000),
    "rust-api": (8000, 8001),
    "web": (3000, 3000),
}


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} COMPOSE_CONFIG.json", file=sys.stderr)
        return 2

    config_path = Path(sys.argv[1])
    try:
        config = json.loads(config_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"cannot read rendered Compose JSON: {exc}", file=sys.stderr)
        return 2

    services = config.get("services")
    if not isinstance(services, dict):
        print("rendered Compose config has no services object", file=sys.stderr)
        return 1

    failed = False
    for service, (target_port, published_port) in EXPECTED.items():
        definition = services.get(service)
        ports = definition.get("ports") if isinstance(definition, dict) else None
        expected = [
            port
            for port in (ports or [])
            if isinstance(port, dict)
            and int(port.get("target", -1)) == target_port
            and str(port.get("published", "")) == str(published_port)
        ]
        if len(ports or []) != 1 or len(expected) != 1:
            print(f"FAIL: {service} must publish only {published_port}:{target_port}", file=sys.stderr)
            failed = True
            continue
        host_ip = expected[0].get("host_ip")
        if host_ip != "127.0.0.1":
            print(f"FAIL: {service} port {published_port} is bound to {host_ip!r}, not 127.0.0.1", file=sys.stderr)
            failed = True
        else:
            print(f"PASS: {service} {published_port}->{target_port} is loopback-only")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
