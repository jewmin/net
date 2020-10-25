# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# 项目名
PROJECT(libnet VERSION 5.0)

# 开关
OPTION(VLD "use Visual Leak Detector to check memory on windows" ON)
OPTION(RELEASE "compile the release version" OFF)

# 设置模块路径
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/logger/common/3rd/vld)

# 平台
IF(MSVC)
	SET(WINDOWS ON)
	SET(LINUX OFF)
ELSEIF(CMAKE_COMPILER_IS_GNUCXX)
	SET(WINDOWS OFF)
	SET(LINUX ON)
	SET(VLD OFF)
ENDIF()

# 显示参数
MESSAGE(STATUS "VLD:" ${VLD})
MESSAGE(STATUS "RELEASE:" ${RELEASE})
MESSAGE(STATUS "WINDOWS:" ${WINDOWS})
MESSAGE(STATUS "LINUX:" ${LINUX})
MESSAGE(STATUS "PROJECT_SOURCE_DIR:" ${PROJECT_SOURCE_DIR})

# 添加源文件
# SET(Common
# 	${PROJECT_SOURCE_DIR}/include/Common/IOBuffer.h
# 	${PROJECT_SOURCE_DIR}/include/Common/BipBuffer.h
# 	${PROJECT_SOURCE_DIR}/src/Common/BipBuffer.cc
# 	${PROJECT_SOURCE_DIR}/include/Common/StraightBuffer.h
# 	${PROJECT_SOURCE_DIR}/src/Common/StraightBuffer.cc
# 	${PROJECT_SOURCE_DIR}/include/Common/ObjectMgr.h
# )
# SET(Sockets
# )
# SET(Reactor
# )
# SET(Interface
# 	${PROJECT_SOURCE_DIR}/include/Interface/INotification.h
# 	${PROJECT_SOURCE_DIR}/include/Interface/Client.h
# 	${PROJECT_SOURCE_DIR}/src/Interface/Client.cc
# 	${PROJECT_SOURCE_DIR}/include/Interface/Connection.h
# 	${PROJECT_SOURCE_DIR}/src/Interface/Connection.cc
# 	${PROJECT_SOURCE_DIR}/include/Interface/Server.h
# 	${PROJECT_SOURCE_DIR}/src/Interface/Server.cc
# 	${PROJECT_SOURCE_DIR}/include/Interface/ConnectionMgr.h
# 	${PROJECT_SOURCE_DIR}/src/Interface/ConnectionMgr.cc
# 	${PROJECT_SOURCE_DIR}/include/Interface/AppService.h
# 	${PROJECT_SOURCE_DIR}/src/Interface/AppService.cc
# 	${PROJECT_SOURCE_DIR}/src/Interface/Interface.cc
# )

SET(srcs
	${PROJECT_SOURCE_DIR}/include/NetworkException.h
	${PROJECT_SOURCE_DIR}/include/Address/AddressFamily.h
	${PROJECT_SOURCE_DIR}/include/Address/IPAddressImpl.h
	${PROJECT_SOURCE_DIR}/include/Address/IPAddress.h
	${PROJECT_SOURCE_DIR}/include/Address/SocketAddressImpl.h
	${PROJECT_SOURCE_DIR}/include/Address/SocketAddress.h
	${PROJECT_SOURCE_DIR}/include/Sockets/UvData.h
	${PROJECT_SOURCE_DIR}/include/Sockets/SocketImpl.h
	${PROJECT_SOURCE_DIR}/include/Sockets/StreamSocketImpl.h
	${PROJECT_SOURCE_DIR}/include/Sockets/Socket.h
	${PROJECT_SOURCE_DIR}/include/Sockets/StreamSocket.h
	${PROJECT_SOURCE_DIR}/include/Sockets/ServerSocketImpl.h
	${PROJECT_SOURCE_DIR}/include/Sockets/ServerSocket.h
	${PROJECT_SOURCE_DIR}/include/Reactor/ConnectState.h
	${PROJECT_SOURCE_DIR}/include/Reactor/EventHandler.h
	${PROJECT_SOURCE_DIR}/include/Reactor/EventReactor.h
	${PROJECT_SOURCE_DIR}/include/Reactor/SocketConnection.h
	${PROJECT_SOURCE_DIR}/include/Reactor/SocketAcceptor.h
	${PROJECT_SOURCE_DIR}/include/Reactor/SocketConnector.h

	${PROJECT_SOURCE_DIR}/src/NetworkException.cc
	${PROJECT_SOURCE_DIR}/src/Address/IPAddressImpl.cc
	${PROJECT_SOURCE_DIR}/src/Address/IPAddress.cc
	${PROJECT_SOURCE_DIR}/src/Address/SocketAddressImpl.cc
	${PROJECT_SOURCE_DIR}/src/Address/SocketAddress.cc
	${PROJECT_SOURCE_DIR}/src/Sockets/SocketImpl.cc
	${PROJECT_SOURCE_DIR}/src/Sockets/StreamSocketImpl.cc
	${PROJECT_SOURCE_DIR}/src/Sockets/Socket.cc
	${PROJECT_SOURCE_DIR}/src/Sockets/StreamSocket.cc
	${PROJECT_SOURCE_DIR}/src/Sockets/ServerSocketImpl.cc
	${PROJECT_SOURCE_DIR}/src/Sockets/ServerSocket.cc
	${PROJECT_SOURCE_DIR}/src/Reactor/EventHandler.cc
	${PROJECT_SOURCE_DIR}/src/Reactor/EventReactor.cc
	${PROJECT_SOURCE_DIR}/src/Reactor/SocketConnection.cc
	${PROJECT_SOURCE_DIR}/src/Reactor/SocketAcceptor.cc
	${PROJECT_SOURCE_DIR}/src/Reactor/SocketConnector.cc
)

