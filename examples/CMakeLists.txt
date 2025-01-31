# --------------------------------------------------------------------
# Project Setup
# --------------------------------------------------------------------
cmake_minimum_required(VERSION 3.26)
project(VulkanExamples LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------
# Output Directories
# --------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# --------------------------------------------------------------------
# Import Tools and Dependencies
# --------------------------------------------------------------------
include(../cmake/CPM.cmake)
include(../cmake/tools.cmake)

cpmaddpackage(NAME Vulkano SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
cpmaddpackage("gh:TheLartians/GroupSourcesByFolder.cmake@1.0")

# --------------------------------------------------------------------
# IMGUI Setup
# --------------------------------------------------------------------
cpmaddpackage(NAME IMGUI URL
              "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.4.zip")

if(IMGUI_ADDED)
  add_library(IMGUI STATIC)

  target_sources(
    IMGUI
    PRIVATE # ImGui core source files
            ${IMGUI_SOURCE_DIR}/imgui.cpp
            ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
            ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
            ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
            ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
            # Miscellaneous utility files
            ${IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
            # Vulkan and GLFW backend implementation files
            ${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

  target_include_directories(
    IMGUI
    PUBLIC ${IMGUI_SOURCE_DIR} ${IMGUI_SOURCE_DIR}/backends
    PRIVATE ${EXAMPLE_LIBRARIES_INCLUDE_DIR})

  # Find Vulkan and GLFW
  find_package(Vulkan REQUIRED)

  # Link Vulkan, GLFW, and other necessary libraries
  target_link_libraries(IMGUI PUBLIC ${Vulkan_LIBRARIES} glfw ${CMAKE_DL_LIBS})
endif()

# --------------------------------------------------------------------
# Examples Setup
# --------------------------------------------------------------------
# List of example folders
set(EXAMPLE_FOLDERS OffscreenRendering Triangle Model)

# Shared main file in the root directory
set(SHARED_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

foreach(EXAMPLE ${EXAMPLE_FOLDERS})
  # Define source and header files for the example
  file(GLOB EXAMPLE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}/*.cpp)
  file(GLOB EXAMPLE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE}/*.hpp)

  # Create executable for each example, including the shared main.cpp
  add_executable(${EXAMPLE} ${SHARED_MAIN} ${EXAMPLE_SOURCE_FILES}
                            ${EXAMPLE_HEADER_FILES})
  target_include_directories(${EXAMPLE}
                             PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE})

  # Link libraries
  target_link_libraries(${EXAMPLE} PUBLIC Vulkano::Vulkano IMGUI)

  # Platform-specific configuration
  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${EXAMPLE})
    set_property(TARGET ${EXAMPLE} PROPERTY WIN32_EXECUTABLE true)
    target_link_options(${EXAMPLE} PUBLIC -static -static-libgcc
                        -static-libstdc++)
  endif()

  if(APPLE)
    list(APPEND APPLE_FRAMEWORKS "-framework Cocoa" "-framework IOKit"
         "-framework CoreFoundation" "-framework CoreVideo")
    target_link_libraries(${EXAMPLE} PRIVATE ${APPLE_FRAMEWORKS})
  endif()

  # Group sources for each example
  groupsourcesbyfolder(${EXAMPLE})
endforeach()
