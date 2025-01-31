#include "vulkano/vo_frameBuffer.hpp"
#include <array>


#define SHADOW_BIAS  ( 1.25F )
#define SHADOW_SLOPE ( 1.75F )


void
voFrameBuffer::Resize( voDeviceContext * device, const uint32_t & width, const uint32_t & height )
{
    voAssert( vkFrameBuffer != VK_NULL_HANDLE );

    voFrameBuffer::CreateParms_t new_parms = parms;
    new_parms.width = width;
    new_parms.height = height;

    Cleanup( device );
    Create( device, new_parms );
}

void
voFrameBuffer::Cleanup( voDeviceContext * device )
{
    if ( parms.hasDepth )
    {
        imageDepth.Cleanup( device );
    }

    if ( parms.hasColor )
    {
        imageColor.Cleanup( device );
    }

    vkDestroyFramebuffer( device->deviceInfo.logical, vkFrameBuffer, nullptr );
    vkDestroyRenderPass( device->deviceInfo.logical, vkRenderPass, nullptr );

    vkFrameBuffer = VK_NULL_HANDLE;
    vkRenderPass = VK_NULL_HANDLE;
}

bool
voFrameBuffer::Create( voDeviceContext * device, CreateParms_t & parms )
{
    this->parms = parms;

    std::vector< VkImageView > imageViews { };

    /* ---------------------------------------- Color ------------------------------------------------------------ */
    if ( parms.hasColor )
    {
        voImage::CreateParms_t parmsImage =
        {
            .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .format     = VK_FORMAT_R8G8B8A8_UNORM,
            .width      = parms.width,
            .height     = parms.height,
            .depth      = 1,
        };

        imageColor.Create( device, parmsImage );

        imageColor.TransitionLayout( device );
        imageViews.push_back( imageColor.vkImageView );
    }

    /* ---------------------------------------- Depth ------------------------------------------------------------ */
    if ( parms.hasDepth )
    {
        voImage::CreateParms_t parmsImage =
        {
            .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .format     = DEPTH_FORMAT,
            .width      = parms.width,
            .height     = parms.height,
            .depth      = 1,
        };

        imageDepth.Create( device, parmsImage );

        imageDepth.TransitionLayout( device );
        imageViews.push_back( imageDepth.vkImageView );
    }

    /* ---------------------------------------- Framebuffer -------------------------------------------------------- */
    CreateRenderPass( device );

    {
        VkFramebufferCreateInfo framebufferInfo =
        {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = vkRenderPass,
            .attachmentCount = static_cast< uint32_t >( imageViews.size() ),
            .pAttachments    = imageViews.data(),
            .width           = parms.width,
            .height          = parms.height,
            .layers          = 1,
        };

        VK_CHECK( vkCreateFramebuffer( device->deviceInfo.logical, &framebufferInfo, nullptr, &vkFrameBuffer ),
                  "Failed to create framebuffer" );
    }

    return true;
}

