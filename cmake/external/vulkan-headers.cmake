string(TIMESTAMP BEFORE "%s")

set(VULKAN_HEADERS_ENABLE_INSTALL ON CACHE BOOL "Install Vulkan-Headers" FORCE)

CPMAddPackage(
  NAME VulkanHeaders
  GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
  VERSION 1.3.301
  GIT_TAG v1.3.301
  OPRTIONS
    "VULKAN_HEADERS_ENABLE_INSTALL ON"
    "VULKAN_HEADERS_ENABLE_TESTS OFF"
)

if(VulkanHeaders_ADDED)
  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR ${VulkanHeaders_SOURCE_DIR}/include)
  list(APPEND VULKANO_LIBRARIES Vulkan::Headers)

  string(TIMESTAMP AFTER "%s")
  math(EXPR DELTA_VULKAN_HEADERS "${AFTER} - ${BEFORE}")
  message(STATUS "Vulkan-Headers time: ${DELTA_VULKAN_HEADERS}s")
else()
  # Try finding Vulkan package as fallback
  find_package(Vulkan REQUIRED)
  if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Neither Vulkan-Headers package nor system Vulkan SDK could be found")
  endif()

  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR ${Vulkan_INCLUDE_DIRS})
  list(APPEND VULKANO_LIBRARIES Vulkan::Vulkan)
  message(STATUS "Using system Vulkan SDK")
endif()
