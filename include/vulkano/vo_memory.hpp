#ifndef VULKANO_MEMORY_HPP
#define VULKANO_MEMORY_HPP

#include <vulkan/vulkan.h>
#include <mutex>
#include <unordered_map>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

struct VoMemoryCreateInfo
{
    VkInstance instance {};
    VkDevice device {};
    VkPhysicalDevice physicalDevice {};
    uint32_t vulkanApiVersion {};
    bool enableStatsString = false;
    bool enableHeapBudget  = false;
};

class voMemory
{
  public:
    // Main interface
    static voMemory * Create( const VoMemoryCreateInfo & createInfo );
    void Destroy();

    // Buffer allocation
    VkResult CreateBuffer(
        const VkBufferCreateInfo & bufferInfo,
        const VmaAllocationCreateInfo & allocInfo,
        VkBuffer & buffer,
        VmaAllocation & allocation,
        VmaAllocationInfo * allocationInfo = nullptr );

    // Image allocation
    VkResult CreateImage(
        const VkImageCreateInfo & imageInfo,
        const VmaAllocationCreateInfo & allocInfo,
        VkImage & image,
        VmaAllocation & allocation,
        VmaAllocationInfo * allocationInfo = nullptr );

    // Memory management
    void DestroyBuffer( VkBuffer buffer, VmaAllocation allocation );
    void DestroyImage( VkImage image, VmaAllocation allocation );
    VkResult MapMemory( VmaAllocation allocation, void ** data );
    void UnmapMemory( VmaAllocation allocation );

    // Memory pools
    VkResult CreatePool( const VmaPoolCreateInfo & createInfo, VmaPool & pool );
    void DestroyPool( VmaPool pool );

    // Statistics and debugging
    void GetHeapBudgets( VmaBudget * budgets );
    std::string GetStatsString() const;

    // Defragmentation
    VkResult DefragmentationBegin(
        const VmaDefragmentationInfo * info,
        VmaDefragmentationStats * stats,
        VmaDefragmentationContext * context );

    void DefragmentationEnd( VmaDefragmentationContext context );

    // Direct allocator access (use carefully)
    VmaAllocator
    GetAllocator() const
    {
        return m_allocator;
    }

  private:
    voMemory() = default;

    VmaAllocator m_allocator = nullptr;
    bool m_enableStatsString = false;
    bool m_enableHeapBudget  = false;

    // Track allocations for debugging
    struct AllocationInfo
    {
        size_t size;
        std::string debugName;
        VmaMemoryUsage usage;
    };

    std::mutex m_allocationMutex;
    std::unordered_map< VmaAllocation, AllocationInfo > m_allocations;
};

#endif /** VULKANO_MEMORY_HPP */
