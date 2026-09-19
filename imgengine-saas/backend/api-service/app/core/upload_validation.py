JPEG_SIGNATURE = b"\xff\xd8\xff"
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def detect_image_content_type(payload: bytes) -> str | None:
    if payload.startswith(JPEG_SIGNATURE):
        return "image/jpeg"
    if payload.startswith(PNG_SIGNATURE):
        return "image/png"
    return None


def extension_for_content_type(content_type: str) -> str:
    return ".jpg" if content_type == "image/jpeg" else ".png"
