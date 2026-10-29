#!/bin/bash
# 设置遇到错误立即退出，避免编译失败后仍然强行运行测试或程序
#它表示只要脚本中出现任何非零返回值的命令（例如 cmake --build 编译报错，或者 ctest 测试不通过），脚本就会立刻停止运行，不会去执行后面的旧程序。
# -e：命令失败时通常立即退出
# -u：使用未定义的变量时退出
# -o pipefail：管道中任何命令失败都视为失败
set -euo pipefail

# 定义变量，方便统一修改
BUILD_DIR="build/release"


clear 

# rm -rf $BUILD_DIR 


# -S .   指定源码目录
# -B     指定构建目录
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

# --parallel：启用并行编译
# nproc：获取当前可用的 CPU 逻辑处理器数量
cmake --build "$BUILD_DIR"  --parallel "$(nproc)"



