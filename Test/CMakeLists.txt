# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test VERSION 2.0.1)
# MESSAGE(${PROJECT_NAME} ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_LIST_DIR})

# 设置
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_SUPPRESS_REGENERATION true)

IF (MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-DUSE_VLD)
	ADD_COMPILE_OPTIONS(/utf-8)
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS(-m64)
	ADD_DEFINITIONS(-std=c++11)
ENDIF()

# 第三方类库
IF (MSVC)
	INCLUDE_DIRECTORIES(${test_suite_SOURCE_DIR}/vld/include)
ENDIF()

IF (MSVC)
	LINK_DIRECTORIES(${test_suite_SOURCE_DIR}/vld/lib)
ENDIF()

# 生成可执行文件
ADD_EXECUTABLE(bench_server ProtocolDef.h BenchServer.h BenchServer.cc ServerMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(bench_server libnet-static libuv vld ws2_32 iphlpapi psapi userenv)
ELSE()
	TARGET_LINK_LIBRARIES(bench_server net.a uv)
ENDIF()

ADD_EXECUTABLE(bench_client ProtocolDef.h BenchClient.h BenchClient.cc ClientMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(bench_client libnet-static libuv vld ws2_32 iphlpapi psapi userenv)
ELSE()
	TARGET_LINK_LIBRARIES(bench_client net.a uv)
ENDIF()

ADD_EXECUTABLE(test_dll DllMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(test_dll libnet libuv vld ws2_32 iphlpapi psapi userenv)
ELSE()
	TARGET_LINK_LIBRARIES(test_dll net uv)
ENDIF()
