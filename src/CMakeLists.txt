#--------------------------------------------------------------------
# Source and Header Files
#--------------------------------------------------------------------
set(VULKANO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/vulkano)
set(VULKANO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

# Header files
set(VULKANO_HEADER_FILES
    ${VULKANO_INCLUDE_DIR}/vo_api.hpp
    ${VULKANO_INCLUDE_DIR}/vo_buffer.hpp
    ${VULKANO_INCLUDE_DIR}/vo_common.hpp
    ${VULKANO_INCLUDE_DIR}/vo_descriptor.hpp
    ${VULKANO_INCLUDE_DIR}/vo_deviceContext.hpp
    ${VULKANO_INCLUDE_DIR}/vo_fence.hpp
    ${VULKANO_INCLUDE_DIR}/vo_frameBuffer.hpp
    ${VULKANO_INCLUDE_DIR}/vo_image.hpp
    ${VULKANO_INCLUDE_DIR}/vo_memory.hpp
    ${VULKANO_INCLUDE_DIR}/vo_model.hpp
    ${VULKANO_INCLUDE_DIR}/vo_pipeline.hpp
    ${VULKANO_INCLUDE_DIR}/vo_renderer.hpp
    ${VULKANO_INCLUDE_DIR}/vo_samplers.hpp
    ${VULKANO_INCLUDE_DIR}/vo_shader.hpp
    ${VULKANO_INCLUDE_DIR}/vo_swapChain.hpp
    ${VULKANO_INCLUDE_DIR}/vo_tools.hpp
    ${VULKANO_INCLUDE_DIR}/vo_window.hpp
    ${VULKANO_INCLUDE_DIR}/vulkano.hpp
)

set(VULKANO_SOURCE_FILES
    ${VULKANO_SOURCE_DIR}/vo_buffer.cpp
    ${VULKANO_SOURCE_DIR}/vo_descriptor.cpp
    ${VULKANO_SOURCE_DIR}/vo_deviceContext.cpp
    ${VULKANO_SOURCE_DIR}/vo_fence.cpp
    ${VULKANO_SOURCE_DIR}/vo_frameBuffer.cpp
    ${VULKANO_SOURCE_DIR}/vo_image.cpp
    ${VULKANO_SOURCE_DIR}/vo_memory.cpp
    ${VULKANO_SOURCE_DIR}/vo_model.cpp
    ${VULKANO_SOURCE_DIR}/vo_pipeline.cpp
    ${VULKANO_SOURCE_DIR}/vo_renderer.cpp
    ${VULKANO_SOURCE_DIR}/vo_samplers.cpp
    ${VULKANO_SOURCE_DIR}/vo_shader.cpp
    ${VULKANO_SOURCE_DIR}/vo_swapChain.cpp
    ${VULKANO_SOURCE_DIR}/vo_window.cpp
)

set(VULKANO_PRIVATE_HEADER_FILES
    ${VULKANO_SOURCE_DIR}/vo_utilities.hpp
)

#--------------------------------------------------------------------
# Library Setup
#--------------------------------------------------------------------
add_library(${PROJECT_NAME} ${VULKANO_SOURCE_FILES} ${VULKANO_HEADER_FILES} ${VULKANO_PRIVATE_HEADER_FILES})
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
  PRIVATE
    ${VULKANO_LIBRARIES_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${VULKANO_INCLUDE_DIR}/vulkano.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${VULKANO_LIBRARIES})

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

set_target_properties(${PROJECT_NAME} PROPERTIES
  LINKER_LANGUAGE CXX
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

#--------------------------------------------------------------------
# Shared Library Configuration
#--------------------------------------------------------------------
if(BUILD_SHARED_LIBS)
  message(STATUS "Building shared library")
  target_compile_definitions(${PROJECT_NAME} PUBLIC VO_SHARED)
endif()

#--------------------------------------------------------------------
# Source Groups
#--------------------------------------------------------------------
GroupSourcesByFolder(${PROJECT_NAME})

