cmake_minimum_required(VERSION 3.22)

project(thingsmqtt VERSION 1.0.0
                       DESCRIPTION "Lua library for interacting with Thingsboard over MQTT"
					   LANGUAGES CXX)

set(THINGSMQTT_TELEMETRY_TOPIC "v1/devices/me/telemetry" CACHE STRING "Topic to publish telemetry to")
option(THINGSMQTT_THREADED "Enable threaded MQTT client" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL Release)
	set(THINGSMQTT_STACK_CHECK OFF)
else()
	set(THINGSMQTT_STACK_CHECK ON)
endif()

# Only do these if this is the main project
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	# Use folders in IDEs
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	include(CTest)

	find_package(Doxygen)
	if (DOXYGEN_FOUND)
		add_subdirectory(docs)
	else()
		message(STATUS "Doxygen not found, documentation target will not be available")
	endif()
endif()

find_package(Lua 5.1 REQUIRED)
find_package(libmosquitto REQUIRED)

set(INCLUDES
	inc/lua-thingsmqtt.h
	src/lua-thingsmqtt-private.hpp
	src/lua-utils.hpp
	src/nlohmann/json.hpp
	src/threadsafe-queue.hpp
	#src/telemetry-cache.hpp
	src/controller.hpp
	src/mqtt/mqtt-client.hpp
	src/mqtt/mqtt-client-singlethread.hpp
	src/mqtt/mqtt-client-threadsafe.hpp
)

set(SOURCES
	src/lua-thingsmqtt.cpp
	src/lua-utils.cpp
	#src/telemetry-cache.cpp
	src/controller.cpp
	src/mqtt/mqtt-client.cpp
	src/mqtt/mqtt-client-singlethread.cpp
	src/mqtt/mqtt-client-threadsafe.cpp
)

configure_file(
	src/thingsmqtt-config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/inc/thingsmqtt-config.hpp
)

add_library(thingsmqtt SHARED
	${INCLUDES}
	${SOURCES}
)

target_compile_features(thingsmqtt PRIVATE cxx_std_17)
set_target_properties(thingsmqtt PROPERTIES
	CXX_EXTENSIONS OFF
	PREFIX ""  # No 'lib' prefix on Unix-like systems
)

# Generate export header
include(GenerateExportHeader)
generate_export_header(thingsmqtt
	BASE_NAME lua_thingsmqtt
	EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/inc/lua-thingsmqtt-export.h
)

# Enable Link Time Optimization if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
	set_target_properties(thingsmqtt PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_include_directories(thingsmqtt PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/inc>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<INSTALL_INTERFACE:include>

	${LUA_INCLUDE_DIR}
)

target_link_libraries(thingsmqtt PRIVATE
	${LUA_LIBRARIES}
	libmosquitto
)

install(TARGETS thingsmqtt
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static
)

# if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
# 	enable_testing()
# 	add_subdirectory(tests)
# endif()
