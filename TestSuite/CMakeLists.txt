# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test_suite VERSION 1.4.0)

# 添加源文件
SET(test_srcs
	${CMAKE_CURRENT_LIST_DIR}/Main.cc
	${CMAKE_CURRENT_LIST_DIR}/AllocatorTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/LoggerTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/RefCountedObjectTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/IOBufferTestSuite.cc
	${CMAKE_CURRENT_LIST_DIR}/NetObjectTestSuite.cc
	# ${CMAKE_CURRENT_LIST_DIR}/ObjectMgrTestSuite.cc
	# ${CMAKE_CURRENT_LIST_DIR}/IPAddressTestSuite.cc
	# ${CMAKE_CURRENT_LIST_DIR}/SocketAddressTestSuite.cc
	# ${CMAKE_CURRENT_LIST_DIR}/SocketTestSuite.cc
	# ${CMAKE_CURRENT_LIST_DIR}/ReactorTestSuite.cc
)
# AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR} test_srcs)
# MESSAGE(${test_srcs})

# 生成可执行文件
ADD_EXECUTABLE(${PROJECT_NAME} ${test_srcs})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libnet-static vld ws2_32 iphlpapi psapi userenv debug gtestd debug gmockd optimized gtest optimized gmock)
	IF(RELEASE)
		ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VLD_DLL_FILE} ${OUTPUT_DIRECTORY_RELEASE})
	ELSE()
		ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VLD_DLL_FILE} ${OUTPUT_DIRECTORY_DEBUG})
	ENDIF()
ELSEIF(LINUX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} net-static debug gtestd debug gmockd optimized gtest optimized gmock)
ENDIF()