void
voFrameBuffer::CreateRenderPass( voDeviceContext * device )
{
    /* ---------------------------------------- Attachments ------------------------------------------------------------- */
    std::vector< VkAttachmentDescription > attachments { };

    /* ---------------------------------------- Color Attachment -------------------------------------------------------- */
    VkAttachmentReference colorAttachmentRef { 0 };
    if ( parms.hasColor )
    {
        VkAttachmentDescription colorAttachment =
        {
            .format         = imageColor.parms.format,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
        imageColor.vkImageLayout = colorAttachment.finalLayout;

        colorAttachmentRef =
        {
            .attachment = static_cast< uint32_t >( attachments.size() ),
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        attachments.push_back( colorAttachment );
    }

    /* ---------------------------------------- Depth Attachment -------------------------------------------------------- */
    VkAttachmentReference depthAttachmentRef { 0 };
    if ( parms.hasDepth )
    {
        VkAttachmentDescription depthAttachment =
        {
            .format         = imageDepth.parms.format,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        imageDepth.vkImageLayout = depthAttachment.finalLayout;

        depthAttachmentRef =
        {
            .attachment = static_cast< uint32_t >( attachments.size() ),
            .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        attachments.push_back( depthAttachment );
    }

    /* ---------------------------------------- Subpass ------------------------------------------------------------- */
    VkSubpassDescription subpass =
    {
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount    = parms.hasColor ? 1U : 0U,
        .pColorAttachments       = parms.hasColor ? &colorAttachmentRef : nullptr,
        .pDepthStencilAttachment = parms.hasDepth ? &depthAttachmentRef : nullptr,
    };

    /* ---------------------------------------- Dependencies -------------------------------------------------------- */
    std::array< VkSubpassDependency, 2 > dependencies { };
    if (parms.hasColor)
    {
        dependencies =
        {
            {
                {
                    VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_MEMORY_READ_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_DEPENDENCY_BY_REGION_BIT
                },
                {
                    0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
                },
            }
        };
    }
    else
    {
        dependencies =
        {
            {
                {
                    VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
                },
                {
                    0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
                },
            }
        };
    }

    /* ---------------------------------------- Render Pass -------------------------------------------------------- */
    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast< uint32_t >( attachments.size() ),
        .pAttachments    = attachments.data(),
        .subpassCount    = 1,
        .pSubpasses      = &subpass,
        .dependencyCount = static_cast< uint32_t >( dependencies.size() ),
        .pDependencies   = dependencies.data(),
    };

    VK_CHECK( vkCreateRenderPass( device->deviceInfo.logical, &renderPassInfo, nullptr, &vkRenderPass ),
             "Failed to create render pass" );
}

void
voFrameBuffer::BeginRenderPass( voDeviceContext * device, const int cmdBufferIndex )
{
    /* -------------------------------------- Clear Values --------------------------------------------------------- */
    {
        std::vector< VkClearValue > clearValues { };
        if ( parms.hasColor )
        {
            VkClearValue value =
            {
                .color = parms.clearColor,
            };
            clearValues.push_back( value );
        }

        if ( parms.hasDepth )
        {
            VkClearValue value =
            {
                .depthStencil = parms.clearDepthStencil,
            };
            clearValues.push_back( value );
        }

        VkRenderPassBeginInfo renderPassBeginInfo =
        {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass      = vkRenderPass,
            .framebuffer     = vkFrameBuffer,
            .renderArea      =
            {
                .offset      = { 0, 0 },
                .extent      = { parms.width, parms.height }
            },
            .clearValueCount = static_cast< uint32_t >( clearValues.size() ),
            .pClearValues    = clearValues.data(),
        };

        vkCmdBeginRenderPass( device->m_vkCommandBuffers[ cmdBufferIndex ], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    /* -------------------------------------- Viewport & Scissor --------------------------------------------------- */
    {
        VkViewport viewport =
        {
            .x        = 0.0f,
            .y        = 0.0f,
            .width    = static_cast< float >( parms.width ),
            .height   = static_cast< float >( parms.height ),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport( device->m_vkCommandBuffers[ cmdBufferIndex ], 0, 1, &viewport );
    }

    {
        VkRect2D scissor =
        {
            .offset = { 0, 0 },
            .extent = { parms.width, parms.height }
        };
        vkCmdSetScissor( device->m_vkCommandBuffers[ cmdBufferIndex ], 0, 1, &scissor );
    }

    /* -------------------------------------- Shadow --------------------------------------------------------------- */
    if ( parms.hasDepth && !parms.hasColor )
    {
        vkCmdSetDepthBias( device->m_vkCommandBuffers[ cmdBufferIndex ], SHADOW_BIAS, 0.0f, SHADOW_SLOPE );
    }
}

void
voFrameBuffer::EndRenderPass( voDeviceContext * device, const int cmdBufferIndex )
{
    vkCmdEndRenderPass( device->m_vkCommandBuffers[ cmdBufferIndex ] );
}
