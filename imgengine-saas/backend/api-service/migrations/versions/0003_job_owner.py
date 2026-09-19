from alembic import op


revision = "0003_job_owner"
down_revision = "0002_job_events"
branch_labels = None
depends_on = None


def upgrade() -> None:
    op.execute("ALTER TABLE jobs ADD COLUMN IF NOT EXISTS owner_key_hash VARCHAR")
    op.execute("CREATE INDEX IF NOT EXISTS ix_jobs_owner_key_hash ON jobs (owner_key_hash)")


def downgrade() -> None:
    op.execute("DROP INDEX IF EXISTS ix_jobs_owner_key_hash")
    op.execute("ALTER TABLE jobs DROP COLUMN IF EXISTS owner_key_hash")
