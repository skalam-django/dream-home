web: daphne dreamhome.asgi:application --access-log 1 --ping-interval 5 --ping-timeout 10 --port $PORT --bind 0.0.0.0 -v2
celeryworker: celery -A dreamhome worker -B --loglevel=info