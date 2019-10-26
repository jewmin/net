# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(test_suite VERSION 1.1.1)

# 添加源文件
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR} test_srcs)
# MESSAGE(${test_srcs})

# 设置
IF (MSVC)
	ADD_DEFINITIONS(-DUSE_VLD)
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF()

# 第三方类库
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/gtest/include)
IF (MSVC)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/vld/include)
ENDIF()

LINK_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/gtest/lib)
IF (MSVC)
	LINK_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/vld/lib)
	SET(vldFile
		${CMAKE_CURRENT_LIST_DIR}/vld/lib/vld_x64.dll
		${CMAKE_CURRENT_LIST_DIR}/vld/lib/dbghelp.dll
		${CMAKE_CURRENT_LIST_DIR}/vld/lib/Microsoft.DTfW.DHL.manifest
		${CMAKE_CURRENT_LIST_DIR}/vld/lib/vld.ini
	)
ELSE()
    LINK_DIRECTORIES(${OUTPUT_DIRECTORY})
ENDIF()

# 生成可执行文件
ADD_EXECUTABLE(${PROJECT_NAME} ${test_srcs})
IF (MSVC)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} libnet-static vld ws2_32 iphlpapi psapi userenv debug libuvd debug gtestd debug gmockd optimized libuv optimized gtest optimized gmock)
	ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${vldFile} ${OUTPUT_DIRECTORY})
ELSE()
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug netd.a optimized net.a uv gtest gmock)
ENDIF()
