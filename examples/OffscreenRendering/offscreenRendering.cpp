#include "offscreenRendering.hpp"

#include "vulkano/vo_frameBuffer.hpp"
#include "vulkano/vo_model.hpp"
#include "vulkano/vo_pipeline.hpp"
#include "vulkano/vo_samplers.hpp"
#include "vulkano/vo_shader.hpp"

#include <cassert>
#include <cstdio>
#include <vector>

voFrameBuffer g_offscreenFrameBuffer;

voPipeline g_checkerboardShadowPipeline;
voShader g_checkerboardShadowShader;
voDescriptors g_checkerboardShadowDescriptors;

voFrameBuffer g_shadowFrameBuffer;
voPipeline g_shadowPipeline;
voShader g_shadowShader;
voDescriptors g_shadowDescriptors;

bool
InitOffscreen( voDeviceContext * device, int width, int height )
{
    bool result;

    //
    //	Build the frame buffer to render into
    //
    {
        voFrameBuffer::CreateParms_t frameBufferParms;
        frameBufferParms.width      = width;
        frameBufferParms.height     = height;
        frameBufferParms.hasColor   = true;
        frameBufferParms.hasDepth   = true;
        frameBufferParms.clearColor = { 0.16F, 0.16F, 0.21F, 1.0F };
        result                      = g_offscreenFrameBuffer.Create( device, frameBufferParms );
        if( !result )
            {
                printf( "ERROR: Failed to create off screen buffer\n" );
                assert( 0 );
                return false;
            }
    }

    //
    //	Shadow
    //
    {
        voFrameBuffer::CreateParms_t frameBufferParms;
        frameBufferParms.width    = 4096;
        frameBufferParms.height   = 4096;
        frameBufferParms.hasColor = false;
        frameBufferParms.hasDepth = true;
        result                    = g_shadowFrameBuffer.Create( device, frameBufferParms );
        if( !result )
            {
                printf( "ERROR: Failed to create off screen buffer\n" );
                assert( 0 );
                return false;
            }

        result = g_shadowShader.Load( device, "shadow" );
        if( !result )
            {
                printf( "ERROR: Failed to load shader\n" );
                assert( 0 );
                return false;
            }

        voDescriptors::CreateParms_t descriptorParms {};
        memset( &descriptorParms, 0, sizeof( descriptorParms ) );
        descriptorParms.numUniformsVertex = 2;
        g_shadowDescriptors.Create( device, descriptorParms );

        voPipeline::CreateParms_t pipelineParms =
            {
                .framebuffer = &g_shadowFrameBuffer,
                .descriptors = &g_shadowDescriptors,
                .shader      = &g_shadowShader,
                .width       = frameBufferParms.width,
                .height      = frameBufferParms.height,
                .cullMode    = voPipeline::CULL_MODE_FRONT,
                .depthTest   = true,
                .depthWrite  = true,
            };
        if( !g_shadowPipeline.Create( device, pipelineParms ) )
            {
                printf( "ERROR: Failed to build pipeline\n" );
                assert( 0 );
                return false;
            }
    }

    //
    //	CheckerBoard Shadow
    //
    {
        result = g_checkerboardShadowShader.Load( device, "checkerboardShadowed" );
        if( !result )
            {
                printf( "ERROR: Failed to load shader\n" );
                assert( 0 );
                return false;
            }

        voDescriptors::CreateParms_t descriptorParms {};
        memset( &descriptorParms, 0, sizeof( descriptorParms ) );
        descriptorParms.numUniformsVertex   = 3;
        descriptorParms.numUniformsFragment = 1;
        descriptorParms.numImageSamplers    = 1;
        g_checkerboardShadowDescriptors.Create( device, descriptorParms );

        voPipeline::CreateParms_t pipelineParms;
        pipelineParms.framebuffer = &g_offscreenFrameBuffer;
        pipelineParms.descriptors = &g_checkerboardShadowDescriptors;
        pipelineParms.shader      = &g_checkerboardShadowShader;
        pipelineParms.width       = g_offscreenFrameBuffer.parms.width;
        pipelineParms.height      = g_offscreenFrameBuffer.parms.height;
        pipelineParms.cullMode    = voPipeline::CULL_MODE_FRONT;
        pipelineParms.depthTest   = true;
        pipelineParms.depthWrite  = true;
        result                    = g_checkerboardShadowPipeline.Create( device, pipelineParms );
        if( !result )
            {
                printf( "ERROR: Failed to build pipeline\n" );
                assert( 0 );
                return false;
            }
    }

    return true;
}

