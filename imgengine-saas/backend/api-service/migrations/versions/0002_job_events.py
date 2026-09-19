from alembic import op


revision = "0002_job_events"
down_revision = "0001_job_retention"
branch_labels = None
depends_on = None


def upgrade() -> None:
    op.execute(
        """
        CREATE TABLE IF NOT EXISTS job_events (
            id SERIAL PRIMARY KEY,
            job_id VARCHAR NOT NULL,
            trace_id VARCHAR,
            event VARCHAR NOT NULL,
            component VARCHAR NOT NULL,
            level VARCHAR NOT NULL,
            message TEXT NOT NULL,
            details TEXT,
            created_at TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
        """
    )
    op.execute("CREATE INDEX IF NOT EXISTS ix_job_events_job_id ON job_events (job_id)")
    op.execute("CREATE INDEX IF NOT EXISTS ix_job_events_trace_id ON job_events (trace_id)")
    op.execute("CREATE INDEX IF NOT EXISTS ix_job_events_created_at ON job_events (created_at)")


def downgrade() -> None:
    op.execute("DROP TABLE IF EXISTS job_events")
