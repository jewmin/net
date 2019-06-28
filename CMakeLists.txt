# cmake版本
cmake_minimum_required(VERSION 3.7.2)

# 项目名
project(libnet VERSION 1.0.1)

# Include files
FILE(GLOB all_headers "*.h*")
source_group("include" FILES ${all_headers})

# Source files
FILE(GLOB all_srcs "*.c*")
source_group("src" FILES ${all_srcs})

# 设置
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_SUPPRESS_REGENERATION true)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()

# 第三方类库
include_directories(${libnet_SOURCE_DIR})
include_directories(${libnet_SOURCE_DIR}/libuv/include)

# 生成静态库
if (MSVC)
    add_library(libnet STATIC ${all_headers} ${all_srcs})
else()
    add_library(net STATIC ${all_headers} ${all_srcs})
endif()