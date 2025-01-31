#include "vulkano/vo_samplers.hpp"

#include "vulkano/vo_deviceContext.hpp"

VkSampler voSamplers::m_samplerStandard { VK_NULL_HANDLE };
VkSampler voSamplers::m_samplerDepth { VK_NULL_HANDLE };


bool
voSamplers::InitializeSamplers( voDeviceContext * device )
{
    {
        VkSamplerCreateInfo samplerInfo =
        {
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter               = VK_FILTER_LINEAR,
            .minFilter               = VK_FILTER_LINEAR,
            .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = 16,
            .compareEnable           = VK_FALSE,
            .compareOp               = VK_COMPARE_OP_ALWAYS,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VK_CHECK( vkCreateSampler( device->deviceInfo.logical, &samplerInfo, nullptr, &m_samplerStandard ),
                  "Failed to create m_samplerStandard" );
    }

    {
        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        VkSamplerCreateInfo samplerInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter     = VK_FILTER_LINEAR,
            .minFilter     = VK_FILTER_LINEAR,
            .mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias    = 0.0f,
            .maxAnisotropy = 1.0f,
            .minLod        = 0.0f,
            .maxLod        = 1.0f,
            .borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        };

        VK_CHECK( vkCreateSampler( device->deviceInfo.logical, &samplerInfo, nullptr, &m_samplerDepth ),
                  "Failed to create m_samplerDepth!" );
    }

    return true;
}

void
voSamplers::Cleanup( voDeviceContext * device )
{
    vkDestroySampler( device->deviceInfo.logical, m_samplerStandard, nullptr );
    vkDestroySampler( device->deviceInfo.logical, m_samplerDepth, nullptr );
}
