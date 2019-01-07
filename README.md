Website screenshot service. Written in [ReasonML](https://reasonml.github.io/).

Running order:

1. Set `CACHE_TTL` and `CACHE_MAX_SIZE` in .env. Use .env-sample as example.
1. Use `docker run -d -e WATCH=false --name chromeheadless --rm -p 8836:80 playfulcorgi/chromeheadless` to run the service.