FROM ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive

# Install necessary packages:
# - build-essential: For basic compilation tools (g++, make).
# - cmake: For configuring the build system.
# - git: for cloning repositories if need to fetch dependencies dynamically.
# - libglfw3-dev: Development files for GLFW, required for window and input handling.
# - libgl-dev: Development files for OpenGL, providing the necessary headers and libraries.
# - libcurl4-openssl-dev: Development files for cURL, used in the Linux path of your code.
# - libxkbcommon-dev, libxinerama-dev, libxcursor-dev, libxi-dev, libxext-dev: is for GLFW
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    curl \
    cmake \
    git \
    libglfw3-dev \
    libgl-dev \
    libcurl4-openssl-dev \
    pkg-config \
    libxkbcommon-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxext-dev \
    && rm -rf /var/lib/apt/lists/* # Clean up apt cache to reduce image size

# Create a non-root user (same UID and GID)
ARG USER=dev
ARG UID=1000
ARG GID=1000


RUN groupadd -g ${GID} ${USER} && \
    useradd -m -u ${UID} -g ${GID} -s /bin/bash ${USER} && \
    usermod -aG sudo ${USER} && \
    echo "${USER} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER ${USER}
WORKDIR /home/${USER}/app
