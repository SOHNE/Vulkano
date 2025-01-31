#ifndef VULKANO_PIPELINE_H
#define VULKANO_PIPELINE_H

#include "vo_api.hpp"
#include "vo_deviceContext.hpp"
#include "vo_descriptor.hpp"
#include <vulkan/vulkan_core.h>
#include <cstring>

class voFrameBuffer;
class voShader;

/**
 * @class voPipeline
 * @brief A class that encapsulates a Vulkan Pipeline.
 *
 * @details The voPipeline class is responsible for managing a Vulkan Pipeline.
 * A pipeline represents the state of all the fixed-function stages and programmable shader stages that will be used during rasterization.
 * The class provides functionalities for creating a pipeline with given parameters, creating a compute pipeline, cleaning up the pipeline,
 * binding the pipeline, dispatching compute commands, and getting a free descriptor.
 *
 * @code
 * voDeviceContext deviceContext;
 * voPipeline pipeline;
 *
 * // Create a pipeline with given parameters
 * voPipeline::CreateParms_t parms = { renderPass, framebuffer, descriptors, shader, width, height, cullMode, depthTest, depthWrite, pushConstantSize, pushConstantShaderStages };
 * pipeline.Create(&deviceContext, parms);
 *
 * // Bind the pipeline
 * pipeline.BindPipeline(cmdBuffer);
 *
 * // Dispatch compute commands
 * pipeline.DispatchCompute(cmdBuffer, groupCountX, groupCountY, groupCountZ);
 *
 * // Cleanup
 * pipeline.Cleanup(&deviceContext);
 * @endcode
 *
 * @see `voFrameBuffer`, `voDescriptors`, `voShader`, `voDescriptor`
 */
class VO_API voPipeline
{
public:
    voPipeline() = default;
    ~voPipeline() = default;

    enum CullMode_t
    {
        CULL_MODE_FRONT,
        CULL_MODE_BACK,
        CULL_MODE_NONE
    };

    /**
    * @struct CreateParms_t
    *
    * @brief Used to configure the `voPipeline` class. Contains parameters that define the behavior and properties of the pipeline.
    */
    struct CreateParms_t
    {
        VkRenderPass    renderPass  { VK_NULL_HANDLE };
        voFrameBuffer * framebuffer { nullptr };
        //*  TODO: Implement multiple descriptors sets */
        voDescriptors * descriptors { nullptr };
        voShader      * shader      { nullptr };

        uint32_t width  { };
        uint32_t height { };

        CullMode_t cullMode { voPipeline::CULL_MODE_NONE };

        uint8_t depthTest  : 1 { false };
        uint8_t depthWrite : 1 { false };

        uint32_t pushConstantSize { 0 };
        VkShaderStageFlagBits pushConstantShaderStages { };

        FORCE_INLINE void Reset() { memset( this, 0, sizeof( CreateParms_t ) ); }
    };

    /* ====================================== Base ==================================================================== */

    /**
     * @brief Creates the pipeline with the given parameters.
     * @param device The Vulkan device context.
     * @param parms The parameters for creating the pipeline.
     * @return True if the pipeline was created successfully, false otherwise.
     */
    VO_API bool Create( voDeviceContext * device, const CreateParms_t & parms );

    /**
     * @brief Creates a compute pipeline with the given parameters.
     * @param device The Vulkan device context.
     * @param parms The parameters for creating the compute pipeline.
     * @return True if the compute pipeline was created successfully, false otherwise.
     */
    VO_API bool CreateCompute( voDeviceContext * device, const CreateParms_t & parms );

    /**
     * @brief Cleans up the pipeline.
     * @param device The Vulkan device context.
     */
    VO_API void Cleanup( voDeviceContext * device );

    /* ====================================== Getters ================================================================== */

    /**
     * @brief Gets a free descriptor from the descriptor pool.
     * @return A voDescriptor object representing a free descriptor in the pool.
     */
    [[nodiscard]]
    VO_API voDescriptor GetFreeDescriptor() const;

    /* ====================================== Bindings ================================================================= */

    /**
     * @brief Binds the pipeline to a command buffer.
     * @param cmdBuffer The command buffer to bind the pipeline to.
     */
     VO_API void BindPipeline( VkCommandBuffer cmdBuffer ) const;

    /**
     * @brief Binds the compute pipeline to a command buffer.
     * @param cmdBuffer The command buffer to bind the compute pipeline to.
     */
    VO_API void BindPipelineCompute( VkCommandBuffer cmdBuffer ) const;

    /**
     * @brief Dispatches compute commands to the command buffer.
     * @param cmdBuffer The command buffer to dispatch the compute commands to.
     * @param groupCountX The number of workgroups to be dispatched in the X dimension.
     * @param groupCountY The number of workgroups to be dispatched in the Y dimension.
     * @param groupCountZ The number of workgroups to be dispatched in the Z dimension.
     */
    VO_API static void DispatchCompute( VkCommandBuffer cmdBuffer, int groupCountX, int groupCountY, int groupCountZ );

    /* ====================================== Pipeline state ============================================================ */

    CreateParms_t m_parms { };

    VkPipelineLayout vkPipelineLayout { VK_NULL_HANDLE };
    VkPipeline       vkPipeline       { VK_NULL_HANDLE };
};


// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================

FORCE_INLINE void
voPipeline::Cleanup( voDeviceContext * device )
{
    vkDestroyPipeline( device->deviceInfo.logical, vkPipeline, nullptr );
    vkDestroyPipelineLayout( device->deviceInfo.logical, vkPipelineLayout, nullptr );

    vkPipeline = VK_NULL_HANDLE; vkPipelineLayout = VK_NULL_HANDLE;
}

FORCE_INLINE voDescriptor
voPipeline::GetFreeDescriptor() const
{
    return m_parms.descriptors->GetFreeDescriptor();
}

FORCE_INLINE void
voPipeline::BindPipeline( VkCommandBuffer cmdBuffer ) const
{
    vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline );
}

FORCE_INLINE void
voPipeline::BindPipelineCompute( VkCommandBuffer cmdBuffer ) const
{
    vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline );
}

FORCE_INLINE void
voPipeline::DispatchCompute( VkCommandBuffer cmdBuffer, int groupCountX, int groupCountY, int groupCountZ )
{
    vkCmdDispatch( cmdBuffer, groupCountX, groupCountY, groupCountZ );
}

#endif //VULKANO_PIPELINE_H
