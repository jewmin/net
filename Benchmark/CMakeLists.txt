# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(benchmark VERSION 1.0.0)

# 设置
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_SUPPRESS_REGENERATION true)

IF (MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-DUSE_VLD)
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS(-std=c++11)
ENDIF()

# 第三方类库
INCLUDE_DIRECTORIES(${libnet_SOURCE_DIR})
INCLUDE_DIRECTORIES(${libnet_SOURCE_DIR}/libuv/include)
IF (MSVC)
	INCLUDE_DIRECTORIES(${test_suite_SOURCE_DIR}/vld/include)
ENDIF()

LINK_DIRECTORIES(${libnet_SOURCE_DIR}/libuv/lib)
IF (MSVC)
	LINK_DIRECTORIES(${test_suite_SOURCE_DIR}/vld/lib)
ENDIF()

# 生成可执行文件
ADD_EXECUTABLE(BenchServer ProtocolDef.h SampleServer.h SampleServer.cc ServerMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(BenchServer libnet libuv advapi32 iphlpapi psapi ws2_32 userenv vld)
ELSE()
	TARGET_LINK_LIBRARIES(BenchServer net uv)
ENDIF()

ADD_EXECUTABLE(BenchClient ProtocolDef.h BenchClient.h BenchClient.cc ClientMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(BenchClient libnet libuv advapi32 iphlpapi psapi ws2_32 userenv vld)
ELSE()
	TARGET_LINK_LIBRARIES(BenchClient net uv)
ENDIF()