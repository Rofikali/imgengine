from celery import Celery

celery = Celery("api", broker="redis://redis:6379/0")
