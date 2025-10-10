FROM ubuntu:25.04

RUN apt-get update && apt-get install -y \
    libgomp1 \
    libxml2 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

CMD ["./build/leiden_test"]
