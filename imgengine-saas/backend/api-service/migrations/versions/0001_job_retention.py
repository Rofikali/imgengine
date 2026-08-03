from alembic import op


revision = "0001_job_retention"
down_revision = None
branch_labels = None
depends_on = None


def upgrade() -> None:
    op.execute(
        """
        CREATE TABLE IF NOT EXISTS jobs (
            id VARCHAR PRIMARY KEY,
            trace_id VARCHAR,
            input VARCHAR NOT NULL,
            output VARCHAR NOT NULL,
            status VARCHAR,
            error TEXT,
            logs TEXT,
            created_at TIMESTAMP WITHOUT TIME ZONE,
            updated_at TIMESTAMP WITHOUT TIME ZONE,
            expires_at TIMESTAMP WITHOUT TIME ZONE
        )
        """
    )
    op.execute("ALTER TABLE jobs ADD COLUMN IF NOT EXISTS expires_at TIMESTAMP WITHOUT TIME ZONE")
    op.execute("CREATE INDEX IF NOT EXISTS ix_jobs_trace_id ON jobs (trace_id)")
    op.execute("CREATE INDEX IF NOT EXISTS ix_jobs_expires_at ON jobs (expires_at)")


def downgrade() -> None:
    op.execute("DROP INDEX IF EXISTS ix_jobs_expires_at")
    op.execute("ALTER TABLE jobs DROP COLUMN IF EXISTS expires_at")
