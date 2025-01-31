#include "vulkano/vo_swapChain.hpp"
#include <vulkan/vulkan_core.h>
#include <array>
#include "vulkano/vo_deviceContext.hpp"

bool
voSwapChain::Create( voDeviceContext * device, int width, int height )
{
    SetExtent( device->GetPhysicalProperties()->surfaceCapabilities, width, height );

    CreateSemaphores( device );
    CreateSwapchain( device );
    CreateDepthStencil( device );
    CreateRenderPass( device );
    CreateFramebuffers( device );

    return true;
}

void
voSwapChain::Cleanup( voDeviceContext * device )
{
    // semaphores
    {
        vkDestroySemaphore( device->deviceInfo.logical, m_vkRenderFinishedSemaphore, nullptr );
        vkDestroySemaphore( device->deviceInfo.logical, m_vkImageAvailableSemaphore, nullptr );

        m_vkRenderFinishedSemaphore = VK_NULL_HANDLE;
        m_vkImageAvailableSemaphore = VK_NULL_HANDLE;
    }

    // depth buffer
    {
        vkDestroyImageView( device->deviceInfo.logical, m_vkDepthImageView, nullptr );
        vkDestroyImage( device->deviceInfo.logical, m_vkDepthImage, nullptr );
        vkFreeMemory( device->deviceInfo.logical, m_vkDepthImageMemory, nullptr );

        m_vkDepthImageView   = VK_NULL_HANDLE;
        m_vkDepthImage       = VK_NULL_HANDLE;
        m_vkDepthImageMemory = VK_NULL_HANDLE;
    }

    // frame buffer
    {
        for( auto & framebuffer : m_framebuffers )
            {
                vkDestroyFramebuffer( device->deviceInfo.logical, framebuffer, nullptr );
            }
        m_framebuffers.clear();
    }

    // render pass
    {
        vkDestroyRenderPass( device->deviceInfo.logical, m_vkRenderPass, nullptr );
        m_vkRenderPass = VK_NULL_HANDLE;
    }

    // color buffers
    {
        for( auto & buffer : m_buffers )
            {
                vkDestroyImageView( device->deviceInfo.logical, buffer.view, nullptr );
            }
        m_buffers.clear();
    }

    // swapchain
    {
        vkDestroySwapchainKHR( device->deviceInfo.logical, m_vkSwapChain, nullptr );
        m_vkSwapChain = VK_NULL_HANDLE;
    }
}

void
voSwapChain::Resize( voDeviceContext * device, int width, int height )
{
    vkDeviceWaitIdle( device->deviceInfo.logical );

    m_resized = true;

    SetExtent( device->GetPhysicalProperties()->surfaceCapabilities, width, height );
    CreateSwapchain( device );
    CreateDepthStencil( device );
    CreateFramebuffers( device );
}

