#!/bin/bash

set -euo pipefail


BUILD_DIR="build/debug"


clear 


cmake -S . -B "$BUILD_DIR"

# --parallel：启用并行编译
# nproc：获取当前可用的 CPU 逻辑处理器数量
cmake --build "$BUILD_DIR"  --parallel "$(nproc)"

