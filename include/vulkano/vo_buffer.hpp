#ifndef VULKANO_BUFFER_H
#define VULKANO_BUFFER_H

#include "vo_api.hpp"
#include "vo_common.hpp"
#include "vo_deviceContext.hpp"

/**
 * @class voBuffer
 * @brief Encapsulates a general purpose Vulkan buffer.
 */
class VO_API voBuffer
{
public:
    voBuffer() = default;
    ~voBuffer() = default;

    /* -------------------------------------- Base --------------------------------------------------------------------- */

    /**
     * @brief Allocates memory for the buffer.
     * @param device The device context.
     * @param data Pointer to the data to be stored in the buffer.
     * @param size Size of the data.
     * @param usageFlags Usage flags for the buffer.
     * @return True if allocation is successful, false otherwise.
     */
    bool Allocate( voDeviceContext * device, const void * data, int size, VkBufferUsageFlagBits usageFlags );

    /** @brief Cleanup the wrapped buffer. */
    void Cleanup( voDeviceContext * device ) const;

    /* -------------------------------------- Mappings ----------------------------------------------------------------- */

    /**
     * @brief Maps the buffer memory for CPU access.
     * @param device The device context.
     * @return A pointer to the mapped buffer memory.
     *
     * @details Allows the CPU to access a region of a device memory object.
     * Establishes a logical pathway between the CPU and a region of a device memory object.
     * This pathway, or 'mapping', is necessary when the CPU requires direct access to the data contained within the buffer.
     * It's important to note that not all memory can be mapped for CPU access. The memory type must have the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT property.
     * After calling this function, the returned pointer can be used to load data into the buffer.
     */
    void * MapBuffer( voDeviceContext * device ) const;

    /**
     * @brief Unmaps the buffer memory.
     * @param device The device context.
     *
     * @details Undoes the mapping of a device memory object.
     * Once we're done with reading from or writing to the mapped memory, we should unmap it.
     * Ensure the termination of this connection to prevent potential memory leaks and undefined behavior.
     * It is also necessary to ensure that all operations involving the mapped memory have completed before this call.
     */
    void UnmapBuffer( voDeviceContext * device ) const;

    /* -------------------------------------- Handlers ----------------------------------------------------------------- */

    VkBuffer              vkBuffer              { VK_NULL_HANDLE }; ///< Buffer handle.
    VkDeviceMemory        vkBufferMemory        { VK_NULL_HANDLE }; ///< Device memory handle.
    VkDeviceSize          vkBufferSize          { 0 };
    VkMemoryPropertyFlags vkMemoryPropertyFlags { 0 };
};


// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================

FORCE_INLINE void
voBuffer::Cleanup(voDeviceContext * device ) const
{
    vkDestroyBuffer( device->deviceInfo.logical, vkBuffer, nullptr );
    vkFreeMemory( device->deviceInfo.logical, vkBufferMemory, nullptr );
}

FORCE_INLINE void *
voBuffer::MapBuffer(voDeviceContext * device ) const
{
    void * mapped_ptr   = nullptr;
    uint32_t byteOffset = 0;
    vkMapMemory( device->deviceInfo.logical, vkBufferMemory, byteOffset, vkBufferSize, 0, &mapped_ptr );
    return mapped_ptr;
}

FORCE_INLINE void
voBuffer::UnmapBuffer(voDeviceContext * device ) const
{
    vkUnmapMemory( device->deviceInfo.logical, vkBufferMemory );
}

#endif //VULKANO_BUFFER_H
