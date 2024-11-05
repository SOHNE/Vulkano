include(CMakeDependentOption)

#--------------------------------------------------------------------
# Build Options
#--------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build Vulkano as a shared library" OFF)
option(USE_CCACHE "Enable compiler cache that can drastically improve build times" ${VULKANO_IS_MAIN})
