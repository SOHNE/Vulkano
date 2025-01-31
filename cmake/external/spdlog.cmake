string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.14.1
  OPTIONS
    "SPDLOG_BUILD_SHARED OFF"
    "SPDLOG_BUILD_STATIC ON"
    "SPDLOG_INSTALL ON"
    "SPDLOG_HEADER_ONLY ON"
)

if(spdlog_ADDED)
  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR ${SPDLOG_SOURCE_DIR}/include)
  list(APPEND VULKANO_LIBRARIES spdlog::spdlog)

  string(TIMESTAMP AFTER "%s")
  math(EXPR DELTA_SPDLOG "${AFTER} - ${BEFORE}")
  message(STATUS "spdlog time: ${DELTA_SPDLOG}s")
else()
  message(FATAL_ERROR "Failed to add spdlog package")
endif()

