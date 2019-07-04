# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(libnet VERSION 1.0.1)

# 添加文件
FILE(GLOB all_headers "*.h")
FILE(GLOB all_srcs "*.cc")
SOURCE_GROUP("" FILES ${all_headers} ${all_srcs})
SET(directory "Address" "Sockets" "Reactor" "Notifications")
FOREACH(_dir ${directory})
    FILE(GLOB ${_dir}_headers "${_dir}/*.h")
    FILE(GLOB ${_dir}_srcs "${_dir}/*.cc")
    SOURCE_GROUP("${_dir}" FILES ${${_dir}_headers} ${${_dir}_srcs})
    FOREACH(_file ${${_dir}_headers})
        LIST(APPEND all_headers ${_file})
    ENDFOREACH()
    FOREACH(_file ${${_dir}_srcs})
        LIST(APPEND all_srcs ${_file})
    ENDFOREACH()
ENDFOREACH()
# MESSAGE(${all_headers})
# MESSAGE(${all_srcs})

# 设置
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_SUPPRESS_REGENERATION true)

IF (MSVC)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

IF (CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS(-std=c++11)
ENDIF()

# 第三方类库
INCLUDE_DIRECTORIES(${libnet_SOURCE_DIR})
INCLUDE_DIRECTORIES(${libnet_SOURCE_DIR}/libuv/include)

# 生成静态库
IF (MSVC)
    ADD_LIBRARY(libnet STATIC ${all_headers} ${all_srcs})
ELSE()
    ADD_LIBRARY(net STATIC ${all_headers} ${all_srcs})
ENDIF()