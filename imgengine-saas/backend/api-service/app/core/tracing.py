# backend/api-service/app/core/tracing.py

from opentelemetry import trace
from opentelemetry.sdk.trace import TracerProvider
from opentelemetry.sdk.trace.export import BatchSpanProcessor
from opentelemetry.exporter.otlp.proto.http.trace_exporter import OTLPSpanExporter
from app.core.config import OTEL_EXPORTER_OTLP_TRACES_ENDPOINT

trace.set_tracer_provider(TracerProvider())

if OTEL_EXPORTER_OTLP_TRACES_ENDPOINT:
    span_processor = BatchSpanProcessor(
        OTLPSpanExporter(endpoint=OTEL_EXPORTER_OTLP_TRACES_ENDPOINT)
    )
    trace.get_tracer_provider().add_span_processor(span_processor)

tracer = trace.get_tracer(__name__)
