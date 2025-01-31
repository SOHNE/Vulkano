#ifndef VULKANO_COMMON_H
#define VULKANO_COMMON_H

#include "vulkano/version.h"

#include <vulkan/vulkan.h>
#include <cassert>
#include "spdlog/spdlog.h"

/// Force a function to be inlined
#ifndef FORCE_INLINE
#    if defined( _MSC_VER )
#        define FORCE_INLINE __forceinline
#    elif defined( __GNUC__ ) || defined( __clang__ )
#        define FORCE_INLINE inline __attribute__( ( always_inline ) )
#    else
#        define FORCE_INLINE inline
#    endif
#endif

#ifdef _WIN32
#    define SPRINTF( buffer, size, format, ... ) sprintf_s( ( buffer ), ( size ), ( format ), __VA_ARGS__ )
#elif __APPLE__ || __linux__
#    define SPRINTF( buffer, size, format, ... ) snprintf( ( buffer ), ( size ), ( format ), __VA_ARGS__ )
#else
#    define SPRINTF( buffer, size, format, ... ) snprintf( ( buffer ), ( size ), ( format ), __VA_ARGS__ )
#endif

/// Memory aligned allocation
#ifndef ALIGNED_ALLOC
#    if defined( _MSC_VER ) || defined( __MINGW32__ ) || defined( __CYGWIN__ )
#        define ALIGNED_ALLOC( size, alignment ) _aligned_malloc( size, alignment )
#        define ALIGNED_FREE( ptr ) _aligned_free( ptr )
#    elif defined( __GNUC__ )
#        define ALIGNED_ALLOC( size, alignment ) aligned_alloc( alignment, size )
#        define ALIGNED_FREE( ptr ) free( ptr )
#    else
#        error "Is aligned allocation supported on this platform?"
#    endif
#endif

#define VO_UNUSED( X ) ( (void)( X ) )
#define voAssert( X ) assert( X )

/**
 * @brief Convert a VkResult to a string
 *
 * @param err The error to convert
 * @return The string representation of the error
 */
[[nodiscard]]
static const char *
VkResultToString( VkResult err )
{
    switch( err )
        {
#define STR( R ) \
    case VK_##R: return #R
            STR( SUCCESS );
            STR( NOT_READY );
            STR( TIMEOUT );
            STR( EVENT_SET );
            STR( EVENT_RESET );
            STR( INCOMPLETE );
            STR( ERROR_OUT_OF_HOST_MEMORY );
            STR( ERROR_OUT_OF_DEVICE_MEMORY );
            STR( ERROR_INITIALIZATION_FAILED );
            STR( ERROR_DEVICE_LOST );
            STR( ERROR_MEMORY_MAP_FAILED );
            STR( ERROR_LAYER_NOT_PRESENT );
            STR( ERROR_EXTENSION_NOT_PRESENT );
            STR( ERROR_FEATURE_NOT_PRESENT );
            STR( ERROR_INCOMPATIBLE_DRIVER );
            STR( ERROR_TOO_MANY_OBJECTS );
            STR( ERROR_FORMAT_NOT_SUPPORTED );
            STR( ERROR_SURFACE_LOST_KHR );
            STR( SUBOPTIMAL_KHR );
            STR( ERROR_OUT_OF_DATE_KHR );
            STR( ERROR_INCOMPATIBLE_DISPLAY_KHR );
            STR( ERROR_NATIVE_WINDOW_IN_USE_KHR );
            STR( ERROR_VALIDATION_FAILED_EXT );
#undef STR
            default: return "UNKNOWN_RESULT";
        }
}

/// Check if a Vulkan function call was successful
#define VK_CHECK( x, ... )                                                                  \
    do                                                                                      \
        {                                                                                   \
            VkResult err = x;                                                               \
            if( err )                                                                       \
                {                                                                           \
                    spdlog::error( "Vulkan error: {} in {}() at {}:{}",                     \
                                   VkResultToString( err ), __func__, __FILE__, __LINE__ ); \
                    if( sizeof( #__VA_ARGS__ ) > 1 )                                        \
                        {                                                                   \
                            spdlog::error( "Message: " __VA_ARGS__ );                       \
                        }                                                                   \
                    std::terminate();                                                       \
                }                                                                           \
        }                                                                                   \
    while( 0 )

#endif // VULKANO_COMMON_H
