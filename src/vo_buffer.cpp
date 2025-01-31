#include "vulkano/vo_buffer.hpp"
#include <cstring>

bool
voBuffer::Allocate(voDeviceContext * device, const void * data, int size, VkBufferUsageFlagBits usageFlags )
{
    vkBufferSize = size;

    /* ----------------------------------------- Create Buffer ---------------------------------------------------------- */
    {
        VkBufferCreateInfo bufferInfo =
        {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = vkBufferSize,                         // Size of buffer (size of 1 vertex * number of vertices)
            .usage       = usageFlags,                           // Multiple types of buffer possible
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,            // Similar to Swap Chain images. Can share buffers (?)
        };

        VK_CHECK( vkCreateBuffer( device->deviceInfo.logical, &bufferInfo, nullptr, &vkBuffer ),
                  "Failed to create buffer" );
    }

    /* ----------------------------------------- Get Buffer Memory Requirements ----------------------------------------- */
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements( device->deviceInfo.logical, vkBuffer, &memRequirements );

       /**
        * 1. VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: Allocated memory is accessible by the host CPU.
        *    It allows us to directly interact with this memory from the host.
        *
        * 2. VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: Ensures simultaneous access to this memory by the host and the device.
        *    It also eliminates the need for explicit flushing or invalidating to synchronize the host and device views of the memory.
        *
        * In summary, we're allocating memory that the host can directly interact with and doesn't require manual synchronization.
        */
        vkMemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkMemoryAllocateInfo allocInfo =
        {
            .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize  = memRequirements.size,
            .memoryTypeIndex = device->FindMemoryTypeIndex( memRequirements.memoryTypeBits, vkMemoryPropertyFlags ),
        };

        VK_CHECK( vkAllocateMemory( device->deviceInfo.logical, &allocInfo, nullptr, &vkBufferMemory ),
                  "Failed to allocate buffer memory" );
    }

    /* ----------------------------------------- Allocate Memory to Buffer ----------------------------------------- */
    {
        if ( data != NULL )
        {
            void * memory = MapBuffer( device );
            memcpy( memory, data, size );
            UnmapBuffer( device );
        }

        vkBindBufferMemory( device->deviceInfo.logical, vkBuffer, vkBufferMemory, 0 );
    }

    return true;
}
