from pathlib import Path, PurePosixPath

from app.core.config import OUTPUT_PREFIX, STORAGE_ROOT, UPLOAD_PREFIX
from app.core.config import S3_ACCESS_KEY_ID, S3_BUCKET, S3_ENDPOINT_URL, S3_PRESIGN_TTL_SECONDS, S3_REGION, S3_SECRET_ACCESS_KEY, STORAGE_BACKEND


class StoragePathError(ValueError):
    pass


class LocalArtifactStore:
    def __init__(self, root: str):
        self.root = Path(root).resolve()

    def ensure_directories(self) -> None:
        self.path_for(UPLOAD_PREFIX).mkdir(parents=True, exist_ok=True)
        self.path_for(OUTPUT_PREFIX).mkdir(parents=True, exist_ok=True)

    def upload_key(self, job_id: str, filename: str) -> str:
        suffix = Path(filename).suffix.lower()
        if suffix not in {".jpg", ".jpeg", ".png"}:
            raise StoragePathError("Unsupported upload extension")
        return f"{UPLOAD_PREFIX}/{job_id}{suffix}"

    def output_key(self, job_id: str) -> str:
        return f"{OUTPUT_PREFIX}/{job_id}.png"

    def path_for(self, key: str) -> Path:
        candidate = PurePosixPath(key)
        if candidate.is_absolute() or ".." in candidate.parts:
            raise StoragePathError("Invalid storage key")
        path = (self.root / Path(*candidate.parts)).resolve()
        try:
            path.relative_to(self.root)
        except ValueError as exc:
            raise StoragePathError("Invalid storage key") from exc
        return path

    def delete(self, key: str) -> None:
        self.path_for(key).unlink(missing_ok=True)

    def upload(self, key: str) -> None:
        return None

    def download(self, key: str) -> Path:
        return self.path_for(key)

    def download_url(self, key: str) -> str | None:
        return None

    def is_ready(self) -> bool:
        self.ensure_directories()
        return self.path_for(UPLOAD_PREFIX).is_dir() and self.path_for(OUTPUT_PREFIX).is_dir()


class S3ArtifactStore(LocalArtifactStore):
    def __init__(self, root: str):
        super().__init__(root)
        import boto3

        self.client = boto3.client(
            "s3",
            endpoint_url=S3_ENDPOINT_URL,
            aws_access_key_id=S3_ACCESS_KEY_ID,
            aws_secret_access_key=S3_SECRET_ACCESS_KEY,
            region_name=S3_REGION,
        )

    def ensure_directories(self) -> None:
        super().ensure_directories()
        try:
            self.client.head_bucket(Bucket=S3_BUCKET)
        except Exception:
            self.client.create_bucket(Bucket=S3_BUCKET)

    def upload(self, key: str) -> None:
        self.client.upload_file(str(self.path_for(key)), S3_BUCKET, key)

    def download(self, key: str) -> Path:
        path = self.path_for(key)
        path.parent.mkdir(parents=True, exist_ok=True)
        self.client.download_file(S3_BUCKET, key, str(path))
        return path

    def delete(self, key: str) -> None:
        super().delete(key)
        self.client.delete_object(Bucket=S3_BUCKET, Key=key)

    def download_url(self, key: str) -> str | None:
        return self.client.generate_presigned_url(
            "get_object",
            Params={"Bucket": S3_BUCKET, "Key": key},
            ExpiresIn=S3_PRESIGN_TTL_SECONDS,
        )

    def is_ready(self) -> bool:
        self.client.head_bucket(Bucket=S3_BUCKET)
        return True


artifact_store = S3ArtifactStore(STORAGE_ROOT) if STORAGE_BACKEND == "s3" else LocalArtifactStore(STORAGE_ROOT)
