string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
  NAME CGLM
  GITHUB_REPOSITORY recp/cglm
  GIT_TAG v0.9.4
  OPTIONS
    "CGLM_SHARED OFF"
    "CGLM_STATIC ON"
    "CGLM_USE_TEST OFF"
    "CGLM_BUILD_DOCS OFF"
)

if(CGLM_ADDED)
  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR ${CGLM_SOURCE_DIR}/include)
  list(APPEND VULKANO_LIBRARIES cglm)

  string(TIMESTAMP AFTER "%s")
  math(EXPR DELTA_CGLM "${AFTER} - ${BEFORE}")
  message(STATUS "CGLM time: ${DELTA_CGLM}s")
else()
  message(FATAL_ERROR "Failed to add CGLM package")
endif()
