#include "vulkano/vo_memory.hpp"
#include <sstream>

#define VMA_IMPLEMENTATION
//#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) org_lwjgl_aligned_alloc((alignment), (size))
//#define VMA_SYSTEM_ALIGNED_FREE(ptr) org_lwjgl_aligned_free(ptr)
#define VMA_VULKAN_VERSION 1003000
#define VMA_DEDICATED_ALLOCATION 1
#define VMA_BIND_MEMORY2 1
#define VMA_MEMORY_BUDGET 1
#define VMA_BUFFER_DEVICE_ADDRESS 1
#define VMA_MEMORY_PRIORITY 1
#define VMA_EXTERNAL_MEMORY 1
#include "vk_mem_alloc.h"

voMemory *
voMemory::Create( const VoMemoryCreateInfo & createInfo )
{
    auto * memory               = new voMemory();
    memory->m_enableStatsString = createInfo.enableStatsString;
    memory->m_enableHeapBudget  = createInfo.enableHeapBudget;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion       = createInfo.vulkanApiVersion;
    allocatorInfo.physicalDevice         = createInfo.physicalDevice;
    allocatorInfo.device                 = createInfo.device;
    allocatorInfo.instance               = createInfo.instance;

    if( createInfo.enableHeapBudget )
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        }

    if( vmaCreateAllocator( &allocatorInfo, &memory->m_allocator ) != VK_SUCCESS )
        {
            delete memory;
            return nullptr;
        }

    return memory;
}

void
voMemory::Destroy()
{
    if( m_allocator )
        {
            if( !m_allocations.empty() )
                {
                    std::stringstream ss;
                    ss << "Warning: " << m_allocations.size() << " VMA allocations not freed:\n";
                    for( const auto & [allocation, info] : m_allocations )
                        {
                            ss << "- Size: " << info.size << " bytes";
                            if( !info.debugName.empty() )
                                {
                                    ss << ", Name: " << info.debugName;
                                }
                            ss << "\n";
                        }
                    // Log or print ss.str()
                }

            vmaDestroyAllocator( m_allocator );
            m_allocator = nullptr;
        }
}

VkResult
voMemory::CreateBuffer(
    const VkBufferCreateInfo & bufferInfo,
    const VmaAllocationCreateInfo & allocInfo,
    VkBuffer & buffer,
    VmaAllocation & allocation,
    VmaAllocationInfo * allocationInfo )
{
    VkResult result = vmaCreateBuffer(
        m_allocator,
        &bufferInfo,
        &allocInfo,
        &buffer,
        &allocation,
        allocationInfo );

    if( result == VK_SUCCESS )
        {
            std::lock_guard< std::mutex > lock( m_allocationMutex );
            AllocationInfo info = {
                bufferInfo.size,
                allocInfo.pUserData ? std::string( static_cast< const char * >( allocInfo.pUserData ) ) : "",
                allocInfo.usage };
            m_allocations[allocation] = info;
        }

    return result;
}

VkResult
voMemory::CreateImage(
    const VkImageCreateInfo & imageInfo,
    const VmaAllocationCreateInfo & allocInfo,
    VkImage & image,
    VmaAllocation & allocation,
    VmaAllocationInfo * allocationInfo )
{
    VkResult result = vmaCreateImage(
        m_allocator,
        &imageInfo,
        &allocInfo,
        &image,
        &allocation,
        allocationInfo );

    if( result == VK_SUCCESS )
        {
            std::lock_guard< std::mutex > lock( m_allocationMutex );
            VkDeviceSize size = imageInfo.extent.width *
                                imageInfo.extent.height *
                                imageInfo.extent.depth; // Approximate size
            AllocationInfo info = {
                size,
                allocInfo.pUserData ? std::string( static_cast< const char * >( allocInfo.pUserData ) ) : "",
                allocInfo.usage };
            m_allocations[allocation] = info;
        }

    return result;
}

void
voMemory::DestroyBuffer( VkBuffer buffer, VmaAllocation allocation )
{
    if( buffer && allocation )
        {
            vmaDestroyBuffer( m_allocator, buffer, allocation );
            std::lock_guard< std::mutex > lock( m_allocationMutex );
            m_allocations.erase( allocation );
        }
}

void
voMemory::DestroyImage( VkImage image, VmaAllocation allocation )
{
    if( image && allocation )
        {
            vmaDestroyImage( m_allocator, image, allocation );
            std::lock_guard< std::mutex > lock( m_allocationMutex );
            m_allocations.erase( allocation );
        }
}

VkResult
voMemory::MapMemory( VmaAllocation allocation, void ** data )
{
    return vmaMapMemory( m_allocator, allocation, data );
}

void
voMemory::UnmapMemory( VmaAllocation allocation )
{
    vmaUnmapMemory( m_allocator, allocation );
}

VkResult
voMemory::CreatePool( const VmaPoolCreateInfo & createInfo, VmaPool & pool )
{
    return vmaCreatePool( m_allocator, &createInfo, &pool );
}

void
voMemory::DestroyPool( VmaPool pool )
{
    if( pool )
        {
            vmaDestroyPool( m_allocator, pool );
        }
}

void
voMemory::GetHeapBudgets( VmaBudget * budgets )
{
    if( m_enableHeapBudget )
        {
            vmaGetHeapBudgets( m_allocator, budgets );
        }
}

std::string
voMemory::GetStatsString() const
{
    if( !m_enableStatsString )
        {
            return "Stats string disabled. Enable with enableStatsString in creation.";
        }

    char * stats;
    vmaBuildStatsString( m_allocator, &stats, true );
    std::string result( stats );
    vmaFreeStatsString( m_allocator, stats );
    return result;
}

VkResult
voMemory::DefragmentationBegin(
    const VmaDefragmentationInfo * info,
    VmaDefragmentationStats * stats,
    VmaDefragmentationContext * context )
{
    return vmaBeginDefragmentation( m_allocator, info, context );
}

void
voMemory::DefragmentationEnd(
    VmaDefragmentationContext context )
{
    vmaEndDefragmentation( m_allocator, context, nullptr );
}
