# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test_suite VERSION 1.5.0)
# MESSAGE(${PROJECT_NAME} " " ${PROJECT_SOURCE_DIR} " " ${CMAKE_CURRENT_SOURCE_DIR} " " ${CMAKE_CURRENT_LIST_DIR} " " ${CMAKE_CURRENT_BINARY_DIR})

# 添加源文件
SET(test_srcs
	${PROJECT_SOURCE_DIR}/Main.cc
	${PROJECT_SOURCE_DIR}/AllocatorTestSuite.cc
	${PROJECT_SOURCE_DIR}/LoggerTestSuite.cc
	${PROJECT_SOURCE_DIR}/RefCountedObjectTestSuite.cc
	${PROJECT_SOURCE_DIR}/IOBufferTestSuite.cc
	${PROJECT_SOURCE_DIR}/NetObjectTestSuite.cc
	${PROJECT_SOURCE_DIR}/ObjectMgrTestSuite.cc
	${PROJECT_SOURCE_DIR}/IPAddressTestSuite.cc
	${PROJECT_SOURCE_DIR}/SocketAddressTestSuite.cc
	${PROJECT_SOURCE_DIR}/SocketTestSuite.cc
	${PROJECT_SOURCE_DIR}/ReactorTestSuite.cc
)
# AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} test_srcs)
# MESSAGE(${test_srcs})

# gtest
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../3rd/gtest/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../3rd/gtest/lib)

SET(GTEST_DLL_FILE_DEBUG
	${PROJECT_SOURCE_DIR}/../3rd/gtest/lib/gmockd.dll
	${PROJECT_SOURCE_DIR}/../3rd/gtest/lib/gtestd.dll
)
SET(GTEST_DLL_FILE_RELEASE
	${PROJECT_SOURCE_DIR}/../3rd/gtest/lib/gmock.dll
	${PROJECT_SOURCE_DIR}/../3rd/gtest/lib/gtest.dll
)

# vld
IF(VLD)
	FIND_PACKAGE(VLD)
	IF(VLD_FOUND)
		ADD_DEFINITIONS(-DUSE_VLD)
		INCLUDE_DIRECTORIES(${VLD_INCLUDE_DIRS})
		MESSAGE("Found VLD version ${VLD_VERSION}")
	ENDIF()
ENDIF()

# 生成可执行文件
ADD_EXECUTABLE(${PROJECT_NAME} ${test_srcs})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libnet-static ${VLD_LIBRARIES} ws2_32 iphlpapi psapi userenv libuv debug gtestd debug gmockd optimized gtest optimized gmock)
	IF(RELEASE)
		ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VLD_DLL_FILE} ${GTEST_DLL_FILE_RELEASE} ${UV_DLL_FILE_RELEASE} ${OUTPUT_DIRECTORY_RELEASE})
	ELSE()
		ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VLD_DLL_FILE} ${GTEST_DLL_FILE_DEBUG} ${UV_DLL_FILE_DEBUG} ${OUTPUT_DIRECTORY_DEBUG})
	ENDIF()
ELSEIF(LINUX)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} net-static uv debug gtestd debug gmockd optimized gtest optimized gmock)
ENDIF()
