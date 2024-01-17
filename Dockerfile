# Based on https://github.com/devcontainers/images/blob/main/src/cpp/.devcontainer/Dockerfile

FROM gcc:12 as build

# Install cmake and ninja
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    cmake \
    git \
    ninja-build \
    zip \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Add non-root user
RUN useradd -ms /bin/bash rfsc

# Switch to non-root user
USER rfsc

# Create vcpkg source directory
WORKDIR /opt/vcpkg
RUN git clone --depth=1 https://github.com/Microsoft/vcpkg.git .

# Install vcpkg
RUN ./bootstrap-vcpkg.sh

# Configure vcpkg
ENV VCPKG_ROOT=/opt/vcpkg \
    VCPKG_DOWNLOADS=/opt/vcpkg/vcpkg-downloads

ENV PATH="${PATH}:${VCPKG_ROOT}"

# Create source directory
WORKDIR /usr/src/mori_echo
RUN chown rfsc:rfsc /usr/src/mori_echo

# Install dependencies
RUN mkdir build/
COPY ./vcpkg.json .
RUN vcpkg install --x-install-root build/vcpkg_installed

# Copy source
COPY . /usr/src/mori_echo

# Build mori_echo
RUN cmake -B build/ --preset release
RUN cmake --build build/

# Run tests
RUN ctest --preset tests -R business_rules
RUN ctest --preset tests -R concurrency
RUN ctest --preset tests -R cipher

FROM gcc:12

LABEL org.opencontainers.image.source=https://github.com/rfsc-mori/mori_echo

WORKDIR /opt/mori_echo

COPY --from=build /usr/src/mori_echo/build/server/mori_echo_server .

# Add non-root user
RUN useradd -ms /bin/bash rfsc

# Switch to non-root user
USER rfsc

ENTRYPOINT [ "./mori_echo_server" ]
