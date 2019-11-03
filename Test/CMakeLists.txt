# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test VERSION 2.1.2)

# 设置
IF (MSVC)
	IF(TCMALLOC)
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCLUDE:__tcmalloc")
		SET(tcLib debug libtcmalloc_minimal-debug optimized libtcmalloc_minimal)
	ELSE()
		SET(tcLib)
	ENDIF()
	IF(VLD)
		ADD_DEFINITIONS(-DUSE_VLD)
	ENDIF()
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()

# 生成可执行文件
ADD_EXECUTABLE(bench_server ProtocolDef.h BenchServer.h BenchServer.cc ServerMain.cc)
ADD_EXECUTABLE(bench_client ProtocolDef.h BenchClient.h BenchClient.cc ClientMain.cc)
ADD_EXECUTABLE(test_dll DllMain.cc)
IF (MSVC)
	TARGET_LINK_LIBRARIES(bench_server libnet-static vld ws2_32 iphlpapi psapi userenv debug libuvd optimized libuv ${tcLib})
	TARGET_LINK_LIBRARIES(bench_client libnet-static vld ws2_32 iphlpapi psapi userenv debug libuvd optimized libuv ${tcLib})
	TARGET_LINK_LIBRARIES(test_dll libnet vld ws2_32 iphlpapi psapi userenv debug libuvd optimized libuv ${tcLib})
	TARGET_COMPILE_DEFINITIONS(test_dll PRIVATE USING_NET_SHARED)
ELSE()
	TARGET_LINK_LIBRARIES(bench_server net-static debug uvd optimized uv)
	TARGET_LINK_LIBRARIES(bench_client net-static debug uvd optimized uv)
	TARGET_LINK_LIBRARIES(test_dll net debug uvd optimized uv)
ENDIF()
