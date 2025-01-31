# CPM Package Lock
# This file should be committed to version control

# PackageProject.cmake
CPMDeclarePackage(PackageProject.cmake
  VERSION 1.12.0
  GITHUB_REPOSITORY TheLartians/PackageProject.cmake
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# GroupSourcesByFolder.cmake
CPMDeclarePackage(GroupSourcesByFolder.cmake
  VERSION 1.0
  GITHUB_REPOSITORY TheLartians/GroupSourcesByFolder.cmake
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
)
# GLFW
CPMDeclarePackage(GLFW
  NAME GLFW
  GIT_TAG 3.4
  GITHUB_REPOSITORY glfw/glfw
  OPTIONS
    "GLFW_BUILD_DOCS OFF"
    "GLFW_BUILD_TESTS OFF"
    "GLFW_BUILD_EXAMPLES OFF"
    "GLFW_INSTALL ON"
)
# VulkanHeaders
CPMDeclarePackage(VulkanHeaders
  NAME VulkanHeaders
  VERSION 1.3.301
  GIT_TAG v1.3.301
  GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
  OPRTIONS VULKAN_HEADERS_ENABLE_INSTALL ON
)
# CGLM
CPMDeclarePackage(CGLM
  NAME CGLM
  GIT_TAG v0.9.4
  GITHUB_REPOSITORY recp/cglm
  OPTIONS
    "CGLM_SHARED OFF"
    "CGLM_STATIC ON"
    "CGLM_USE_TEST OFF"
    "CGLM_BUILD_DOCS OFF"
)
# spdlog
CPMDeclarePackage(spdlog
  NAME spdlog
  VERSION 1.14.1
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS
    "SPDLOG_BUILD_SHARED OFF"
    "SPDLOG_BUILD_STATIC ON"
    "SPDLOG_INSTALL ON"
    "SPDLOG_HEADER_ONLY ON"
)