bool
CleanupOffscreen( voDeviceContext * device )
{
    g_offscreenFrameBuffer.Cleanup( device );

    g_checkerboardShadowPipeline.Cleanup( device );
    g_checkerboardShadowShader.Cleanup( device );
    g_checkerboardShadowDescriptors.Cleanup( device );

    g_shadowPipeline.Cleanup( device );
    g_shadowShader.Cleanup( device );
    g_shadowDescriptors.Cleanup( device );
    g_shadowFrameBuffer.Cleanup( device );
    return true;
}

void
DrawOffscreen( voDeviceContext * device, int cmdBufferIndex, voBuffer * uniforms, const voRenderModel * renderModels, const int numModels )
{
    VkCommandBuffer cmdBuffer = device->m_vkCommandBuffers[cmdBufferIndex];

    const int camOffset = 0;
    const int camSize   = sizeof( float ) * 16 * 4;

    const int shadowCamOffset = device->GetAligendUniformByteOffset( camOffset + camSize );
    const int shadowCamSize   = camSize;

    //
    //	Update the Shadows
    //
    {
        g_shadowFrameBuffer.imageDepth.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );

        g_shadowFrameBuffer.BeginRenderPass( device, cmdBufferIndex );

        // Binding the pipeline is effectively the "use shader" we had back in our opengl apps
        g_shadowPipeline.BindPipeline( cmdBuffer );
        for( int i = 0; i < numModels; i++ )
            {
                const voRenderModel & renderModel = renderModels[i];

                // Descriptor is how we bind our buffers and images
                voDescriptor descriptor = g_shadowPipeline.GetFreeDescriptor();
                descriptor.BindBuffer( uniforms, shadowCamOffset, shadowCamSize, 0 );                     // bind the camera matrices
                descriptor.BindBuffer( uniforms, renderModel.uboByteOffset, renderModel.uboByteSize, 1 ); // bind the model matrices
                descriptor.BindDescriptor( device, cmdBuffer, &g_shadowPipeline );
                renderModel.model->DrawIndexed( cmdBuffer );
            }

        g_shadowFrameBuffer.EndRenderPass( device, cmdBufferIndex );

        g_shadowFrameBuffer.imageDepth.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL );
    }

    //
    //	Draw the World
    //
    {
        g_offscreenFrameBuffer.imageColor.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );
        g_offscreenFrameBuffer.BeginRenderPass( device, cmdBufferIndex );

        //
        //	Draw the models
        //
        {
            // Binding the pipeline is effectively the "use shader" we had back in our opengl apps
            g_checkerboardShadowPipeline.BindPipeline( cmdBuffer );
            for( int i = 0; i < numModels; i++ )
                {
                    const voRenderModel & renderModel = renderModels[i];

                    // Descriptor is how we bind our buffers and images
                    voDescriptor descriptor = g_checkerboardShadowPipeline.GetFreeDescriptor();
                    descriptor.BindBuffer( uniforms, camOffset, camSize, 0 );                                 // bind the camera matrices
                    descriptor.BindBuffer( uniforms, renderModel.uboByteOffset, renderModel.uboByteSize, 1 ); // bind the model matrices
                    descriptor.BindBuffer( uniforms, shadowCamOffset, shadowCamSize, 2 );                     // bind the shadow camera matrices
                    descriptor.BindImage( VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, g_shadowFrameBuffer.imageDepth.vkImageView, voSamplers::m_samplerStandard, 0 );
                    descriptor.BindDescriptor( device, cmdBuffer, &g_checkerboardShadowPipeline );
                    renderModel.model->DrawIndexed( cmdBuffer );
                }
        }

        g_offscreenFrameBuffer.EndRenderPass( device, cmdBufferIndex );
        g_offscreenFrameBuffer.imageColor.TransitionLayout( cmdBuffer, VK_IMAGE_LAYOUT_GENERAL );
    }
}

void
Resize( voDeviceContext * device, int width, int height )
{
    g_offscreenFrameBuffer.Resize( device, width, height );
}
