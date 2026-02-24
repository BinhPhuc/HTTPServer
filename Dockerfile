# Build stage
FROM debian:12-slim AS build

RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  git \
  curl \
  zip \
  unzip \
  tar \
  pkg-config \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN sed -i 's/\r//' script/build.sh script/run.sh script/build_and_run.sh && \
  chmod +x script/build.sh && \
  ./script/build.sh

# Run stage
FROM debian:12-slim AS run

RUN apt-get update && apt-get install -y \
  libstdc++6 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /app/build/server ./server
COPY --from=build /app/public ./public

EXPOSE 8080

CMD ["./server"]
