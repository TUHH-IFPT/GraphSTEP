#!/bin/bash

# Install required packages
DEBIAN_FRONTEND=noninteractive sudo apt update -y
DEBIAN_FRONTEND=noninteractive sudo apt install -y libssl-dev libcurl4-openssl-dev libeigen3-dev zip libyaml-cpp-dev libspdlog-dev

mkdir -p external/stepcode/build
pushd external/stepcode/build
    cmake .. -DSC_BUILD_SCHEMAS=ap242 -DSC_BUILD_TYPE=debug
    make -j$(nproc)
popd