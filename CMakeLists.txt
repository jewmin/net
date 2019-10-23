# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

# 项目名
PROJECT(libnet VERSION 2.0.1)
# MESSAGE(${PROJECT_NAME} ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_LIST_DIR})

# 添加源文件
SET(Common
	${CMAKE_CURRENT_LIST_DIR}/CommonDef.h
	${CMAKE_CURRENT_LIST_DIR}/Logger.h
	${CMAKE_CURRENT_LIST_DIR}/Logger.cc
	${CMAKE_CURRENT_LIST_DIR}/ObjectMgr.h
	${CMAKE_CURRENT_LIST_DIR}/Packet.hpp
	${CMAKE_CURRENT_LIST_DIR}/PacketReader.hpp
	${CMAKE_CURRENT_LIST_DIR}/RefCountedObject.h
	${CMAKE_CURRENT_LIST_DIR}/SharedPtr.h
)
SET(Address
	${CMAKE_CURRENT_LIST_DIR}/Address/AddressFamily.h
	${CMAKE_CURRENT_LIST_DIR}/Address/IPAddress.h
	${CMAKE_CURRENT_LIST_DIR}/Address/IPAddress.cc
	${CMAKE_CURRENT_LIST_DIR}/Address/IPAddressImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Address/IPAddressImpl.cc
	${CMAKE_CURRENT_LIST_DIR}/Address/SocketAddress.h
	${CMAKE_CURRENT_LIST_DIR}/Address/SocketAddress.cc
	${CMAKE_CURRENT_LIST_DIR}/Address/SocketAddressImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Address/SocketAddressImpl.cc
)
SET(Sockets
	${CMAKE_CURRENT_LIST_DIR}/Sockets/ServerSocket.h
	${CMAKE_CURRENT_LIST_DIR}/Sockets/ServerSocket.cc
	${CMAKE_CURRENT_LIST_DIR}/Sockets/ServerSocketImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Sockets/ServerSocketImpl.cc
	${CMAKE_CURRENT_LIST_DIR}/Sockets/Socket.h
	${CMAKE_CURRENT_LIST_DIR}/Sockets/Socket.cc
	${CMAKE_CURRENT_LIST_DIR}/Sockets/SocketImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Sockets/SocketImpl.cc
	${CMAKE_CURRENT_LIST_DIR}/Sockets/StreamSocket.h
	${CMAKE_CURRENT_LIST_DIR}/Sockets/StreamSocket.cc
	${CMAKE_CURRENT_LIST_DIR}/Sockets/StreamSocketImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Sockets/StreamSocketImpl.cc
)
SET(Reactor
	${CMAKE_CURRENT_LIST_DIR}/Reactor/EventHandler.h
	${CMAKE_CURRENT_LIST_DIR}/Reactor/EventHandler.cc
	${CMAKE_CURRENT_LIST_DIR}/Reactor/EventReactor.h
	${CMAKE_CURRENT_LIST_DIR}/Reactor/EventReactor.cc
	${CMAKE_CURRENT_LIST_DIR}/Reactor/SocketAcceptor.h
	${CMAKE_CURRENT_LIST_DIR}/Reactor/SocketAcceptor.cc
	${CMAKE_CURRENT_LIST_DIR}/Reactor/SocketConnection.h
	${CMAKE_CURRENT_LIST_DIR}/Reactor/SocketConnection.cc
	${CMAKE_CURRENT_LIST_DIR}/Reactor/SocketConnector.h
	${CMAKE_CURRENT_LIST_DIR}/Reactor/SocketConnector.cc
)
SET(Core
	${CMAKE_CURRENT_LIST_DIR}/Core/IEvent.h
	${CMAKE_CURRENT_LIST_DIR}/Core/INotification.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketAcceptorImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketAcceptorImpl.cc
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketClient.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketClient.cc
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketConnectionImpl.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketConnectionImpl.cc
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketServer.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketServer.cc
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketWrapper.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketWrapper.cc
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketWrapperMgr.h
	${CMAKE_CURRENT_LIST_DIR}/Core/SocketWrapperMgr.cc
)
SET(Interface
	${CMAKE_CURRENT_LIST_DIR}/Interface/ApplicationContext.h
	${CMAKE_CURRENT_LIST_DIR}/Interface/ApplicationContext.cc
	${CMAKE_CURRENT_LIST_DIR}/Interface/Interface.h
	${CMAKE_CURRENT_LIST_DIR}/Interface/Interface.cc
)

SET(all_srcs
	${Common}
	${Address}
	${Sockets}
	${Reactor}
	${Core}
	${Interface}
)
# MESSAGE(${all_srcs})

# 生成目录结构
SOURCE_GROUP("" FILES ${Common})
SOURCE_GROUP(Address FILES ${Address})
SOURCE_GROUP(Sockets FILES ${Sockets})
SOURCE_GROUP(Reactor FILES ${Reactor})
SOURCE_GROUP(Core FILES ${Core})
SOURCE_GROUP(Interface FILES ${Interface})

# 设置
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_SUPPRESS_REGENERATION true)

IF (MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_COMPILE_OPTIONS(/utf-8)
ENDIF()

IF (CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS(-m64)
	ADD_DEFINITIONS(-std=c++11)
ENDIF()

# 头文件
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/libuv/include)

# 第三方类库
LINK_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/libuv/lib)

# 输出目录
SET(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

# 生成静态库、动态库
IF (MSVC)
	ADD_LIBRARY(libnet-static STATIC ${all_srcs})
	ADD_LIBRARY(libnet SHARED ${all_srcs})
	TARGET_COMPILE_DEFINITIONS(libnet PRIVATE BUILDING_NET_SHARED)
	TARGET_LINK_LIBRARIES(libnet libuv ws2_32 iphlpapi psapi userenv)
ELSE()
	ADD_LIBRARY(net-static STATIC ${all_srcs})
	SET_TARGET_PROPERTIES(net-static PROPERTIES OUTPUT_NAME "net")
	ADD_LIBRARY(net SHARED ${all_srcs})
	TARGET_COMPILE_OPTIONS(net PRIVATE -fPIC -fvisibility=hidden)
ENDIF()

# 添加单元测试项目、压测项目
ADD_SUBDIRECTORY(TestSuite)
ADD_SUBDIRECTORY(Benchmark)