### python logs

    curl -O "https://example.com/image.jpg"    # download test image from internet using curl 


    docker compose logs -f worker
    docker compose logs -f api

### Testing with Curl

    curl -X POST "http://localhost:8000/generate" -H "Content-Type: application/json" -d '{
  "input": "/data/uploads/input.jpg",
  "output": "/data/outputs/out.png",
  "cols": 6,
  "rows": 2
}'

### Standard COMPOSE UP/DOWN ( from anywhere )

    👉 -v ensures:

    DB resets cleanly
    No old broken state

    docker compose -f ./infra/docker-compose.yml up --build -d

    docker compose -f ./infra/docker-compose.yml down -v

    ls -R /workspaces/imgengine/imgengine-saas/
    docker exec -it infra-api-1 uv run python -c "from app.main import app; [print(route.path) for route in app.routes]"

### Restarting

    docker compose restart api

sudo rm -rf /workspaces/imgengine/imgengine-saas/data/outputs/*
sudo rm -rf /workspaces/imgengine/imgengine-saas/data/uploads/*
