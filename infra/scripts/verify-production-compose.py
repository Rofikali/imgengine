#!/usr/bin/env python3
"""Fail closed on the IMGENGINE single-host production Compose contract."""

import json
import sys
from pathlib import Path


RUNTIME_SERVICES = ("caddy", "rust-api", "web")


def as_list(value: object) -> list[object]:
    return value if isinstance(value, list) else []


def has_all(values: object, required: set[str]) -> bool:
    return required.issubset({str(value) for value in as_list(values)})


def has_secret_mount(service: object, source: str, target: str) -> bool:
    for entry in as_list(service.get("secrets") if isinstance(service, dict) else None):
        if not isinstance(entry, dict):
            continue
        if (
            entry.get("source") == source
            and entry.get("target") == target
            and entry.get("uid") == "10001"
            and entry.get("gid") == "10001"
            and entry.get("mode") == "0440"
        ):
            return True
    return False


def validate(config: object) -> list[str]:
    if not isinstance(config, dict):
        return ["rendered Compose config must be an object"]
    services = config.get("services")
    if not isinstance(services, dict):
        return ["rendered Compose config has no services object"]

    errors: list[str] = []
    required_services = {"caddy-data-init", *RUNTIME_SERVICES}
    if set(services) != required_services:
        errors.append(f"services must be exactly {sorted(required_services)}")

    data_init = services.get("caddy-data-init")
    if not isinstance(data_init, dict):
        errors.append("caddy-data-init service is missing")
    else:
        image = data_init.get("image")
        if not isinstance(image, str) or "@sha256:" not in image:
            errors.append("caddy-data-init image must be pinned by immutable digest")
        if data_init.get("user") != "0:0":
            errors.append("caddy-data-init must be explicitly root-only for volume ownership setup")
        if data_init.get("read_only") is not True:
            errors.append("caddy-data-init must use a read-only root filesystem")
        if not has_all(data_init.get("cap_drop"), {"ALL"}):
            errors.append("caddy-data-init must drop all Linux capabilities")
        allowed_init_capabilities = {"CHOWN", "DAC_OVERRIDE", "FOWNER"}
        init_capabilities = {str(value) for value in as_list(data_init.get("cap_add"))}
        if init_capabilities != allowed_init_capabilities:
            errors.append("caddy-data-init must add only ownership-setup capabilities")
        init_security_options = {str(value) for value in as_list(data_init.get("security_opt"))}
        if "no-new-privileges:true" not in init_security_options:
            errors.append("caddy-data-init must set no-new-privileges")
        if data_init.get("network_mode") != "none":
            errors.append("caddy-data-init must have no network access")
        if as_list(data_init.get("ports")):
            errors.append("caddy-data-init must not publish a host port")

    for name in RUNTIME_SERVICES:
        service = services.get(name)
        if not isinstance(service, dict):
            errors.append(f"{name} service is missing")
            continue
        image = service.get("image")
        if not isinstance(image, str) or "@sha256:" not in image:
            errors.append(f"{name} image must be pinned by immutable digest")
        if service.get("build") is not None:
            errors.append(f"{name} must not build from source in production Compose")
        if service.get("user") != "10001:10001":
            errors.append(f"{name} must run as UID/GID 10001")
        if service.get("read_only") is not True:
            errors.append(f"{name} must use a read-only root filesystem")
        if not has_all(service.get("cap_drop"), {"ALL"}):
            errors.append(f"{name} must drop all Linux capabilities")
        security_options = {str(value) for value in as_list(service.get("security_opt"))}
        if "no-new-privileges:true" not in security_options:
            errors.append(f"{name} must set no-new-privileges")
        if not service.get("mem_limit") or not service.get("memswap_limit"):
            errors.append(f"{name} must set memory and swap ceilings")
        if not service.get("pids_limit"):
            errors.append(f"{name} must set a PID ceiling")
        if not service.get("tmpfs"):
            errors.append(f"{name} must use bounded tmpfs storage")
        if not service.get("healthcheck"):
            errors.append(f"{name} must define a health check")

    caddy = services.get("caddy", {})
    if isinstance(caddy, dict):
        if not has_all(caddy.get("cap_add"), {"NET_BIND_SERVICE"}):
            errors.append("caddy must add only the privileged-port bind capability")
        ports = as_list(caddy.get("ports"))
        expected_ports = {("80", "80"), ("443", "443")}
        actual_ports = {
            (str(port.get("published")), str(port.get("target")))
            for port in ports
            if isinstance(port, dict)
        }
        if actual_ports != expected_ports or len(ports) != 2:
            errors.append("caddy must be the only public entry point on 80 and 443")

    for name in ("rust-api", "web"):
        service = services.get(name, {})
        if isinstance(service, dict) and as_list(service.get("ports")):
            errors.append(f"{name} must not publish a host port")

    api = services.get("rust-api", {})
    web = services.get("web", {})
    if isinstance(api, dict):
        environment = api.get("environment", {})
        if isinstance(environment, dict) and "IMGENGINE_API_KEYS" in environment:
            errors.append("rust-api must read API keys from a secret file, not an environment value")
        if not isinstance(environment, dict) or environment.get("IMGENGINE_API_KEYS_FILE") != "/run/secrets/imgengine_api_keys":
            errors.append("rust-api must point IMGENGINE_API_KEYS_FILE at its mounted secret")
        if not has_secret_mount(api, "imgengine_api_keys", "imgengine_api_keys"):
            errors.append("rust-api must mount a UID/GID-10001 mode-0440 API-key secret")
    if isinstance(web, dict):
        environment = web.get("environment", {})
        if isinstance(environment, dict) and "NUXT_IMGENGINE_RENDER_API_KEY" in environment:
            errors.append("web must read the render credential from a secret file, not an environment value")
        if not isinstance(environment, dict) or environment.get("IMGENGINE_RENDER_API_KEY_FILE") != "/run/secrets/imgengine_render_api_key":
            errors.append("web must point IMGENGINE_RENDER_API_KEY_FILE at its mounted secret")
        if not has_secret_mount(web, "imgengine_render_api_key", "imgengine_render_api_key"):
            errors.append("web must mount a UID/GID-10001 mode-0440 render-key secret")

    networks = config.get("networks")
    if not isinstance(networks, dict) or networks.get("app", {}).get("internal") is not True:
        errors.append("app network must be internal")
    if isinstance(api, dict) and set(api.get("networks", {})) != {"app"}:
        errors.append("rust-api must attach only to the internal app network")
    if isinstance(web, dict) and set(web.get("networks", {})) != {"edge", "app"}:
        errors.append("web must bridge only the edge and internal app networks")
    if isinstance(caddy, dict) and set(caddy.get("networks", {})) != {"edge"}:
        errors.append("caddy must attach only to the edge network")

    return errors


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} COMPOSE_CONFIG.json", file=sys.stderr)
        return 2
    try:
        # PowerShell commonly writes UTF-8 with a BOM; accept both canonical
        # UTF-8 and BOM-prefixed UTF-8 rendered Compose files.
        config = json.loads(Path(sys.argv[1]).read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"cannot read rendered Compose config: {error}", file=sys.stderr)
        return 2
    errors = validate(config)
    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS: production Compose exposes only Caddy on 80/443")
    print("PASS: runtime images are digest-pinned and hardened")
    print("PASS: application credentials are file-backed Docker secrets")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
