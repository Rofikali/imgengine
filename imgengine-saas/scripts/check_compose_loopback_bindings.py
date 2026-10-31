#!/usr/bin/env python3
"""Fail closed if any Docker Compose host-published port escapes loopback."""

import json
import sys
from pathlib import Path


EXPECTED = {
    "api": (8000, 8000),
    "rust-api": (8000, 8001),
    "web": (3000, 3000),
}


def validate_config(config: object) -> list[str]:
    """Return all loopback-policy violations in rendered Compose JSON."""
    if not isinstance(config, dict):
        return ["rendered Compose config must be an object"]

    services = config.get("services")
    if not isinstance(services, dict):
        return ["rendered Compose config has no services object"]

    errors: list[str] = []
    for service, (target_port, published_port) in EXPECTED.items():
        definition = services.get(service)
        ports = definition.get("ports") if isinstance(definition, dict) else None
        expected = [
            port for port in (ports or [])
            if isinstance(port, dict)
            and str(port.get("target", "")) == str(target_port)
            and str(port.get("published", "")) == str(published_port)
        ]
        if not isinstance(ports, list) or len(ports) != 1 or len(expected) != 1:
            errors.append(
                f"{service} must publish only {published_port}:{target_port}"
            )

    # Check every published host port, including optional Compose profiles.
    for service, definition in services.items():
        if not isinstance(definition, dict):
            errors.append(f"{service} service definition must be an object")
            continue
        ports = definition.get("ports", [])
        if not isinstance(ports, list):
            errors.append(f"{service} ports must be a list")
            continue
        for index, port in enumerate(ports):
            if not isinstance(port, dict):
                errors.append(f"{service} port entry {index} must be an object")
                continue
            host_ip = port.get("host_ip")
            if host_ip != "127.0.0.1":
                published = port.get("published", "<unknown>")
                target = port.get("target", "<unknown>")
                errors.append(
                    f"{service} published port {published}->{target} "
                    f"uses host_ip {host_ip!r}; expected '127.0.0.1'"
                )

    return errors


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

    errors = validate_config(config)
    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        return 1

    print("PASS: required API and Nuxt ports match the deployment contract")
    print("PASS: every published host port is bound to 127.0.0.1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
