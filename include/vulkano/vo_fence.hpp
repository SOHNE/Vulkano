#ifndef VULKANO_FENCE_H
#define VULKANO_FENCE_H

#include "vo_api.hpp"
#include "vulkano/vo_common.hpp"


class voDeviceContext;

/**
 * @class voFence
 * @brief A class that encapsulates a Vulkan Fence.
 *
 * @details The voFence class is responsible for managing a Vulkan Fence.
 * A fence is a synchronization primitive that can be used to insert a dependency from a queue to the host.
 * The class provides functionalities for creating a Vulkan fence object, waiting for the fence to become signaled, and getting the Vulkan fence object.
 *
 * @code
 * voDeviceContext deviceContext;
 * voFence fence(&deviceContext);
 *
 * // Create the Vulkan fence object
 * fence.Create(&deviceContext);
 *
 * // Wait for the fence to become signaled
 * fence.Wait(&deviceContext);
 *
 * // Get the Vulkan fence object
 * VkFence vkFence = fence.GetFence();
 * @endcode
 *
 * @see `voDeviceContext`
 */
class VO_API voFence
{
public:
    explicit voFence( voDeviceContext * device );
    ~voFence();

    /** @brief Get the Vulkan fence object */
   FORCE_INLINE VkFence GetFence() const;

private:
    /**
     * @brief Create the Vulkan fence object
     *
     * @param device A pointer to the voDeviceContext object that the fence will be associated with.
     */
    void Create( voDeviceContext * device );

    /**
     * @brief Wait for the fence to become signaled
     *
     * @param device A pointer to the voDeviceContext object that the fence is associated with
     * @return True if the fence was signaled, false otherwise
     */
    void Wait( voDeviceContext * device );

    voDeviceContext * m_device { nullptr }; ///< A pointer to the voDeviceContext object that the fence is associated with

    VkFence m_vkFence { VK_NULL_HANDLE };   ///< The Vulkan fence object
};


// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================


FORCE_INLINE VkFence
voFence::GetFence() const
{
  return m_vkFence;
}

#endif //VULKANO_FENCE_H
