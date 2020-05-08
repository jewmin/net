# cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)
# MESSAGE("CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})
# MESSAGE("CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})
# MESSAGE("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
# MESSAGE("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
# MESSAGE("CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
# MESSAGE("CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR})

SET(GTEST_VERSION 1.8.1)
# Download and unpack googletest at configure time
CONFIGURE_FILE(CMakeLists.txt.in googletest-download/CMakeLists.txt)
EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download")
EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download")

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
ADD_SUBDIRECTORY("${CMAKE_CURRENT_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")

SET(GTEST_ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/googletest-src)

FIND_PATH(GTEST_GTEST_INCLUDE_DIR
	NAMES gtest/gtest.h
	HINTS ${gtest_SOURCE_DIR}/include
)

FIND_PATH(GTEST_GMOCK_INCLUDE_DIR
	NAMES gmock/gmock.h
	HINTS ${gmock_SOURCE_DIR}/include
)

SET(GTEST_GTEST_LIBRARY optimized gtest debug gtestd)
SET(GTEST_GMOCK_LIBRARY optimized gmock debug gmockd)
SET(GTEST_INCLUDE_DIRS ${GTEST_GTEST_INCLUDE_DIR} ${GTEST_GMOCK_INCLUDE_DIR} CACHE PATH "gtest library directory" FORCE)
SET(GTEST_LIBRARIES ${GTEST_GTEST_LIBRARY} ${GTEST_GMOCK_LIBRARY} CACHE DOC "gtest all library" FORCE)