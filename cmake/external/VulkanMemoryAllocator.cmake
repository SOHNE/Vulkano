string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
  NAME VulkanMemoryAllocator
  GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG 05973d8aeb1a4d12f59aadfb86d20decadba82d1
  OPTIONS
    "VMA_ENABLE_INSTALL ON"
    "VMA_BUILD_DOCUMENTATION OFF"
    "VMA_BUILD_SAMPLES OFF"
)

if(VulkanMemoryAllocator_ADDED)
  list(APPEND VULKANO_LIBRARIES_INCLUDE_DIR${VulkanMemoryAllocator_SOURCE_DIR}/include)
  list(APPEND VULKANO_LIBRARIES GPUOpen::VulkanMemoryAllocator)

  string(TIMESTAMP AFTER "%s")
  math(EXPR DELTA_VMA "${AFTER} - ${BEFORE}")
  message(STATUS "VulkanMemoryAllocator time: ${DELTA_VMA}s")
else()
  message(FATAL_ERROR "VulkanMemoryAllocator could not be added")
endif()
