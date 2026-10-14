FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    build-essential \
    gcc \
    g++ \
    cmake \
    ninja-build \
    cppcheck \
    git \
    python3 \
    python3-venv \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install Conan and CycloneDX Conan in an isolated environment.
RUN python3 -m venv /opt/ci-tools \
    && /opt/ci-tools/bin/pip install --no-cache-dir --upgrade pip \
    && /opt/ci-tools/bin/pip install --no-cache-dir \
       conan cyclonedx-conan

ENV PATH="/opt/ci-tools/bin:${PATH}"

RUN conan profile detect --force

WORKDIR /workspace

CMD ["bash"]
