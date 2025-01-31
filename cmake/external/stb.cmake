string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
  NAME STB_IMAGE
  URL https://github.com/nothings/stb/archive/refs/heads/master.zip
  DOWNLOAD_ONLY TRUE
)

if(STB_IMAGE_ADDED)
  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR ${STB_IMAGE_SOURCE_DIR})

  string(TIMESTAMP AFTER "%s")
  math(EXPR DELTA_STB_IMAGE "${AFTER} - ${BEFORE}")
  message(STATUS "stb_image time: ${DELTA_STB_IMAGE}s")
else()
  message(FATAL_ERROR "Failed to add stb_image package")
endif()
