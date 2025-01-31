#ifndef VULKANO_DESCRIPTOR_H
#define VULKANO_DESCRIPTOR_H

#include "vo_api.hpp"
#include "vo_common.hpp"

class voDeviceContext;
class voDescriptors;
class voBuffer;
class voPipeline;
class voImage;

// ======================================================================================================================
// ============================================ voDescriptors ============================================================
// ======================================================================================================================

/**
 * @class voDescriptor
 * @brief A class that encapsulates a Vulkan Descriptor.
 *
 * @details The voDescriptor class is responsible for managing a Vulkan Descriptor.
 * A descriptor is a data structure that describes some resource, such as an image or buffer, 
 * that can be accessed by the shader stages of a pipeline. 
 * The class provides functionalities for binding images and buffers to a descriptor set, 
 * and binding the descriptor set to a command buffer.
 *
 * @code
 * voDeviceContext deviceContext;
 * voDescriptor descriptor;
 *
 * // Bind an image to the descriptor
 * descriptor.BindImage(imageLayout, imageView, sampler, slot);
 *
 * // Bind a buffer to the descriptor
 * voBuffer buffer;
 * descriptor.BindBuffer(&buffer, offset, size, slot);
 *
 * // Bind the descriptor to a command buffer
 * voPipeline pipeline;
 * descriptor.BindDescriptor(&deviceContext, commandBuffer, &pipeline);
 * @endcode
 *
 * @see `voDeviceContext`, `voBuffer`, `voPipeline`
 */
class VO_API voDescriptor
{
  public:
    voDescriptor();
    ~voDescriptor() = default;

    /**
     * @brief Binds an image to a specific slot in the descriptor set.
     *
     * @param imageLayout The layout of the image to be bound.
     * @param imageView The view of the image to be bound.
     * @param sampler The sampler object to be used for the image.
     * @param slot The slot in the descriptor set to bind the image to.
     */
    void BindImage( VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler, int slot );

    /**
     * @brief Binds a buffer to a specific slot in the descriptor set.
     *
     * @param uniformBuffer The buffer to be bound.
     * @param offset The offset in the buffer to start binding from.
     * @param size The size of the buffer to bind.
     * @param slot The slot in the descriptor set to bind the buffer to.
     */
    void BindBuffer( voBuffer * uniformBuffer, VkDeviceSize offset, VkDeviceSize size, int slot );

    /**
     * @brief Binds the descriptor set to a command buffer.
     *
     * @param device The device context to use for binding.
     * @param vkCommandBuffer The command buffer to bind the descriptor set to.
     * @param pso The pipeline object to use for binding.
     */
    void BindDescriptor( voDeviceContext * device, VkCommandBuffer vkCommandBuffer, voPipeline * pso );

  private:
    friend class voDescriptors;
    voDescriptors * m_parent { nullptr };

    int m_id { -1 }; ///< Id of the descriptor set to be used

    int m_numBuffers { 0 }; ///< Total amount of buffers binded
    static const int MAX_BUFFERS { 16 };
    VkDescriptorBufferInfo m_bufferInfo[MAX_BUFFERS] {};

    int m_numImages { 0 }; ///< Total amount of images binded
    static const int MAX_IMAGEINFO { 16 };
    VkDescriptorImageInfo m_imageInfo[MAX_IMAGEINFO] {};
};

// ======================================================================================================================
// ============================================ voDescriptor ===========================================================
// ======================================================================================================================

/**
 * @class voDescriptors
 * @brief A class that encapsulates a set of Vulkan Descriptors.
 *
 * @details The voDescriptors class is responsible for managing a set of Vulkan Descriptors.
 * A descriptor set is a collection of descriptors, which are used to bind resources like images and buffers to a command buffer.
 * The class provides functionalities for creating a descriptor pool and descriptor set layout, 
 * cleaning up and destroying the descriptor pool and descriptor set layout, and getting a free descriptor from the pool.
 *
 * @see `voDeviceContext`, `voDescriptor`
 */
class VO_API voDescriptors
{
  public:
    voDescriptors()  = default;
    ~voDescriptors() = default;

    /**
    * @struct CreateParms_t
    *
    * Used to configure the `voDescriptors` class.
    * Contains parameters that define the behavior and properties of the descriptor.
    */
    struct CreateParms_t
    {
        uint32_t numUniformsVertex { 0 };
        uint32_t numUniformsFragment { 0 };
        uint32_t numImageSamplers { 0 };
    };
    CreateParms_t m_parms {};

    /**
     * @brief Create a descriptor pool and descriptor set layout.
     *
     * @param device The Vulkan device to create the descriptor pool and layout for.
     * @param parms The creation parameters for the descriptor pool and layout.
     */
    void Create( voDeviceContext * device, const CreateParms_t & parms );

    /**
     * @brief Clean up and destroy the descriptor pool and descriptor set layout.
     *
     * @param device The Vulkan device to destroy the descriptor pool and layout for.
     */
    void Cleanup( voDeviceContext * device );

    /**
     * @brief Get a free descriptor from the pool.
     *
     * @return A voDescriptor object representing a free descriptor in the pool.
     */
    voDescriptor GetFreeDescriptor();

    /* -------------------------------------- Handlers ----------------------------------------------------------------- */

    static const int MAX_DESCRIPTOR_SETS = 256;

    VkDescriptorPool vkDescriptorPool { VK_NULL_HANDLE };
    VkDescriptorSetLayout vkDescriptorSetLayout { VK_NULL_HANDLE };

    int numDescriptorUsed { 0 };                              ///< Number of descriptors in the pool that are in use
    VkDescriptorSet vkDescriptorSets[MAX_DESCRIPTOR_SETS] {}; ///< All descriptor set objects
};

// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================

FORCE_INLINE voDescriptor
voDescriptors::GetFreeDescriptor()
{
    voDescriptor descriptor = {};
    descriptor.m_parent     = this;
    descriptor.m_id         = numDescriptorUsed++ % MAX_DESCRIPTOR_SETS;
    return descriptor;
}

#endif //VULKANO_DESCRIPTOR_H
