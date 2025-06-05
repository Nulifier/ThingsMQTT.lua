find_path(CJSON_INCLUDE_DIR
	NAMES
		cjson/cJSON.h
	PATHS
		/usr/local/include
		/usr/include
)

find_library(CJSON_LIBRARY
	NAMES
		cjson
	PATHS
		/usr/local/lib
		/usr/lib
)

if(CJSON_LIBRARY)
	set(CJSON_LIBRARIES ${CJSON_LIBRARY} CACHE STRING "cJSON libraries")
endif()

if(CJSON_INCLUDE_DIR)
	file(STRINGS ${CJSON_INCLUDE_DIR}/cjson/cJSON.h cjson_version_str REGEX "#define[ \t]+CJSON_VERSION_MAJOR[ \t]+[0-9]+")
	string(REGEX REPLACE "^#define[ \t]+CJSON_VERSION_MAJOR[ \t]+([0-9]+)" "\\1" cjson_version_major "${cjson_version_str}")
	file(STRINGS ${CJSON_INCLUDE_DIR}/cjson/cJSON.h cjson_version_str REGEX "#define[ \t]+CJSON_VERSION_MINOR[ \t]+[0-9]+")
	string(REGEX REPLACE "^#define[ \t]+CJSON_VERSION_MINOR[ \t]+([0-9]+)" "\\1" cjson_version_minor "${cjson_version_str}")
	file(STRINGS ${CJSON_INCLUDE_DIR}/cjson/cJSON.h cjson_version_str REGEX "#define[ \t]+CJSON_VERSION_PATCH[ \t]+[0-9]+")
	string(REGEX REPLACE "^#define[ \t]+CJSON_VERSION_PATCH[ \t]+([0-9]+)" "\\1" cjson_version_patch "${cjson_version_str}")

	set(CJSON_VERSION "${cjson_version_major}.${cjson_version_minor}.${cjson_version_patch}")

	unset(cjson_version_str)
	unset(cjson_version_major)
	unset(cjson_version_minor)
	unset(cjson_version_patch)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CJSON
								  REQUIRED_VARS CJSON_LIBRARIES CJSON_INCLUDE_DIR
								  VERSION_VAR CJSON_VERSION
								  HANDLE_VERSION_RANGE)
