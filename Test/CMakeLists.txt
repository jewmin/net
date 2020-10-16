# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# 项目名
PROJECT(net_test VERSION 2.0)

# 添加源文件
SET(srcs
	${PROJECT_SOURCE_DIR}/Main.cc
	${PROJECT_SOURCE_DIR}/NetworkExpectionTestSuite.cc
	${PROJECT_SOURCE_DIR}/IPAddressTestSuite.cc
	${PROJECT_SOURCE_DIR}/SocketAddressTestSuite.cc
	${PROJECT_SOURCE_DIR}/SocketImplTestSuite.cc
	${PROJECT_SOURCE_DIR}/SocketTestSuite.cc
	# ${PROJECT_SOURCE_DIR}/IOBufferTestSuite.cc
	# ${PROJECT_SOURCE_DIR}/ObjectMgrTestSuite.cc
	# ${PROJECT_SOURCE_DIR}/ReactorTestSuite.cc
	# ${PROJECT_SOURCE_DIR}/ServiceTestSuite.cc
	# ${PROJECT_SOURCE_DIR}/InterfaceTestSuite.cc
)

# 单元测试
ADD_EXECUTABLE(unittest-net-static ${srcs})
ADD_EXECUTABLE(unittest-net ${srcs})
TARGET_LINK_LIBRARIES(unittest-net-static net-static logger-static common-static ${LIBUV_LIBRARIES} ${GTEST_LIBRARIES})
TARGET_LINK_LIBRARIES(unittest-net net logger common ${LIBUV_LIBRARIES} ${GTEST_LIBRARIES})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(unittest-net-static ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_LINK_LIBRARIES(unittest-net ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
	TARGET_COMPILE_DEFINITIONS(unittest-net PRIVATE USING_COMMON_SHARED)
ENDIF()


# 生成可执行文件
# ADD_EXECUTABLE(bench_server ProtocolDef.h BenchServer.h BenchServer.cc BenchCommon.h BenchCommon.cc ServerMain.cc)
# ADD_EXECUTABLE(bench_client ProtocolDef.h BenchClient.h BenchClient.cc BenchCommon.h BenchCommon.cc ClientMain.cc)
# ADD_EXECUTABLE(test_dll DllMain.cc)
# ADD_EXECUTABLE(bench_socket_client BenchSocketClient.cc)
# ADD_EXECUTABLE(bench_socket_server BenchSocketServer.cc)
# ADD_EXECUTABLE(bench_reactor_client BenchReactorClient.cc)
# ADD_EXECUTABLE(bench_reactor_server BenchReactorServer.cc)
# TARGET_LINK_LIBRARIES(bench_server net ${LIBUV_LIBRARIES})
# TARGET_LINK_LIBRARIES(bench_client net ${LIBUV_LIBRARIES})
# TARGET_LINK_LIBRARIES(test_dll net ${LIBUV_LIBRARIES})
# TARGET_LINK_LIBRARIES(bench_socket_client net ${LIBUV_LIBRARIES})
# TARGET_LINK_LIBRARIES(bench_socket_server net ${LIBUV_LIBRARIES})
# TARGET_LINK_LIBRARIES(bench_reactor_client net ${LIBUV_LIBRARIES})
# TARGET_LINK_LIBRARIES(bench_reactor_server net ${LIBUV_LIBRARIES})
# IF(WINDOWS)
# 	TARGET_LINK_LIBRARIES(bench_server ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_LINK_LIBRARIES(bench_client ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_LINK_LIBRARIES(test_dll ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_LINK_LIBRARIES(bench_socket_client ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_LINK_LIBRARIES(bench_socket_server ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_LINK_LIBRARIES(bench_reactor_client ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_LINK_LIBRARIES(bench_reactor_server ws2_32 iphlpapi psapi userenv ${VLD_LIBRARIES})
# 	TARGET_COMPILE_DEFINITIONS(bench_server PRIVATE USING_NET_SHARED)
# 	TARGET_COMPILE_DEFINITIONS(bench_client PRIVATE USING_NET_SHARED)
# 	TARGET_COMPILE_DEFINITIONS(test_dll PRIVATE USING_NET_SHARED)
# 	TARGET_COMPILE_DEFINITIONS(bench_socket_client PRIVATE USING_NET_SHARED)
# 	TARGET_COMPILE_DEFINITIONS(bench_socket_server PRIVATE USING_NET_SHARED)
# 	TARGET_COMPILE_DEFINITIONS(bench_reactor_client PRIVATE USING_NET_SHARED)
# 	TARGET_COMPILE_DEFINITIONS(bench_reactor_server PRIVATE USING_NET_SHARED)
# ENDIF()