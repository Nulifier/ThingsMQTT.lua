find_path(LIBMOSQUITTO_INCLUDE_DIR
	NAMES
		mosquitto.h
	PATHS
		/usr/local/include
		/usr/include
)

find_library(LIBMOSQUITTO_LIBRARY
	NAMES
		mosquitto
	PATHS
		/usr/local/lib
		/usr/lib
)

if(LIBMOSQUITTO_LIBRARY)
	set(LIBMOSQUITTO_LIBRARIES ${LIBMOSQUITTO_LIBRARY} CACHE STRING "Mosquitto libraries")
endif()

if(LIBMOSQUITTO_INCLUDE_DIR)
	file(STRINGS ${LIBMOSQUITTO_INCLUDE_DIR}/mosquitto.h mosquitto_version_str REGEX "#define[ \t]+LIBMOSQUITTO_MAJOR[ \t]+[0-9]+")
	string(REGEX REPLACE "^#define[ \t]+LIBMOSQUITTO_MAJOR[ \t]+([0-9]+)" "\\1" mosquitto_version_major "${mosquitto_version_str}")
	file(STRINGS ${LIBMOSQUITTO_INCLUDE_DIR}/mosquitto.h mosquitto_version_str REGEX "#define[ \t]+LIBMOSQUITTO_MINOR[ \t]+[0-9]+")
	string(REGEX REPLACE "^#define[ \t]+LIBMOSQUITTO_MINOR[ \t]+([0-9]+)" "\\1" mosquitto_version_minor "${mosquitto_version_str}")
	file(STRINGS ${LIBMOSQUITTO_INCLUDE_DIR}/mosquitto.h mosquitto_version_str REGEX "#define[ \t]+LIBMOSQUITTO_REVISION[ \t]+[0-9]+")
	string(REGEX REPLACE "^#define[ \t]+LIBMOSQUITTO_REVISION[ \t]+([0-9]+)" "\\1" mosquitto_version_patch "${mosquitto_version_str}")

	set(LIBMOSQUITTO_VERSION "${mosquitto_version_major}.${mosquitto_version_minor}.${mosquitto_version_patch}")

	unset(mosquitto_version_str)
	unset(mosquitto_version_major)
	unset(mosquitto_version_minor)
	unset(mosquitto_version_patch)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libmosquitto
								  REQUIRED_VARS LIBMOSQUITTO_LIBRARIES LIBMOSQUITTO_INCLUDE_DIR
								  VERSION_VAR LIBMOSQUITTO_VERSION
								  HANDLE_VERSION_RANGE)

add_library(libmosquitto INTERFACE)
target_include_directories(libmosquitto INTERFACE ${LIBMOSQUITTO_INCLUDE_DIR})
target_link_libraries(libmosquitto INTERFACE ${LIBMOSQUITTO_LIBRARIES})
