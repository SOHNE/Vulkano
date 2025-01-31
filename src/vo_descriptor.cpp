#include "vulkano/vo_descriptor.hpp"
#include "vulkano/vo_pipeline.hpp"
#include "vulkano/vo_buffer.hpp"
#include "vo_utilities.hpp"
#include <vector>
#include <cassert>


// ======================================================================================================================
// ============================================ voDescriptor ============================================================
// ======================================================================================================================


voDescriptor::voDescriptor()
    : m_parent( NULL )
    , m_id( -1 )
    , m_numImages( 0 )
    , m_numBuffers( 0 )
{
    memset( m_bufferInfo, 0, sizeof( VkDescriptorBufferInfo ) * MAX_BUFFERS );
    memset( m_imageInfo, 0, sizeof( VkDescriptorImageInfo ) * MAX_IMAGEINFO );
}

void
voDescriptor::BindImage( VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler, int slot )
{
    assert( slot < MAX_IMAGEINFO );
    assert( m_numImages < MAX_IMAGEINFO );

    m_imageInfo[ slot ] =
    {
        .sampler     = sampler,
        .imageView   = imageView,
        .imageLayout = imageLayout,
    };

    ++m_numImages;
}

void
voDescriptor::BindBuffer( voBuffer * uniformBuffer, VkDeviceSize offset, VkDeviceSize size, int slot )
{
    assert( slot < MAX_BUFFERS );
    assert( m_numBuffers < MAX_BUFFERS );

    m_bufferInfo[ slot ] =
    {
        .buffer = uniformBuffer->vkBuffer,
        .offset = offset,
        .range  = size,
    };

    ++m_numBuffers;
}

void
voDescriptor::BindDescriptor( voDeviceContext * device, VkCommandBuffer vkCommandBuffer, voPipeline * pso )
{
    const uint32_t numDescriptors = m_numImages + m_numBuffers;

    // Describe the connection between a binding and a buffer.
    // How a buffer is going to connect to a descriptor set.

    /* ----------------------------------------- Allocate descriptors ----------------------------------------- */
    const uint32_t allocationSize = sizeof( VkWriteDescriptorSet ) * numDescriptors;
    VkWriteDescriptorSet * descriptorWrites = reinterpret_cast< VkWriteDescriptorSet * >( alloca( allocationSize ) );
    memset( descriptorWrites, 0, allocationSize );

    /* ----------------------------------------- Descriptor Write Information ---------------------------------- */
    {
        uint32_t idx = 0;

        for ( size_t i = 0; i < m_numBuffers; ++i, ++idx )
        {
            descriptorWrites[ idx ] =
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_parent->vkDescriptorSets[ m_id ], // Descriptor set target
                .dstBinding      = idx,                                // Binding to update (matches with binding on layout/shader)
                .dstArrayElement = 0,                                  // Index in array to update
                .descriptorCount = 1,                                  // Amount to update
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // Type of descriptor
                .pBufferInfo     = &m_bufferInfo[ i ],                 // Information about buffer data to bind
            };
        }

        for ( size_t i = 0; i < m_numImages; ++i, ++idx )
        {
            descriptorWrites[ idx ] =
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_parent->vkDescriptorSets[ m_id ],
                .dstBinding      = idx,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo      = &m_imageInfo[ i ],
            };
        }
    }

    /* ----------------------------------------- Update & Bind ------------------------------------------------- */

    vkUpdateDescriptorSets( device->deviceInfo.logical, numDescriptors, descriptorWrites, 0, nullptr );
    vkCmdBindDescriptorSets( vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pso->vkPipelineLayout, 0, 1, &m_parent->vkDescriptorSets[ m_id ], 0, nullptr );
}


// ======================================================================================================================
// ============================================ voDescriptors ===========================================================
// ======================================================================================================================


