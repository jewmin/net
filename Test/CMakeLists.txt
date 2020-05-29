# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test VERSION 3.3.0)

# 生成可执行文件
ADD_EXECUTABLE(bench_server ProtocolDef.h BenchServer.h BenchServer.cc BenchCommon.h BenchCommon.cc ServerMain.cc)
ADD_EXECUTABLE(bench_client ProtocolDef.h BenchClient.h BenchClient.cc BenchCommon.h BenchCommon.cc ClientMain.cc)
ADD_EXECUTABLE(test_dll DllMain.cc)
TARGET_LINK_LIBRARIES(bench_server net-static ${LIBUV_LIBRARIES})
TARGET_LINK_LIBRARIES(bench_client net-static ${LIBUV_LIBRARIES})
TARGET_LINK_LIBRARIES(test_dll net ${LIBUV_LIBRARIES})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(bench_server ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_LINK_LIBRARIES(bench_client ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_LINK_LIBRARIES(test_dll ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_COMPILE_DEFINITIONS(test_dll PRIVATE USING_NET_SHARED)
ENDIF()