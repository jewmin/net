# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test VERSION 2.0.1)

# 设置
IF (MSVC)
	ADD_DEFINITIONS(-DUSE_VLD)
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()

# 第三方类库
IF (MSVC)
	INCLUDE_DIRECTORIES(${test_suite_SOURCE_DIR}/vld/include)
ENDIF()

IF (MSVC)
	LINK_DIRECTORIES(${test_suite_SOURCE_DIR}/vld/lib)
ELSE()
    LINK_DIRECTORIES(${OUTPUT_DIRECTORY})
ENDIF()

# 生成可执行文件
ADD_EXECUTABLE(bench_server ProtocolDef.h BenchServer.h BenchServer.cc ServerMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(bench_server libnet-static vld ws2_32 iphlpapi psapi userenv debug libuvd optimized libuv)
ELSE()
	TARGET_LINK_LIBRARIES(bench_server debug netd.a optimized net.a uv)
ENDIF()

ADD_EXECUTABLE(bench_client ProtocolDef.h BenchClient.h BenchClient.cc ClientMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(bench_client libnet-static vld ws2_32 iphlpapi psapi userenv debug libuvd optimized libuv)
ELSE()
	TARGET_LINK_LIBRARIES(bench_client debug netd.a optimized net.a uv)
ENDIF()

ADD_EXECUTABLE(test_dll DllMain.cc)
IF (MSVC)
	TARGET_COMPILE_DEFINITIONS(test_dll PRIVATE USING_NET_SHARED)
	TARGET_LINK_LIBRARIES(test_dll libnet vld ws2_32 iphlpapi psapi userenv debug libuvd optimized libuv)
ELSE()
	TARGET_LINK_LIBRARIES(test_dll net uv)
ENDIF()
