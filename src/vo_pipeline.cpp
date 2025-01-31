#include "vulkano/vo_pipeline.hpp"
#include "vulkano/vo_descriptor.hpp"
#include "vulkano/vo_deviceContext.hpp"
#include "vulkano/vo_frameBuffer.hpp"
#include "vulkano/vo_model.hpp"
#include "vulkano/vo_shader.hpp"

#ifndef SHADER_ENTRY_POINT
#    define SHADER_ENTRY_POINT "main"
#endif /** SHADER_ENTRY_POINT */

bool
voPipeline::Create( voDeviceContext * device, const CreateParms_t & parms )
{
    if( VK_NULL_HANDLE != vkPipeline )
        {
            Cleanup( device );
        }

    m_parms = parms;

    const int width  = static_cast< int >( parms.width );
    const int height = static_cast< int >( parms.height );

    /* ----------------------------------------- Shader Stages Creation ----------------------------------------- */

    std::vector< VkPipelineShaderStageCreateInfo > shaderStages {};
    for( const auto & module : parms.shader->modules )
        {
            VkPipelineShaderStageCreateInfo shaderStageInfo =
                {
                    .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage  = module.second.stage,
                    .module = module.second.module,
                    .pName  = SHADER_ENTRY_POINT,
                };

            shaderStages.push_back( shaderStageInfo );
        }

    /* ----------------------------------------- Vertex Input ----------------------------------------- */

    VkVertexInputBindingDescription bindingDescription = vert_t::GetBindingDescription();
    vert_t::AttrDesc attributeDescriptions             = vert_t::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo =
        {
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = 1,                                                       // Number of vertex binding descriptions
            .pVertexBindingDescriptions      = &bindingDescription,                                     // List of vertex binding descriptions (data spacing/stride information)
            .vertexAttributeDescriptionCount = static_cast< uint32_t >( attributeDescriptions.size() ), // Number of vertex attribute descriptions
            .pVertexAttributeDescriptions    = attributeDescriptions.data()                             // List of vertex attribute descriptions (data format and where to bind to from)
        };

    /* ----------------------------------------- Input Assembly ----------------------------------------- */

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo =
        {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // Primitive type to assemble vertices as
            .primitiveRestartEnable = VK_FALSE                             // Allow overriding of "strip" topology to start new primitives
        };

    /**
     * VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST treats each set of three vertices as an independent triangle.
     * For example, if we have six vertices (v0, v1, v2, v3, v4, v5), it will form two triangles: (v0, v1, v2) and (v3, v4, v5).
     *
     * v0     v3
     *  *-----*
     *  |   / |
     *  |  /  |
     *  | /   |
     *  |/    |
     *  *-----*
     * v2     v5
     */

    /* ----------------------------------------- Viewport & Scissor ----------------------------------------- */

    VkViewport viewport =
        {
            .x = 0.0F, // x start coordinate
            .y = 0.0F, // y start coordinate

            .width  = (float)width,  // width of the framebuffer
            .height = (float)height, // height of the framebuffer

            .minDepth = 0.0F, // Min depth of the framebuffer
            .maxDepth = 1.0F  // Max depth of the framebuffer
        };

    VkRect2D scissor =
        {
            .offset = {              0,                0}, // Offset to use region from
            .extent = {(uint32_t)width, (uint32_t)height}, // Extent to describre region to use, starting at offset
    };

    VkPipelineViewportStateCreateInfo viewportStateInfo =
        {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,         // Number of viewports to use
            .pViewports    = &viewport, // List of viewports to use
            .scissorCount  = 1,         // Number of scissor rectangles to use
            .pScissors     = &scissor   // List of scissor rectangles to use
        };

    /* ----------------------------------------- Dynamic States ----------------------------------------- */

    // Dynamic states to enable
    // WARNING: If you are resizing the window, you need to recreate the swap chain,
    // 			 swap chain images, and any image views associated with output attachments to the swap chain
    std::array< VkDynamicState, 3 > dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_DEPTH_BIAS };

    // Dynamic state creation information
    VkPipelineDynamicStateCreateInfo dynamicStateInfo =
        {
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast< uint32_t >( dynamicStates.size() ), // Number of dynamic states to enable
            .pDynamicStates    = dynamicStates.data()                             // List of dynamic states to enable
        };

    /* ----------------------------------------- Rasterizer ----------------------------------------- */

    // How to draw the polygons
    VkPipelineRasterizationStateCreateInfo rasterizerInfo =
        {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,                        // Change if fragments beyond near/far planes are clamped (default) or discarded
            .rasterizerDiscardEnable = VK_FALSE,                        // Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
            .polygonMode             = VK_POLYGON_MODE_FILL,            // How to handle filling points between vertices
            .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE, // Winding to determine which side is front
            .depthBiasEnable         = VK_FALSE,                        // Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)
            .lineWidth               = 1.0F,                            // How thick lines should be when drawn
        };

    // Determine the culling face mode
    switch( parms.cullMode )
        {
            case CULL_MODE_FRONT:
                rasterizerInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
                break;

            case CULL_MODE_BACK:
                rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
                break;

            default:
                rasterizerInfo.cullMode = VK_CULL_MODE_NONE;
                break;
        }

    /* ----------------------------------------- Multisampling ----------------------------------------- */

    // How to handle multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingInfo =
        {
            .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT, // Number of samples to use per fragment
            .sampleShadingEnable  = VK_FALSE,              // Enable multisample shading or not
        };

    /* ----------------------------------------- Depth Stencil ----------------------------------------- */

    // Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,

            .depthTestEnable  = parms.depthTest ? VK_TRUE : VK_FALSE,  // Enable checking depth to determine fragment write
            .depthWriteEnable = parms.depthWrite ? VK_TRUE : VK_FALSE, // Enable writing to the depth buffer (to replace old values)
            .depthCompareOp   = VK_COMPARE_OP_LESS,                    // Comparison operation that allows an overwriting (is in front)

            .depthBoundsTestEnable = VK_FALSE, // Depth bounds test: Does the depth value exist between two bounds
            .stencilTestEnable     = VK_FALSE, // Enable checking stencil value

            .front = {}, // Stencil operations for front-facing triangles
            .back  = {}, // Stencil operations for back-facing triangles

            .minDepthBounds = 0.0F, // Min depth bounds
            .maxDepthBounds = 1.0F, // Max depth bounds
        };

    /**
     * Depth Testing (depthCompareOp):
     *
     * - VK_COMPARE_OP_NEVER            : Always pass the depth test
     * - VK_COMPARE_OP_LESS             : Pass if the new depth is less than the old depth
     * - VK_COMPARE_OP_EQUAL            :                          equal to the old depth
     * - VK_COMPARE_OP_LESS_OR_EQUAL    :                          less than or equal to the old depth
     * - VK_COMPARE_OP_GREATER          :                          greater than the old depth
     * - VK_COMPARE_OP_NOT_EQUAL        :                          not equal to the old depth
     * - VK_COMPARE_OP_GREATER_OR_EQUAL :                          greater than or equal to the old depth
     * - VK_COMPARE_OP_ALWAYS           : Always pass the depth test
     */

    /* ----------------------------------------- Color Blending ----------------------------------------- */

    // How to handle the colours
    VkPipelineColorBlendAttachmentState colorBlendAttachment =
        {
            .blendEnable = VK_TRUE, // Enable blending

            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,           // How to handle blending of new colour
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // How to handle blending of old colour
            .colorBlendOp        = VK_BLEND_OP_ADD,                     // Type of blend operation to use

            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // How to handle blending of new alpha
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // How to handle blending of old alpha
            .alphaBlendOp        = VK_BLEND_OP_ADD,      // Type of blend operation to use for alpha

            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | // Which colours to apply the blend to
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };

    /**
     * Blend Equation:
     *
     * newColor.rgb = ( srcColourBlendFactor * newColor )    colourBlendOp    ( dstColourBlendFactor * oldColor )
     * newColor.a   = ( srcAlphaBlendFactor * newAlpha  )    alphaBlendOp     ( dstAlphaBlendFactor * oldAlpha  )
     *
     * Summarised:
     *
     * ( VK_BLEND_FACTOR_SRC_ALPHA * newColor ) + ( VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * oldColor )
     * ( newAlpha + newColor ) + ( ( 1 - newAlpha ) * oldColor )
     */

    // How to handle all the colours and alpha
    VkPipelineColorBlendStateCreateInfo colorBlendingInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,

            .logicOpEnable = VK_FALSE, // Alternative to calculations is to use logical operations
            .logicOp       = VK_LOGIC_OP_COPY, // What logical operation to use

            .attachmentCount = 1, // Number of colour blend attachments
            .pAttachments    = &colorBlendAttachment, // Information about how to handle blending

            .blendConstants = { 0.0F, 0.0F, 0.0F, 0.0F }  // (Optional) Constants to use for blending [VK_BLEND_FACTOR_CONSTANT_COLOR]
    };

    /* ----------------------------------------- Pipeline Layout ----------------------------------------- */

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts    = VK_NULL_HANDLE,
    };

    // Check if descriptors are present and have bindings
    if( parms.descriptors && parms.descriptors->vkDescriptorSetLayout != VK_NULL_HANDLE )
        {
            // Update pipeline layout info
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts    = &parms.descriptors->vkDescriptorSetLayout;
        }

    /* ----------------------------------------- Push constants ----------------------------------------- */

    // Check for push constants
    if( parms.pushConstantSize > 0 )
        {
            // Define push constant values (no 'create' needed)
            VkPushConstantRange pushConstantRange =
                {
                    .stageFlags = parms.pushConstantShaderStages, // Shader stage push constant will go to
                    .offset     = 0,                              // Offset into given data to pass to push constant
                    .size       = parms.pushConstantSize,         // Size of data being passed
                };

            // Set push constant to layout
            pipelineLayoutInfo.pushConstantRangeCount = 1;                  // Number of push constant ranges
            pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange; // Pointer to array of push constant ranges
        }

    VK_CHECK( vkCreatePipelineLayout( device->deviceInfo.logical, &pipelineLayoutInfo, VK_NULL_HANDLE, &vkPipelineLayout ),
              "Failed to create pipeline layout" );

    /* ----------------------------------------- Create Pipeline ----------------------------------------- */

    VkGraphicsPipelineCreateInfo pipelineInfo =
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

            // Shader stages
            .stageCount = static_cast< uint32_t >( shaderStages.size() ),
            .pStages    = shaderStages.data(),

            // States creation
            .pVertexInputState   = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pViewportState      = &viewportStateInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState   = &multisamplingInfo,
            .pDepthStencilState  = &depthStencilInfo,
            .pColorBlendState    = &colorBlendingInfo,
            .pDynamicState       = &dynamicStateInfo,

            // Layout setup
            .layout     = vkPipelineLayout,
            .renderPass = parms.renderPass,
            .subpass    = 0,

            // Pipeline derivatives : Can create multiple pipelines that derive from one another for optimisation
            .basePipelineHandle = VK_NULL_HANDLE, // Pipeline to derive from
            .basePipelineIndex  = -1              // Index of the base pipeline to derive from
        };

    // Attach a valid render pass
    if( parms.framebuffer != nullptr )
        {
            pipelineInfo.renderPass = parms.framebuffer->vkRenderPass;
        }

    // used to store and reuse previously created pipelines, reducing the cost of pipeline creation
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    VK_CHECK( vkCreateGraphicsPipelines( device->deviceInfo.logical, pipelineCache, 1, &pipelineInfo, VK_NULL_HANDLE, &vkPipeline ),
              "Failed to create pipeline" );

    return true;
}

