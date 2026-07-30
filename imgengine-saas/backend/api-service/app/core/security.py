# backend/core/security.py 

from fastapi import Header, HTTPException, status

from app.core.config import API_KEYS, INTERNAL_API_TOKEN


def verify_api_key(x_api_key: str = Header(...)):
    if x_api_key not in API_KEYS:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid API key")


def verify_internal_token(x_internal_token: str = Header(...)):
    if x_internal_token != INTERNAL_API_TOKEN:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid internal token",
        )