uint32_t
voSwapChain::BeginFrame( voDeviceContext * device )
{
    m_currentImageIndex = 0;

    // Get image index
    {
        VkResult result = vkAcquireNextImageKHR( device->deviceInfo.logical, m_vkSwapChain, UINT64_MAX, m_vkImageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex );
        voAssert( ( VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result ) &&
                  "Failed to acquire swap chain image" );
    }

    // Reset the command buffer
    vkResetCommandBuffer( device->m_vkCommandBuffers[m_currentImageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

    // Begin recording draw commands
    VkCommandBufferBeginInfo beginInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        };

    vkBeginCommandBuffer( device->m_vkCommandBuffers[m_currentImageIndex], &beginInfo );

    return m_currentImageIndex;
}

void
voSwapChain::EndFrame( voDeviceContext * device )
{
    VK_CHECK( vkEndCommandBuffer( device->m_vkCommandBuffers[m_currentImageIndex] ),
              "Failed to record command buffer" );

    /* ------------------------------------------------ Submit ------------------------------------------------------------ */
    {
        VkPipelineStageFlags waitStages[1] =
            {
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            };

        VkSubmitInfo submitInfo =
            {
                .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount   = 1,
                .pWaitSemaphores      = &m_vkImageAvailableSemaphore,
                .pWaitDstStageMask    = waitStages,
                .commandBufferCount   = 1,
                .pCommandBuffers      = &device->m_vkCommandBuffers[m_currentImageIndex],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores    = &m_vkRenderFinishedSemaphore,
            };

        VK_CHECK( vkQueueSubmit( device->m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ),
                  "Failed to submit queue" );
    }

    /* ------------------------------------------------ Present ------------------------------------------------------------ */
    {
        VkPresentInfoKHR presentInfo =
            {
                .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores    = &m_vkRenderFinishedSemaphore,
                .swapchainCount     = 1,
                .pSwapchains        = &m_vkSwapChain,
                .pImageIndices      = &m_currentImageIndex,
            };

        VkResult result = vkQueuePresentKHR( device->presentQueue, &presentInfo );
        if( m_resized || VK_SUBOPTIMAL_KHR == result || VK_ERROR_OUT_OF_DATE_KHR == result )
            {
                m_resized = false;
                Resize( device, m_width, m_height );
                return;
            }

        voAssert( VK_SUCCESS == result && "Failed to acquire swap chain image" );
    }

    vkQueueWaitIdle( device->presentQueue );
}

void
voSwapChain::BeginRenderPass( voDeviceContext * device )
{
    /* ------------------------------------------------ Render Pass ------------------------------------------------------------ */
    {
        VkClearValue clearValues[2];
        clearValues[0].color        = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo =
            {
                .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass  = m_vkRenderPass,
                .framebuffer = m_framebuffers[m_currentImageIndex],
                .renderArea  = {
                                .offset = { 0, 0 },
                                .extent = m_vkExtent,
                                },
                .clearValueCount = 2,
                .pClearValues    = clearValues,
        };
        vkCmdBeginRenderPass( device->m_vkCommandBuffers[m_currentImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    /* ------------------------------------------------ Viewport --------------------------------------------------------------- */
    {
        VkViewport viewport = {
            .x        = 0.0F,
            .y        = 0.0F,
            .width    = static_cast< float >( m_width ),
            .height   = static_cast< float >( m_height ),
            .minDepth = 0.0F,
            .maxDepth = 1.0F,
        };
        vkCmdSetViewport( device->m_vkCommandBuffers[m_currentImageIndex], 0, 1, &viewport );
    }

    /* ----------------------------------------------- Scissor ------------------------------------------------------------------- */
    {
        VkRect2D scissor = {
            .offset = {
                       .x = 0,
                       .y = 0,
                       },
            .extent = {
                       .width  = static_cast< uint32_t >( m_width ),
                       .height = static_cast< uint32_t >( m_height ),
                       }
        };
        vkCmdSetScissor( device->m_vkCommandBuffers[m_currentImageIndex], 0, 1, &scissor );
    }
}

void
voSwapChain::EndRenderPass( voDeviceContext * device ) const
{
    vkCmdEndRenderPass( device->m_vkCommandBuffers[m_currentImageIndex] );
}

void
voSwapChain::CreateSemaphores( voDeviceContext * device )
{
    VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VK_CHECK( vkCreateSemaphore( device->deviceInfo.logical, &semaphoreInfo, nullptr, &m_vkImageAvailableSemaphore ),
              "Failed to create semaphore!" );

    VK_CHECK( vkCreateSemaphore( device->deviceInfo.logical, &semaphoreInfo, nullptr, &m_vkRenderFinishedSemaphore ),
              "Failed to create semaphore!" );
}

void
voSwapChain::CreateSwapchain( voDeviceContext * device )
{
    /* -------------------------------------- Swapchain Config ------------------------------------------------------ */
    const physical_device_properties_t * physicalDeviceInfo = device->GetPhysicalProperties();

    // Get best suitable format and mode
    const VkSurfaceFormatKHR surfaceFormat = ChooseBestSurfaceFormat( physicalDeviceInfo->surfaceFormats );
    const VkPresentModeKHR   presentMode   = ChooseBestPresentationMode( physicalDeviceInfo->presentModes );

    // If there's a maximum image count and we've exceeded it, clamp to the max
    uint32_t imageCount = physicalDeviceInfo->surfaceCapabilities.minImageCount + 1;
    if( physicalDeviceInfo->surfaceCapabilities.maxImageCount > 0 && imageCount > physicalDeviceInfo->surfaceCapabilities.maxImageCount )
        {
            imageCount = physicalDeviceInfo->surfaceCapabilities.maxImageCount;
        }

    // Find a supported composite alpha format (not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR compositeAlpha = ChooseCompositeAlpha( physicalDeviceInfo->surfaceCapabilities );

    // Stores previous swapchain for resources reuse (also, it allows present already acquired images)
    VkSwapchainKHR oldSwapchain = m_vkSwapChain;

    // Specify the transformation to apply to images in the swapchain before they are presented
    VkSurfaceTransformFlagsKHR preTransform { physicalDeviceInfo->surfaceCapabilities.currentTransform };
    if( physicalDeviceInfo->surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
        {
            // Non-rotated transform, if possible
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }

    VkSwapchainCreateInfoKHR createInfo =
        {
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,                  // Structure type identifier
            .surface               = device->VkSurface,                                            // Swapchain surface
            .minImageCount         = imageCount,                                                   // Minimum images in swap chain
            .imageFormat           = surfaceFormat.format,                                         // Swap chain image format
            .imageColorSpace       = surfaceFormat.colorSpace,                                     // Swap chain color space
            .imageExtent           = m_vkExtent,                                                   // Swap chain image size
            .imageArrayLayers      = 1,                                                            // Number of layers for each image in chain
            .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                          // What attachment images will be used as (e.g. color, depth, stencil)
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,                                    // Image sharing mode, exclusive means images can be used by one queue family at a time
            .queueFamilyIndexCount = 0,                                                            // Number of queue families that will be using the images in the swap chain
            .pQueueFamilyIndices   = VK_NULL_HANDLE,                                               // Array of queue family indices to use images
            .preTransform          = static_cast< VkSurfaceTransformFlagBitsKHR >( preTransform ), // Transformations to be applied to images in chain
            .compositeAlpha        = compositeAlpha,                                               // How to handle blending images with external graphics
            .presentMode           = presentMode,                                                  // How images should be presented to the screen
            .clipped               = VK_TRUE,                                                      // Whether to clip parts of images not in view (e.g. behind another window, off screen, etc)
            .oldSwapchain          = oldSwapchain                                                  // Reuse previous swapchain
        };

    // Adjust sharing mode for concurrent queues
    if( device->queueIds.graphicsFamily != device->queueIds.presentationFamily )
        {
            // Use std::array to store queue family indices
            std::array< uint32_t, 2 > queueFamilyIndices =
                {
                    static_cast< uint32_t >( device->queueIds.graphicsFamily ),
                    static_cast< uint32_t >( device->queueIds.presentationFamily ),
                };

            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast< uint32_t >( queueFamilyIndices.size() );
            createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
        }

    // Toggle on transfer source on swap chain images, if supported
    if( physicalDeviceInfo->surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT )
        {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

    // Toggle on transfer destination on swap chain images, if supported
    if( physicalDeviceInfo->surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT )
        {
            createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

    /* -------------------------------------- Create Swapchain ---------------------------------------------------------- */
    VK_CHECK( vkCreateSwapchainKHR( device->deviceInfo.logical, &createInfo, VK_NULL_HANDLE, &m_vkSwapChain ),
              "Failed to create swap chain" );

    /* -------------------------------------- Destroy previous Swapchain ------------------------------------------------ */
    // If recreating a swapchain, destroy the previous one.
    // Cleans all presentable images
    if( VK_NULL_HANDLE != oldSwapchain )
        {
            for( auto & buffer : m_buffers )
                {
                    vkDestroyImageView( device->deviceInfo.logical, buffer.view, VK_NULL_HANDLE );
                }

            // swapchain
            vkDestroySwapchainKHR( device->deviceInfo.logical, oldSwapchain, VK_NULL_HANDLE );
        }

    /* -------------------------------------- Color images -------------------------------------------------------------- */
    VK_CHECK( vkGetSwapchainImagesKHR( device->deviceInfo.logical, m_vkSwapChain, &imageCount, VK_NULL_HANDLE ),
              "Failed to get Swapchain images count" );

    std::vector< VkImage > images { imageCount };

    VK_CHECK( vkGetSwapchainImagesKHR( device->deviceInfo.logical, m_vkSwapChain, &imageCount, images.data() ),
              "Failed to get Swapchain images" );

    // Set surface format
    m_vkColorImageFormat = surfaceFormat.format;

    /* -------------------------------------- Create Color Image Views ------------------------------------------------------ */
    m_buffers.resize( imageCount );

    for( uint32_t i = 0; i < imageCount; i++ )
        {
            m_buffers[i].image = images[i];

            /**
            * `VkImageViewCreateInfo` specifies the parameters for creating an image view in Vulkan.
            * Is a way of interpreting or viewing an image's data in a manner that is most suitable for a specific use case.
            * It allows to specify how to access the image and which part of the image to access.
            */
            VkImageViewCreateInfo viewInfo =
                {
                    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image      = m_buffers[i].image, // Image to create view for
                    .viewType   = VK_IMAGE_VIEW_TYPE_2D, // Type of image (1D, 2D, 3D, Cube, etc)
                    .format     = m_vkColorImageFormat, // Format of the image data
                    .components = {
                                   .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   },
                    // What part of the image to view
                    .subresourceRange = {
                        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, // Which aspect of the image to view (e.g. COLOR_BIT for viewing color)
                        .baseMipLevel   = 0,                         // Start mipmap level to view from
                        .levelCount     = 1,                         // Number of mipmap levels to view
                        .baseArrayLayer = 0,                         // Start array level to view from
                        .layerCount     = 1,                         // Number of array levels to view
                    },
            };

            VK_CHECK( vkCreateImageView( device->deviceInfo.logical, &viewInfo, nullptr, &m_buffers[i].view ),
                      "Failed to create texture image view" );
        }
}

void
voSwapChain::CreateDepthStencil( voDeviceContext * device )
{
    // Clear previous depth image
    if( VK_NULL_HANDLE != m_vkDepthImageView )
        {
            vkDestroyImageView( device->deviceInfo.logical, m_vkDepthImageView, nullptr );
            vkDestroyImage( device->deviceInfo.logical, m_vkDepthImage, nullptr );
            vkFreeMemory( device->deviceInfo.logical, m_vkDepthImageMemory, nullptr );
        }

    /* -------------------------------------- Depth Format ------------------------------------------------------------ */
    {
        const bool validFormat = ChooseBestDepthFormat( device->deviceInfo.physical, &m_vkDepthFormat );
        voAssert( validFormat && "Failed to get a suitable depth format" );
    }

    /* -------------------------------------- Create Depth Image ------------------------------------------------------ */
    {
        /**
         * `VkImageCreateInfo` defines the parameters for creating a Vulkan Image object.
         * This includes properties such as image type (1D, 2D, 3D), format, extent (dimensions), and usage.
         * By setting these properties, we can control how the image will be stored in memory and how it will be used in shader stages.
         */
        VkImageCreateInfo imageInfo =
            {
                .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D, // Type of image (1D, 2D, or 3D)
                .format    = m_vkDepthFormat, // Format type of the image
                .extent    = {
                              .width  = m_vkExtent.width,  // Width of the image extent
                       .height = m_vkExtent.height, // Height of the image extent
                       .depth  = 1,                 // 2D image, so depth must be 1 (no 3D aspect)
                },
                .mipLevels     = 1, // Number of mip levels
                .arrayLayers   = 1, // Number of layers (Used for multi-layer images, like stereoscopic 3D or cube maps)
                .samples       = VK_SAMPLE_COUNT_1_BIT, // Number of samples for multi-sampling
                .tiling        = VK_IMAGE_TILING_OPTIMAL, // How the data should be tiled
                .usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, // Bit flags defining what the image will be used for
                .sharingMode   = VK_SHARING_MODE_EXCLUSIVE, // Whether the image can be shared between queues
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Layout of the image data on creation
        };

        VK_CHECK( vkCreateImage( device->deviceInfo.logical, &imageInfo, nullptr, &m_vkDepthImage ),
                  "Failed to create image" );

        /**
         * `VkMemoryRequirements` provides information about the memory requirements of an image or buffer,
         * including size, alignment, and memory type indices.
         */
        VkMemoryRequirements memRequirements {};
        vkGetImageMemoryRequirements( device->deviceInfo.logical, m_vkDepthImage, &memRequirements );

        VkMemoryAllocateInfo allocInfo =
            {
                .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize  = memRequirements.size,
                .memoryTypeIndex = device->FindMemoryTypeIndex( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ),
            };

        VK_CHECK( vkAllocateMemory( device->deviceInfo.logical, &allocInfo, nullptr, &m_vkDepthImageMemory ),
                  "Failed to allocate image memory" );

        /**
         * `vkBindImageMemory` associates an image with a memory allocation,
         * effectively assigning the memory to store the image data.
         */
        VK_CHECK( vkBindImageMemory( device->deviceInfo.logical, m_vkDepthImage, m_vkDepthImageMemory, 0 ),
                  "Failed to bind image memory" );
    }

    /* -------------------------------------- Create Depth Image View ------------------------------------------------------ */
    {
        VkImageViewCreateInfo viewInfo =
            {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image            = m_vkDepthImage,
                .viewType         = VK_IMAGE_VIEW_TYPE_2D,
                .format           = m_vkDepthFormat,
                .subresourceRange = {
                                     .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
                                     .baseMipLevel   = 0,
                                     .levelCount     = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount     = 1,
                                     },
        };

        VK_CHECK( vkCreateImageView( device->deviceInfo.logical, &viewInfo, nullptr, &m_vkDepthImageView ),
                  "Failed to create texture image view" );
    }
}

void
voSwapChain::CreateRenderPass( voDeviceContext * device )
{

    /* -------------------------------------- Attachments ------------------------------------------------------ */
    /**
     * `VkAttachmentDescription` specifies the properties of an attachment (like a color buffer or depth buffer),
     * including its format, samples, usage, and how its contents should be handled throughout the rendering process.
     * This is **crucial** in setting up render pass configurations in Vulkan.
     *
     */
    VkAttachmentDescription colorAttachment =
        {
            .format  = m_vkColorImageFormat,  // Format to use for the attachment
            .samples = VK_SAMPLE_COUNT_1_BIT, // Number of samples to write for multisampling

            .loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,  // Describes what to do with the attachment before rendering
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // Describes what to do with the attachment after rendering

            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // Describes what to do with the stencil before rendering
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // Describes what to do with the stencil after rendering

            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,       // Image data layout before render pass
            .finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Image data layout after render pass
        };

    VkAttachmentDescription depthAttachment =
        {
            .format         = m_vkDepthFormat,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

    std::array< VkAttachmentDescription, 2 > attachments =
        {
            colorAttachment,
            depthAttachment,
        };

    /* -------------------------------------- Attachments References ------------------------------------------------------ */
    /**
     * `VkAttachmentReference` references an attachment (like a color buffer or depth buffer) in a render pass.
     * It specifies the index of the attachment in the `pAttachments` array and the layout the attachment will be in during a subpass.
     * This allows the attachment to be accessed by the shaders during rendering.
     */
    VkAttachmentReference colorAttachmentRef =
        {
            .attachment = 0,                                        // Attachment index that this reference refers to
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // Layout to use when the attachment is referred to
        };

    VkAttachmentReference depthAttachmentRef =
        {
            .attachment = 1,
            .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

    /* -------------------------------------- Subpasses -------------------------------------------------------------------- */
    /**
     * `VkSubpassDescription` describes the contents of a subpass within a render pass.
     * It specifies the color, depth/stencil, input, and resolve attachments that will be used in the subpass, as well as how they should be used.
     * This is crucial for setting up multiple rendering operations that can be batched together for efficiency.
     */
    VkSubpassDescription subpass =
        {
            .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount    = 1,
            .pColorAttachments       = &colorAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
        };

    /* -------------------------------------- Dependencies -------------------------------------------------------------------- */
    /**
     * `VkSubpassDependency` describes the dependencies between subpasses in a render pass.
     * It specifies the source and destination subpasses and how operations in the source subpass can affect those in the destination subpass.
     * This is important for synchronizing operations within a render pass, ensuring that operations are executed in the correct order.
     */
    VkSubpassDependency dependency =
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,

            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

    /* -------------------------------------- Render Pass -------------------------------------------------------------------- */
    VkRenderPassCreateInfo renderPassInfo =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,

            .attachmentCount = static_cast< uint32_t >( attachments.size() ),
            .pAttachments    = attachments.data(),

            .subpassCount = 1,
            .pSubpasses   = &subpass,

            .dependencyCount = 1,
            .pDependencies   = &dependency,
        };

    VK_CHECK( vkCreateRenderPass( device->deviceInfo.logical, &renderPassInfo, nullptr, &m_vkRenderPass ),
              "Failed to create the render pass" );
}

void
voSwapChain::CreateFramebuffers( voDeviceContext * device )
{
    /* -------------------------------------- Cleanup Framebuffers ------------------------------------------------------ */
    if( !m_framebuffers.empty() )
        {
            for( auto & framebuffer : m_framebuffers )
                {
                    vkDestroyFramebuffer( device->deviceInfo.logical, framebuffer, nullptr );
                }
            m_framebuffers.clear();
        }

    /* -------------------------------------- Create Framebuffers ------------------------------------------------------ */
    m_framebuffers.resize( m_buffers.size() );

    for( size_t i = 0; i < m_buffers.size(); i++ )
        {
            VkImageView attachments[2] =
                {
                    m_buffers[i].view,
                    m_vkDepthImageView,
                };

            VkFramebufferCreateInfo framebufferInfo =
                {
                    .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass      = m_vkRenderPass,
                    .attachmentCount = 2,
                    .pAttachments    = attachments,
                    .width           = m_vkExtent.width,
                    .height          = m_vkExtent.height,
                    .layers          = 1,
                };

            VK_CHECK( vkCreateFramebuffer( device->deviceInfo.logical, &framebufferInfo, nullptr, &m_framebuffers[i] ),
                      "Failed to create the frame buffer" );
        }
}

VkSurfaceFormatKHR
voSwapChain::ChooseBestSurfaceFormat( const std::vector< VkSurfaceFormatKHR > & InFormats )
{
    // List of formats in request order
    const std::array< VkFormat, 8 > formats = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM,
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8_SRGB,
        VK_FORMAT_R8G8B8_SRGB,
    };

    // Color space to request
    const VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    // Handle empty or undefined format list
    if( InFormats.empty() )
        {
            throw std::runtime_error( "No surface formats available" );
        }

    // Special case: Undefined single format
    if( InFormats.size() == 1 && InFormats[0].format == VK_FORMAT_UNDEFINED )
        {
            return { formats[0], color_space };
        }

    // Try to find a format from the preferred list
    for( const auto & preferredFormat : formats )
        {
            for( const auto & availableFormat : InFormats )
                {
                    if( availableFormat.format == preferredFormat &&
                        availableFormat.colorSpace == color_space )
                        {
                            return availableFormat;
                        }
                }
        }

    // If no preferred format is found, return the first available format
    return InFormats[0];
}

VkPresentModeKHR
voSwapChain::ChooseBestPresentationMode( const std::vector< VkPresentModeKHR > & InPresentationModes )
{
    // Preferred presentation modes in order of priority
    const std::array< VkPresentModeKHR, 3 > PreferredModes = {
        VK_PRESENT_MODE_MAILBOX_KHR,   // Low-latency triple buffering
        VK_PRESENT_MODE_IMMEDIATE_KHR, // Lowest latency, potential screen tearing
        VK_PRESENT_MODE_FIFO_KHR       // Guaranteed fallback, V-Sync
    };

    // Find the first available preferred mode
    for( VkPresentModeKHR PreferredMode : PreferredModes )
        {
            // Check if the current preferred mode is supported
            auto ModeIterator = std::find(
                InPresentationModes.begin(),
                InPresentationModes.end(),
                PreferredMode );

            // If mode is found, return it
            if( ModeIterator != InPresentationModes.end() )
                {
                    return PreferredMode;
                }
        }

    // Fallback to FIFO if no preferred modes are available
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
voSwapChain::ChooseSwapExtent( const VkSurfaceCapabilitiesKHR & InCap, int width, int height )
{
    // If the surface size is already defined by the system, use the current extent
    if( UINT32_MAX != InCap.currentExtent.width )
        {
            return InCap.currentExtent;
        }

    // Create an actual extent based on the requested width and height
    VkExtent2D actualExtent = {
        static_cast< uint32_t >( width ),
        static_cast< uint32_t >( height ) };

    // Clamp the width to the allowed range
    actualExtent.width = std::clamp(
        actualExtent.width,
        InCap.minImageExtent.width,
        InCap.maxImageExtent.width );

    // Clamp the height to the allowed range
    actualExtent.height = std::clamp(
        actualExtent.height,
        InCap.minImageExtent.height,
        InCap.maxImageExtent.height );

    return actualExtent;
}

VkBool32
voSwapChain::ChooseBestDepthFormat( VkPhysicalDevice & InPhysicalDevice, VkFormat * OutDepthFormat )
{
    // Prioritized list of depth formats, from most preferred to least preferred
    const std::array< VkFormat, 5 > PreferredDepthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT, // High precision with stencil
        VK_FORMAT_D32_SFLOAT,         // High precision
        VK_FORMAT_D24_UNORM_S8_UINT,  // Good precision with stencil
        VK_FORMAT_D16_UNORM_S8_UINT,  // Lower precision with stencil
        VK_FORMAT_D16_UNORM           // Lowest precision
    };

    // Iterate through preferred formats
    for( const VkFormat & format : PreferredDepthFormats )
        {
            // Query format properties
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(
                InPhysicalDevice,
                format,
                &formatProperties );

            // Check if format supports depth/stencil attachment
            if( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
                {
                    // Found a suitable format
                    *OutDepthFormat = format;
                    return VK_TRUE;
                }
        }

    // No suitable depth format found
    *OutDepthFormat = VK_FORMAT_UNDEFINED;
    return VK_FALSE;
}

VkCompositeAlphaFlagBitsKHR
voSwapChain::ChooseCompositeAlpha( const VkSurfaceCapabilitiesKHR & InCap )
{
    // Prioritized composite alpha flags in order of preference
    const std::array< VkCompositeAlphaFlagBitsKHR, 4 > PreferredCompositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,          // Fully opaque surface
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,  // Pre-multiplied alpha blending
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, // Post-multiplied alpha blending
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR          // Inherit alpha from parent window
    };

    // Find the first supported composite alpha flag
    for( const auto & compositeAlphaFlag : PreferredCompositeAlphaFlags )
        {
            // Check if the current flag is supported by the surface
            if( InCap.supportedCompositeAlpha & compositeAlphaFlag )
                {
                    return compositeAlphaFlag;
                }
        }

    // Fallback to the first available flag if no preferred flags are supported
    return PreferredCompositeAlphaFlags[0];
}
