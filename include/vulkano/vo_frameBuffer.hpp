#ifndef VULKANO_FRAMEBUFFER_H
#define VULKANO_FRAMEBUFFER_H

#include "vo_api.hpp"
#include "vo_deviceContext.hpp"
#include "vo_image.hpp"
#include <vulkan/vulkan_core.h>


// TODO: Do a search for best suitable depth format based on physical device properties
#define DEPTH_FORMAT VK_FORMAT_D32_SFLOAT


/**
 * @class voFrameBuffer
 * @brief A class that encapsulates a Vulkan Frame Buffer.
 *
 * @details The voFrameBuffer class is responsible for managing a Vulkan Frame Buffer.
 * A frame buffer is a collection of memory buffers that serve as the destination for rendering operations.
 * The class provides functionalities for creating a frame buffer with given parameters, resizing the frame buffer and its attachments,
 * cleaning up the frame buffer, beginning and ending the render pass for the frame buffer, and creating the render pass for the frame buffer.
 *
 * @code
 * voDeviceContext deviceContext;
 * voFrameBuffer frameBuffer;
 *
 * // Create a frame buffer with given parameters
 * voFrameBuffer::CreateParms_t parms = { width, height, hasDepth, hasColor, clearColor, clearDepthStencil };
 * frameBuffer.Create(&deviceContext, parms);
 *
 * // Resize the frame buffer and its attachments
 * frameBuffer.Resize(&deviceContext, newWidth, newHeight);
 *
 * // Begin the render pass for the frame buffer
 * frameBuffer.BeginRenderPass(&deviceContext, cmdBufferIndex);
 *
 * // End the render pass for the frame buffer
 * frameBuffer.EndRenderPass(&deviceContext, cmdBufferIndex);
 *
 * // Cleanup
 * frameBuffer.Cleanup(&deviceContext);
 * @endcode
 *
 * @see `voDeviceContext`, `voImage`
 */
class VO_API voFrameBuffer
{
public:
    voFrameBuffer() = default;
    ~voFrameBuffer() = default;

    /**
     * @struct CreateParms_t
     * @brief Parameters for creating a Vulkan render framebuffer
     */
    struct VO_API CreateParms_t
    {
        uint32_t width        { 0 };      ///< Width of the framebuffer
        uint32_t height       { 0 };      ///< Height of the framebuffer
        uint8_t  hasDepth : 1 { false };  ///< Whether the framebuffer has a depth attachment
        uint8_t  hasColor : 1 { false };  ///< Whether the framebuffer has a color attachment

        VkClearColorValue         clearColor        { };                         ///< The clear color value for the framebuffer
        VkClearDepthStencilValue  clearDepthStencil { 1.0F, 0 }; ///< The clear depthStencil value for the framebuffer
    };

    /**
     * @brief Creates the framebuffer with the given parameters
     * @param device The Vulkan device context
     * @param parms The parameters for creating the framebuffer
     * @return True if the framebuffer was created successfully, false otherwise
     */
    bool Create( voDeviceContext * device, CreateParms_t & parms );

    /**
     * @brief Resizes the framebuffer and its attachments.
     * @param device The Vulkan device context.
     * @param width The new width of the framebuffer.
     * @param height The new height of the framebuffer.
     */
    void Resize( voDeviceContext * device, const uint32_t & width, const uint32_t & height );

    /**
     * @brief Cleans up the framebuffer
     * @param device The Vulkan device context
     */
    void Cleanup( voDeviceContext * device );

    /**
     * @brief Begins the render pass for the framebuffer
     * @param device The Vulkan device context
     * @param cmdBufferIndex The index of the command buffer
     */
    void BeginRenderPass( voDeviceContext * device, int cmdBufferIndex );

    /**
     * @brief Ends the render pass for the framebuffer
     * @param device The Vulkan device context
     * @param cmdBufferIndex The index of the command buffer
     */
    void EndRenderPass( voDeviceContext * device, int cmdBufferIndex );

    CreateParms_t parms { 0 };

    voImage imageDepth { }; ///< The depth attachment for the framebuffer
    voImage imageColor { }; ///< The color attachment for the framebuffer

    VkFramebuffer vkFrameBuffer { VK_NULL_HANDLE };
    VkRenderPass  vkRenderPass  { VK_NULL_HANDLE };

private:
    /** @brief Creates the render pass for the framebuffer */
    void CreateRenderPass( voDeviceContext * device );
};

#endif //VULKANO_FRAMEBUFFER_H
