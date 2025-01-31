#ifndef VULKANO_SAMPELRS_H
#define VULKANO_SAMPELRS_H

#include "vo_api.hpp"
#include <vulkan/vulkan_core.h>

class voDeviceContext;

/**
 * @class voSamplers
 * @brief A class that encapsulates Vulkan Samplers.
 *
 * @details The voSamplers class is responsible for managing Vulkan Samplers.
 * A sampler is a collection of state information and is used to dictate how data is fetched from an image.
 * The class provides functionalities for initializing samplers, cleaning up samplers, and accessing standard and depth samplers.
 *
 * @code
 * voDeviceContext deviceContext;
 *
 * // Initialize samplers
 * voSamplers::InitializeSamplers(&deviceContext);
 *
 * // Access standard sampler
 * VkSampler standardSampler = voSamplers::m_samplerStandard;
 *
 * // Access depth sampler
 * VkSampler depthSampler = voSamplers::m_samplerDepth;
 *
 * // Cleanup
 * voSamplers::Cleanup(&deviceContext);
 * @endcode
 */
class VO_API voSamplers
{
public:
    static bool InitializeSamplers( voDeviceContext * device );
    static void Cleanup( voDeviceContext * device );

    static VkSampler m_samplerStandard;
    static VkSampler m_samplerDepth;
};


#endif //VULKANO_SAMPELRS_H
