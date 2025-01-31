#include "vulkano/vo_fence.hpp"
#include "vulkano/vo_deviceContext.hpp"
#include <vulkan/vulkan_core.h>


voFence::voFence( voDeviceContext * device )
    : m_device( device )
{
    Create( device );
}

voFence::~voFence()
{
    Wait( m_device );
}




void
voFence::Create( voDeviceContext * device )
{
    VkFenceCreateInfo fenceCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0,
    };

    VK_CHECK( vkCreateFence( device->deviceInfo.logical, &fenceCreateInfo, nullptr, &m_vkFence ),
              "Failed to create fence!" );
}

void
voFence::Wait( voDeviceContext * device )
{
    VK_CHECK( vkWaitForFences( device->deviceInfo.logical, 1, &m_vkFence, VK_TRUE, UINT64_MAX ),
              "Failed to wait for fence!" );

    vkDestroyFence( device->deviceInfo.logical, m_vkFence, nullptr );
}
