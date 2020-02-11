# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test_suite VERSION 1.3.0)

# 添加源文件
SET(test_srcs
	${CMAKE_CURRENT_LIST_DIR}/Main.cc
	${CMAKE_CURRENT_LIST_DIR}/AllocatorTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/LoggerTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/IPAddressTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/SocketAddressTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/RefCountedObjectTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/SocketTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/ReactorTestSuite.cc
)
# AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR} test_srcs)
# MESSAGE(${test_srcs})

# 生成可执行文件
ADD_EXECUTABLE(${PROJECT_NAME} ${test_srcs})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libnet-static vld ws2_32 iphlpapi psapi userenv debug libuvd debug gtestd debug gmockd optimized libuv optimized gtest optimized gmock)
	ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dllFile} ${OUTPUT_DIRECTORY})
ELSEIF(LINUX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} net-static debug uvd debug gtestd debug gmockd optimized uv optimized gtest optimized gmock)
ENDIF()
