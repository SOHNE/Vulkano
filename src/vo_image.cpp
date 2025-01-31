#include "vulkano/vo_image.hpp"
#include "vulkano/vo_deviceContext.hpp"

bool
voImage::Create( voDeviceContext * device, const CreateParms_t & parms )
{
    this->parms = parms;

    /* ------------------------------------------------ Create Image --------------------------------------------------- */
    {
        VkImageCreateInfo image =
            {
                .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_1D,
                .format    = parms.format,
                .extent    = {
                              .width  = parms.width,
                              .height = parms.height,
                              .depth  = parms.depth },
                .mipLevels   = 1,
                .arrayLayers = 1,
                .samples     = VK_SAMPLE_COUNT_1_BIT,
                .tiling      = VK_IMAGE_TILING_OPTIMAL,
                .usage       = parms.usageFlags,
        };

        if( parms.height > 1 )
            {
                image.imageType = VK_IMAGE_TYPE_2D;
            }

        if( parms.depth > 1 )
            {
                image.imageType = VK_IMAGE_TYPE_3D;
            }

        if( VK_FORMAT_D32_SFLOAT == parms.format )
            {
                image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            }
        else
            {
                image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            }

        VK_CHECK( vkCreateImage( device->deviceInfo.logical, &image, VK_NULL_HANDLE, &vkImage ),
                  "Failed to create image" );
    }

    /* ------------------------------------------------ Allocate Memory ------------------------------------------------ */
    {
        VkMemoryRequirements memReqs {};
        vkGetImageMemoryRequirements( device->deviceInfo.logical, vkImage, &memReqs );

        VkMemoryAllocateInfo memAlloc =
            {
                .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize  = memReqs.size,
                .memoryTypeIndex = device->FindMemoryTypeIndex( memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ),
            };

        VK_CHECK( vkAllocateMemory( device->deviceInfo.logical, &memAlloc, VK_NULL_HANDLE, &vkDeviceMemory ),
                  "Failed to allocate memory" );

        VK_CHECK( vkBindImageMemory( device->deviceInfo.logical, vkImage, vkDeviceMemory, 0 ),
                  "Failed to bind image memory" );
    }

    /* ------------------------------------------------ Create Image View ----------------------------------------------- */
    {
        VkImageViewCreateInfo imageView =
            {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image            = vkImage,
                .viewType         = VK_IMAGE_VIEW_TYPE_1D,
                .format           = parms.format,
                .subresourceRange = {
                                     .baseMipLevel   = 0,
                                     .levelCount     = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount     = 1 },
        };

        if( parms.height > 1 )
            {
                imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            }

        if( parms.depth > 1 )
            {
                imageView.viewType = VK_IMAGE_VIEW_TYPE_3D;
            }

        if( VK_FORMAT_D32_SFLOAT == parms.format )
            {
                imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
        else
            {
                imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

        VK_CHECK( vkCreateImageView( device->deviceInfo.logical, &imageView, VK_NULL_HANDLE, &vkImageView ),
                  "Failed to create image view" );
    }

    return true;
}

void
voImage::Cleanup( voDeviceContext * device ) const
{
    vkDestroyImageView( device->deviceInfo.logical, vkImageView, VK_NULL_HANDLE );
    vkDestroyImage( device->deviceInfo.logical, vkImage, VK_NULL_HANDLE );
    vkFreeMemory( device->deviceInfo.logical, vkDeviceMemory, VK_NULL_HANDLE );
}

void
voImage::TransitionLayout( voDeviceContext * device )
{
    // Transition the image layout
    VkCommandBuffer vkCommandBuffer = device->CreateCommandBuffer( VK_COMMAND_BUFFER_LEVEL_PRIMARY );

    VkImageMemoryBarrier barrier =
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask       = 0,
            .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout           = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = vkImage,
            .subresourceRange    = {
                                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel   = 0,
                                    .levelCount     = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount     = 1 },
    };

    if( VK_FORMAT_D32_SFLOAT == parms.format )
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

    VkPipelineStageFlags sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(
        vkCommandBuffer,
        sourceStage, destinationStage,
        0,
        0, VK_NULL_HANDLE,
        0, VK_NULL_HANDLE,
        1, &barrier );

    device->FlushCommandBuffer( vkCommandBuffer, device->m_vkGraphicsQueue );

    vkImageLayout = VK_IMAGE_LAYOUT_GENERAL;
}

void
voImage::TransitionLayout( VkCommandBuffer cmdBuffer, VkImageLayout newLayout )
{
    if( vkImageLayout == newLayout ) return;

    VkImageMemoryBarrier barrier =
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask       = 0,
            .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout           = vkImageLayout,
            .newLayout           = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = vkImage,
            .subresourceRange    = {
                                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel   = 0,
                                    .levelCount     = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount     = 1 },
    };

    if( VK_FORMAT_D32_SFLOAT == parms.format )
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

    VkPipelineStageFlags sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(
        cmdBuffer,
        sourceStage, destinationStage,
        0,
        0, VK_NULL_HANDLE,
        0, VK_NULL_HANDLE,
        1, &barrier );

    vkImageLayout = newLayout;
}
