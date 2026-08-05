# backend/core/security.py 

import hashlib
import hmac

from fastapi import Header, HTTPException, status

from app.core.config import API_KEYS, INTERNAL_API_TOKEN


def api_key_fingerprint(api_key: str) -> str:
    return hashlib.sha256(api_key.encode("utf-8")).hexdigest()


def verify_api_key(x_api_key: str = Header(...)) -> str:
    if not any(hmac.compare_digest(x_api_key, configured_key) for configured_key in API_KEYS):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid API key")
    return api_key_fingerprint(x_api_key)


def verify_internal_token(x_internal_token: str = Header(...)):
    if x_internal_token != INTERNAL_API_TOKEN:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid internal token",
        )
