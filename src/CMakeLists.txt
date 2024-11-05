#--------------------------------------------------------------------
# Source and Header Files
#--------------------------------------------------------------------
set(VULKANO_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/vulkano)
set(VULKANO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

# Header files
set(VULKANO_HEADER_FILES
    ${VULKANO_INCLUDE_DIR}/vulkano.hpp
)

set(VULKANO_SOURCE_FILES

)

#--------------------------------------------------------------------
# Library Setup
#--------------------------------------------------------------------
add_library(${PROJECT_NAME} ${VULKANO_SOURCE_FILES} ${VULKANO_HEADER_FILES})
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
  PRIVATE
    ${VULKANO_LIBRARIES_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${DURA_INCLUDE_DIR}/vulkano.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${VULKANO_LIBRARIES})

# Enforce standards conformance on MSVC
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