# 生成目录结构
SOURCE_GROUP(TREE ${PROJECT_SOURCE_DIR} FILES ${srcs})

# 设置输出目录
SET(CMAKE_DEBUG_POSTFIX "d")
SET(CMAKE_SUPPRESS_REGENERATION true)
SET(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
SET(OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
SET(OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY_DEBUG})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY_DEBUG})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY_DEBUG})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE})

# 编译项
IF(WINDOWS)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
	ADD_COMPILE_OPTIONS(/utf-8)
ELSEIF(LINUX)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -ggdb")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")
	ADD_DEFINITIONS(-m64)
ENDIF()

# 显式输出编译选项
IF(RELEASE)
	SET(CMAKE_BUILD_TYPE Release)
	MESSAGE(STATUS "flags:" ${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS})
ELSE()
	SET(CMAKE_BUILD_TYPE Debug)
	MESSAGE(STATUS "flags:" ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS})
ENDIF()

# libuv
ADD_SUBDIRECTORY(3rd/libuv)
SET(LIBUV_LIBRARY optimized uv debug uvd)
SET(LIBUV_LIBRARIES ${LIBUV_LIBRARY} CACHE STRING "libuv all library" FORCE)
SET(LIBUV_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rd/libuv/include)
SET(LIBUV_INCLUDE_DIRS ${LIBUV_INCLUDE_DIR} CACHE PATH "libuv include directory" FORCE)

# liblogger
ADD_SUBDIRECTORY(logger)

# vld
IF(VLD)
	FIND_PACKAGE(VLD)
	IF(VLD_FOUND)
		ADD_DEFINITIONS(-DUSE_VLD)
		INCLUDE_DIRECTORIES(${VLD_INCLUDE_DIRS})
	ENDIF()
ENDIF()

# 头文件
SET(NET_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include CACHE PATH "net include directory" FORCE)
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${COMMON_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${LOGGER_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${LIBUV_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${NET_INCLUDE_DIRS})

# 库目录
LINK_DIRECTORIES(${OUTPUT_DIRECTORY})

# 生成静态库、动态库
ADD_LIBRARY(net-static STATIC ${srcs})
ADD_LIBRARY(net SHARED ${srcs})
TARGET_LINK_LIBRARIES(net-static logger-static common-static ${LIBUV_LIBRARIES})
TARGET_LINK_LIBRARIES(net logger common ${LIBUV_LIBRARIES})
IF(WINDOWS)
	TARGET_LINK_LIBRARIES(net ws2_32 iphlpapi psapi userenv)
	TARGET_COMPILE_DEFINITIONS(net PRIVATE BUILDING_COMMON_SHARED)
ELSEIF(LINUX)
	TARGET_COMPILE_OPTIONS(net PRIVATE -fPIC -fvisibility=hidden)
ENDIF()

# 添加测试项目
ADD_SUBDIRECTORY(test)