string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
  NAME assimp
  GITHUB_REPOSITORY assimp/assimp
  GIT_TAG v5.4.3
  OPTIONS
    "ASSIMP_BUILD_TESTS OFF"
    "ASSIMP_NO_EXPORT OFF"
    "ASSIMP_BUILD_SAMPLES OFF"
    "ASSIMP_BUILD_DOCS OFF"
    "ASSIMP_BUILD_ZLIB ON"
    "ASSIMP_INSTALL ON"
    "BUILD_SHARED_LIBS OFF"
    "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
    "ASSIMP_INJECT_DEBUG_POSTFIX ON"
)

if(assimp_ADDED)
  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR ${assimp_SOURCE_DIR}/include)
  list(APPEND VULKANO_LIBRARIES assimp)

  string(TIMESTAMP AFTER "%s")
  math(EXPR DELTA_ASSIMP "${AFTER} - ${BEFORE}")
  message(STATUS "assimp time: ${DELTA_ASSIMP}s")
else()
  message(FATAL_ERROR "Failed to add assimp package")
endif()
