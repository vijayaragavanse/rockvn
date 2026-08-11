# syntax=docker/dockerfile:1
# Build stage: toolchain + vcpkg pinned to the manifest baseline. The cache
# mount keeps vcpkg's binary cache across builds so dependency compiles are
# paid once per baseline change, not once per image build.
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build git curl ca-certificates zip unzip tar pkg-config \
    && rm -rf /var/lib/apt/lists/*

ARG VCPKG_COMMIT=659c7023698fc73894a0a296cc89a0143da912bb
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && git -C /opt/vcpkg checkout ${VCPKG_COMMIT} \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics
ENV VCPKG_ROOT=/opt/vcpkg

WORKDIR /src
COPY . .
RUN --mount=type=cache,target=/cache/vcpkg \
    export VCPKG_DEFAULT_BINARY_CACHE=/cache/vcpkg \
    && cmake --preset linux-release \
    && cmake --build --preset linux-release --target user_service

# Runtime stage: the binary, a non-root user, and curl for the healthcheck.
# vcpkg's Linux triplet links dependencies statically, so no library tree
# needs copying — only glibc/libstdc++ from the matching base image.
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends curl ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --system --uid 10001 --no-create-home rockvn

COPY --from=build /src/build/linux-release/services/user/user_service /usr/local/bin/user_service

USER 10001
ENV USER_SERVICE_LOG_FORMAT=json
EXPOSE 8080
HEALTHCHECK --interval=10s --timeout=2s --start-period=5s \
  CMD curl -fsS http://127.0.0.1:8080/health || exit 1
ENTRYPOINT ["/usr/local/bin/user_service"]
