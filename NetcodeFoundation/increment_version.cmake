set(CONFIGURABLE_CXX_FILE "${SRC_DIR}/Version.in.cpp")
set(OUTPUT_CXX_FILE "${SRC_DIR}/Generated_Version.cpp")
set(CACHE_FILE "${SRC_DIR}/BuildNumberCache.txt")
set(DATE_CACHE_FILE "${SRC_DIR}/BuildNumberDateCache.txt")

set(MAJOR_NUMBER "0")
set(MINOR_NUMBER "0")

file(TIMESTAMP ${VERSIONED_BINARY} EXECUTABLE_TIMESTAMP)

IF(EXISTS ${DATE_CACHE_FILE})
	file(READ ${DATE_CACHE_FILE} SAVED_TIMESTAMP)

	IF(NOT ${EXECUTABLE_TIMESTAMP} MATCHES ${SAVED_TIMESTAMP})
		IF(EXISTS ${CACHE_FILE})
			file(READ ${CACHE_FILE} BUILD_NUMBER)
			math(EXPR BUILD_NUMBER "${BUILD_NUMBER}+1")
		ELSE()
			set(BUILD_NUMBER "1")
		ENDIF()
		configure_file(${CONFIGURABLE_CXX_FILE} ${OUTPUT_CXX_FILE})

		file(WRITE ${CACHE_FILE} "${BUILD_NUMBER}")
	ENDIF()
ENDIF()

file(WRITE ${DATE_CACHE_FILE} "${EXECUTABLE_TIMESTAMP}")

