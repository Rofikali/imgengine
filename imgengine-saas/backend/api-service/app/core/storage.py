from pathlib import Path, PurePosixPath

from app.core.config import OUTPUT_PREFIX, STORAGE_ROOT, UPLOAD_PREFIX


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


artifact_store = LocalArtifactStore(STORAGE_ROOT)