void voDescriptors::Create(voDeviceContext *device, const voDescriptors::CreateParms_t &parms)
{
    m_parms = parms;

    const uint32_t numUniforms = parms.numUniformsFragment + parms.numUniformsVertex;

    /* ---------------------------------------- Descriptor Pool --------------------------------------------------------- */
    {
        std::vector< VkDescriptorPoolSize > poolSizes { };

        if ( numUniforms > 0 )
        {
            VkDescriptorPoolSize poolSize =
            {
                .type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount  = numUniforms * MAX_DESCRIPTOR_SETS,
            };
            poolSizes.push_back( poolSize );
        }

        if ( parms.numImageSamplers > 0 )
        {
            VkDescriptorPoolSize poolSize =
            {
                .type              = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount   = parms.numImageSamplers * MAX_DESCRIPTOR_SETS
            };
            poolSizes.push_back( poolSize );
        }

        VkDescriptorPoolCreateInfo poolInfo =
        {
            .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags          = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets        = MAX_DESCRIPTOR_SETS + 1,
            .poolSizeCount  = static_cast< uint32_t >( poolSizes.size() ),
            .pPoolSizes     = poolSizes.data(),
        };

        VK_CHECK( vkCreateDescriptorPool( device->deviceInfo.logical, &poolInfo, nullptr, &vkDescriptorPool ),
                  "Failed to create descriptor pool" );
    }

    /* ----------------------------------------- Create Descriptor Set Layout ----------------------------------------- */
    {
        VkDescriptorSetLayoutBinding * uniformBindings = static_cast< VkDescriptorSetLayoutBinding * >( alloca( sizeof( VkDescriptorSetLayoutBinding ) * ( numUniforms ) ) );
        memset( uniformBindings, 0, sizeof( VkDescriptorSetLayoutBinding ) * ( numUniforms ) );

        uint32_t id { 0 };

        for ( uint32_t i = 0; i < parms.numUniformsVertex; ++i, ++id )
        {
            VkDescriptorSetLayoutBinding uniformBinding =
            {
                .binding            = id,  // Binding point in shader
                .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = VK_NULL_HANDLE,
            };
            uniformBindings[ id ] = uniformBinding;
        }

        for ( uint32_t i = 0; i < parms.numUniformsFragment; ++i, ++id )
        {
            VkDescriptorSetLayoutBinding imageSamplerBinding =
            {
                .binding            = id,
                .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = VK_NULL_HANDLE,
            };
            uniformBindings[ id ] = imageSamplerBinding;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo =
        {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount  = numUniforms,
            .pBindings     = uniformBindings,
        };

        VK_CHECK( vkCreateDescriptorSetLayout( device->deviceInfo.logical, &layoutInfo, nullptr, &vkDescriptorSetLayout ),
                  "Failed to create descriptor set layout" );
    }

    /* ----------------------------------------- Create descriptor sets ----------------------------------------- */
    {
        std::vector< VkDescriptorSetLayout > layouts { MAX_DESCRIPTOR_SETS, vkDescriptorSetLayout };

        VkDescriptorSetAllocateInfo allocInfo =
        {
            .sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool      = vkDescriptorPool,
            .descriptorSetCount  = MAX_DESCRIPTOR_SETS,
            .pSetLayouts         = layouts.data(),
        };

        VK_CHECK( vkAllocateDescriptorSets( device->deviceInfo.logical, &allocInfo, vkDescriptorSets ),
                  "Failed to allocate descriptor sets" );
    }
}

void
voDescriptors::Cleanup( voDeviceContext * device )
{
    vkFreeDescriptorSets( device->deviceInfo.logical, vkDescriptorPool, MAX_DESCRIPTOR_SETS, vkDescriptorSets );
    vkDestroyDescriptorSetLayout( device->deviceInfo.logical, vkDescriptorSetLayout, nullptr );
    vkDestroyDescriptorPool( device->deviceInfo.logical, vkDescriptorPool, nullptr );
}
