# Use an ARMv7l Debian base image
# FROM arm32v7/debian:bullseye

# Use an ARM32 Ubuntu 20.04 base image
# FROM arm32v7/ubuntu:20.04`

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV SONAR_VERSION=5.0.1.3006
ENV AWS_SDK_VERSION=1.11.219
ENV AWS_IOT_SDK_VERSION=v1.20.0

# Install required packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    doxygen \
    graphviz \
    cmake \
    gcc \
    g++ \
    gcc \
    lcov \
    cloc \
    # llvm-12-dev \
    # libclang-12-dev \
    # clang-12 \
    # llvm-dev \
    openjdk-17-jre \
    openjdk-17-jdk \
    liblttng-ust-dev \
    libtool \
    cppcheck \
    python3-pip \
    python-is-python3 \
    valgrind \
    clang-tidy \
    clang-format \
    libgtest-dev \
    gdb-multiarch \
    meson \
    sqlite3 \
    libopencv-dev \
    unzip \
    wget \
    git \
    kmod \
    curl \
    sqlite3 \
    uuid-dev \
    expect \
    ccache \
    jq \
    gettext-base \
    pandoc \
    texlive-latex-base \
    texlive-fonts-recommended \
    texlive-fonts-extra \
    gnumeric && \
    rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip install cpplint flawfinder csv2md strictdoc==0.0.43a1

# Install the AWS Greengrass Development Kit
RUN pip3 install -U git+https://github.com/aws-greengrass/aws-greengrass-gdk-cli.git@v1.3.0

# Download and install Build Wrapper for Sonar
RUN wget -nc https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip -P /tmp && \
    unzip /tmp/build-wrapper-linux-x86.zip -d /tools && \
    rm /tmp/build-wrapper-linux-x86.zip

# Download and install Sonar Scanner
RUN wget -nc https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_VERSION}-linux.zip -P /tmp && \
    unzip /tmp/sonar-scanner-cli-${SONAR_VERSION}-linux.zip -d /tools && \
    rm /tmp/sonar-scanner-cli-${SONAR_VERSION}-linux.zip

# Set the PATH for Sonar Scanner
ENV SONAR_SCANNER_HOME=/tools/sonar-scanner-${SONAR_VERSION}-linux/bin
ENV PATH="${SONAR_SCANNER_HOME}:${PATH}:/tools/build-wrapper-linux-x86:${PATH}"

# Setup git hooks
RUN git config --global include.path ../.gitconfig

# Check and install AWS CLI
RUN if ! command -v aws &>/dev/null; then \
        echo "AWS CLI is not installed. Installing..."; \
        wget -O /tmp/awscli.zip https://awscli.amazonaws.com/awscli-exe-linux-arm.zip && \
        unzip /tmp/awscli.zip -d /tmp && \
        sudo /tmp/aws/install; \
        rm -rf /tmp/awscli.zip /tmp/aws; \
    else \
        echo "AWS CLI is already installed."; \
    fi

# Install AWS SDK for C++
RUN if [ ! -d "/opt/sdk_workspace" ]; then \
        echo "AWS SDK is not installed, installing..."; \
        /setup/install_aws_sdk.sh; \
    else \
        echo "AWS C++ SDK is already installed."; \
    fi

# Create build directory
RUN mkdir -p /build
WORKDIR /build

# Clone and build AWS SDK for C++
RUN git clone --branch ${AWS_SDK_VERSION} https://github.com/aws/aws-sdk-cpp.git /aws-sdk-cpp && \
    cd /aws-sdk-cpp && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS=ON -DBUILD_ONLY=s3 .. && \
    make && make install

# Clone and build AWS IoT Device SDK v2
RUN git clone --branch ${AWS_IOT_SDK_VERSION} https://github.com/aws/aws-iot-device-sdk-cpp-v2.git /aws-iot-sdk && \
    cd /aws-iot-sdk && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make && make install

# Set the parent directory for the source files
WORKDIR /build/..

# Run CMake with the specified toolchain file and enable unity builds
RUN cmake -DCMAKE_TOOLCHAIN_FILE=../scripts/cmake/arm_toolchain.cmake -DCMAKE_UNITY_BUILD=ON ..