from alembic import op


revision = "0004_job_idempotency"
down_revision = "0003_job_owner"
branch_labels = None
depends_on = None


def upgrade() -> None:
    op.execute("ALTER TABLE jobs ADD COLUMN IF NOT EXISTS idempotency_key VARCHAR")
    op.execute(
        """
        CREATE UNIQUE INDEX IF NOT EXISTS uq_jobs_owner_idempotency_key
        ON jobs (owner_key_hash, idempotency_key)
        WHERE idempotency_key IS NOT NULL
        """
    )


def downgrade() -> None:
    op.execute("DROP INDEX IF EXISTS uq_jobs_owner_idempotency_key")
    op.execute("ALTER TABLE jobs DROP COLUMN IF EXISTS idempotency_key")
