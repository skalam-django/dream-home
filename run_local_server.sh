daphne dreamhome.asgi:application --access-log 1 --ping-interval 5 --ping-timeout 10 --port 8000 --bind localhost -v2
celery -A dreamhome worker -B --loglevel=info
