# backend/api-service/app/core/tracing.py

from opentelemetry import trace
from opentelemetry.sdk.trace import TracerProvider
from opentelemetry.sdk.trace.export import BatchSpanProcessor
from opentelemetry.exporter.otlp.proto.http.trace_exporter import OTLPSpanExporter

trace.set_tracer_provider(TracerProvider())

span_processor = BatchSpanProcessor(
    OTLPSpanExporter(endpoint="http://jaeger:4318/v1/traces")
)

trace.get_tracer_provider().add_span_processor(span_processor)

tracer = trace.get_tracer(__name__)