bool
voPipeline::CreateCompute( voDeviceContext * device, const CreateParms_t & parms )
{
    if( vkPipeline != VK_NULL_HANDLE )
        {
            Cleanup( device );
        }

    m_parms = parms;

    /* ----------------------------------------- Shader Stages Creation ----------------------------------------- */

    VkPipelineShaderStageCreateInfo shaderStageInfo =
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT,
            .module = parms.shader->modules[voShader::SHADER_STAGE_COMPUTE].module,
            .pName  = SHADER_ENTRY_POINT,
        };

    /* ----------------------------------------- Pipeline Layout ----------------------------------------- */

    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
        {
            .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts    = &parms.descriptors->vkDescriptorSetLayout,
        };

    /* ----------------------------------------- Push constants ----------------------------------------- */

    if( parms.pushConstantSize > 0 )
        {
            VkPushConstantRange pushConstantRange =
                {
                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                    .offset     = 0,
                    .size       = parms.pushConstantSize,
                };

            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
        }

    VK_CHECK( vkCreatePipelineLayout( device->deviceInfo.logical, &pipelineLayoutInfo, VK_NULL_HANDLE, &vkPipelineLayout ),
              "Failed to create pipeline layout" );

    /* ----------------------------------------- Create Compute Pipeline ----------------------------------------- */

    VkComputePipelineCreateInfo pipelineInfo =
        {
            .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage              = shaderStageInfo,
            .layout             = vkPipelineLayout,
            .basePipelineHandle = VK_NULL_HANDLE,
        };

    VK_CHECK( vkCreateComputePipelines( device->deviceInfo.logical, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &vkPipeline ),
              "Failed to create pipeline" );

    return true;
}
