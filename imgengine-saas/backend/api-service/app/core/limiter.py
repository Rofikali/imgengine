# backend/core/limiter.py


import hashlib

from fastapi import Request
from slowapi import Limiter
from slowapi.util import get_remote_address


def rate_limit_key(request: Request) -> str:
    api_key = request.headers.get("X-API-Key")
    if not api_key:
        return get_remote_address(request)
    return hashlib.sha256(api_key.encode("utf-8")).hexdigest()


limiter = Limiter(key_func=rate_limit_key)
