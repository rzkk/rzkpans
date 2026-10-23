cmake_minimum_required(VERSION 3.25)



#开发过程有可能多套环境 ， 指定版本
set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE FILEPATH "C++ compiler" FORCE)


# 指定使用 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


project(pans VERSION 0.1.0  LANGUAGES CXX C)



include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

message(STATUS "CMAKE_INSTALL_BINDIR : ${CMAKE_INSTALL_BINDIR}")


option(PANS_BUILD_EXAMPLES "Build examples" ON)
option(PANS_BUILD_TESTS    "Build tests"    ON)  # 是否编译测试程序
option(PANS_BUILD_TOOLS    "Build tools"    ON)  # 是否编译工具程序


if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel) #设置图形界面的选项
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")


include(cmake/pansOptions.cmake)  # 编译参数

add_subdirectory(pans) # 添加子目录

if(PANS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "------------------------")
endif()

if(PANS_BUILD_TOOLS)
    add_subdirectory(tools)
endif()