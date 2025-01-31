#ifndef VULKANO_UTILITIES_H
#define VULKANO_UTILITIES_H

#include <spdlog/spdlog.h>
#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "vulkano/vo_common.hpp"

#define GetCurrentDir getcwd

static char g_ApplicationDirectory[FILENAME_MAX];
static bool g_WasInitialized = false;

namespace fs = std::filesystem;

static void
InitializeFileSystem()
{
    if( g_WasInitialized )
        {
            return;
        }
    g_WasInitialized = true;

    const bool result = GetCurrentDir( g_ApplicationDirectory, sizeof( g_ApplicationDirectory ) );
    assert( result );
    if( result )
        {
            spdlog::info( "ApplicationDirectory: {}", g_ApplicationDirectory );
        }
    else
        {
            spdlog::error( "Unable to get current working directory!" );
        }
}

static void
RelativePathToFullPath( const char * relativePathName, char * fullPath )
{
    InitializeFileSystem();
    snprintf( fullPath, FILENAME_MAX, "%s/%s", g_ApplicationDirectory, relativePathName );
}

class FileReadException : public std::runtime_error
{
  public:
    explicit FileReadException( const std::string & message )
        : std::runtime_error( message )
    {
    }
};

static bool
FileExisits( const std::string & fileNameLocal )
{
    fs::path fullPath = fs::path( g_ApplicationDirectory ) / fileNameLocal;

    return fs::exists( fullPath );
}

static std::optional< std::vector< std::uint8_t > >
GetFileData( const std::string & fileNameLocal )
{
    InitializeFileSystem();

    // Construct the full path
    fs::path fullPath = fs::path( g_ApplicationDirectory ) / fileNameLocal;

    // Open file for reading in binary mode
    std::ifstream file( fullPath, std::ios::binary | std::ios::ate );
    if( !file.is_open() )
        {
            spdlog::error( "Unable to open file: {}", fullPath.string() );
            throw FileReadException( "Failed to open file" );
        }
    else if( file.fail() )
        {
            spdlog::error( "Unable to read file: {}", fullPath.string() );
            throw FileReadException( "Failed to read file" );
        }

    // Get file size
    auto fileSize = file.tellg();
    file.seekg( 0, std::ios::beg );

    // Create and size the vector
    std::vector< std::uint8_t > buffer( static_cast< std::size_t >( fileSize ) );

    // Read the data
    if( !file.read( reinterpret_cast< char * >( buffer.data() ), fileSize ) )
        {
            spdlog::error( "Reading file failed: {}", fullPath.string() );
            throw FileReadException( "Failed to read file" );
        }

    spdlog::info( "Successfully read file: {}", fullPath.string() );
    return buffer;
}

#endif // VULKANO_UTILITIES_H

