#ifndef VULKANO_IMAGE_H
#define VULKANO_IMAGE_H

#include <vulkan/vulkan_core.h>
#include "vo_api.hpp"

class voDeviceContext;

/**
 * @class voImage
 * @brief A class that encapsulates a Vulkan Image.
 *
 * @details The voImage class is responsible for managing a Vulkan Image.
 * An image represents a multidimensional array of data that can be used in various ways by the shader stages.
 * The class provides functionalities for creating an image with given parameters, cleaning up the image, 
 * transitioning the image layout, and getting the Vulkan image, image view, and device memory objects.
 *
 * @code
 * voDeviceContext deviceContext;
 * voImage image;
 *
 * // Create an image with given parameters
 * voImage::CreateParms_t parms = { usageFlags, format, width, height, depth };
 * image.Create(&deviceContext, parms);
 *
 * // Transition the image layout
 * image.TransitionLayout(&deviceContext);
 * image.TransitionLayout(cmdBuffer, newLayout);
 *
 * // Cleanup
 * image.Cleanup(&deviceContext);
 * @endcode
 *
 * @see `voDeviceContext`
 */
class VO_API voImage
{
  public:
    voImage()  = default;
    ~voImage() = default;

    /**
     * @struct CreateParms_t
     * @brief Parameters for creating a Vulkan image.
     */
    struct CreateParms_t
    {
        VkImageUsageFlags usageFlags; ///< The usage flags for the image
        VkFormat          format;     ///< The format of the image
        uint32_t          width;      ///< The width of the image
        uint32_t          height;     ///< The height of the image
        uint32_t          depth;      ///< The depth of the image
    };

    /**
     * @brief Creates the image with the given parameters.
     * @param device The Vulkan device context.
     * @param parms The parameters for creating the image.
     * @return True if the image was created successfully, false otherwise.
     */
    bool Create( voDeviceContext * device, const CreateParms_t & parms );

    /**
     * @brief Cleans up the image.
     * @param device The Vulkan device context.
     */
    void Cleanup( voDeviceContext * device ) const;

    /**
     * @brief Transitions the image layout.
     * @param device The Vulkan device context.
     */
    void TransitionLayout( voDeviceContext * device );

    /**
     * @brief Transitions the image layout.
     * @param cmdBuffer The command buffer to use for the transition.
     * @param newLayout The new layout for the image.
     */
    void TransitionLayout( VkCommandBuffer cmdBuffer, VkImageLayout newLayout );

    CreateParms_t parms {};

    VkImage        vkImage { VK_NULL_HANDLE };        ///< The Vulkan image object
    VkImageView    vkImageView { VK_NULL_HANDLE };    ///< The Vulkan image view object
    VkDeviceMemory vkDeviceMemory { VK_NULL_HANDLE }; ///< The Vulkan device memory object associated with the image

    VkImageLayout vkImageLayout {}; ///< The current layout of the image
};

#endif //VULKANO_IMAGE_H
