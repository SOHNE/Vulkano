#include "vulkano/vo_deviceContext.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "vo_utilities.hpp"
#include "vulkano/vo_common.hpp"
#include "vulkano/vo_fence.hpp"

// ======================================================================================================================
// ============================================ Function Set ============================================================
// ======================================================================================================================

PFN_vkCreateDebugReportCallbackEXT function_set_t::vkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT function_set_t::vkDestroyDebugReportCallbackEXT;

void
function_set_t::Link( VkInstance instance )
{
    function_set_t::vkCreateDebugReportCallbackEXT =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
    function_set_t::vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
}

// ======================================================================================================================
// ============================================ Physical Device Properties ==============================================
// ======================================================================================================================

bool
physical_device_properties_t::SetProperties( VkPhysicalDevice device, VkSurfaceKHR vkSurface )
{
    physicalDevice = device;

    vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );
    vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memoryProperties );
    vkGetPhysicalDeviceFeatures( physicalDevice, &features );

    /* ---------------------------------------- VkSurfaceCapabilitiesKHR ------------------------------------------------ */
    {
        VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, vkSurface, &surfaceCapabilities ),
                  "Failed to vkGetPhysicalDeviceSurfaceCapabilitiesKHR!" );
    }

    /* ---------------------------------------- VkSurfaceFormatKHR ------------------------------------------------------ */
    {
        uint32_t numFormats;
        VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, vkSurface, &numFormats, NULL ),
                  "Failed to vkGetPhysicalDeviceSurfaceFormatsKHR" );

        surfaceFormats.resize( numFormats );
        VK_CHECK(
            vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, vkSurface, &numFormats, surfaceFormats.data() ),
            "Failed to vkGetPhysicalDeviceSurfaceFormatsKHR" );
    }

    /* ---------------------------------------- VkPresentModeKHR -------------------------------------------------------- */
    {
        uint32_t numPresentModes;
        VK_CHECK( vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, vkSurface, &numPresentModes, NULL ),
                  "Failed to vkGetPhysicalDeviceSurfacePresentModesKHR" );

        presentModes.resize( numPresentModes );
        VK_CHECK( vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, vkSurface, &numPresentModes,
                                                             presentModes.data() ),
                  "Failed to vkGetPhysicalDeviceSurfacePresentModesKHR" );
    }

    /* ---------------------------------------- VkQueueFamilyProperties -------------------------------------------------------- */
    {
        uint32_t numQueues = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &numQueues, NULL );
        if( numQueues == 0 )
            {
                throw std::runtime_error( "Failed to vkGetPhysicalDeviceQueueFamilyProperties\n" );
            }

        queueFamilyProperties.resize( numQueues );
        vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &numQueues, queueFamilyProperties.data() );
        if( numQueues == 0 )
            {
                throw std::runtime_error( "ERROR: Failed to vkGetPhysicalDeviceQueueFamilyProperties\n" );
            }
    }

    /* ---------------------------------------- VkExtensionProperties -------------------------------------------------------- */
    {
        uint32_t numExtensions;
        VK_CHECK( vkEnumerateDeviceExtensionProperties( physicalDevice, NULL, &numExtensions, NULL ),
                  "Failed to vkEnumerateDeviceExtensionProperties" );

        extensionProperties.resize( numExtensions );
        VK_CHECK( vkEnumerateDeviceExtensionProperties( physicalDevice, NULL, &numExtensions,
                                                        extensionProperties.data() ),
                  "Failed to vkEnumerateDeviceExtensionProperties" );

        if( numExtensions == 0 )
            {
                throw std::runtime_error( "Failed to vkEnumerateDeviceExtensionProperties" );
            }
    }

    return true;
}

bool
physical_device_properties_t::HasExtensionsSupport( const char ** extensions, const int num ) const
{
    for( int i = 0; i < num; i++ )
        {
            const char * extension = extensions[i];

            bool bHasExtension = false;
            for( const auto & property : extensionProperties )
                {
                    if( strcmp( extension, property.extensionName ) == 0 )
                        {
                            bHasExtension = true;
                            break;
                        }
                }

            if( !bHasExtension )
                {
                    return false;
                }
        }

    return true;
}

// ======================================================================================================================
// ============================================ Device Context ==========================================================
// ======================================================================================================================

static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanErrorMessage(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
    size_t location, int32_t code, const char * layerPrefix,
    const char * msg, void * userData )
{
    // Log Vulkan error message with red text (using spdlog for consistent logging)
    spdlog::error( msg );

    // Trigger assertion failure
    voAssert( 0 );

    return VK_FALSE;
}

bool
voDeviceContext::CreateInstance( bool enableLayers, const std::vector< const char * > & extensions_required )
{
    this->enableLayers = enableLayers;

    std::vector< const char * > extensions = extensions_required;
    extensions.push_back( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );

    std::vector< const char * > validationLayers;
    if( enableLayers )
        {
            validationLayers = m_validationLayers;

            uint32_t numLayers = 0u;
            vkEnumerateInstanceLayerProperties( &numLayers, nullptr );

            std::vector< VkLayerProperties > layerProperties( numLayers );
            vkEnumerateInstanceLayerProperties( &numLayers, layerProperties.data() );

            spdlog::info( "-------------------------------------------------" );
            spdlog::info( "Available Vulkan Instance Layers:" );
            for( int i = 0; i < numLayers; i++ )
                {
                    spdlog::info( "\t{}", layerProperties[i].layerName );

                    // Prioritize "VK_LAYER_KHRONOS_validation" by clearing any other layers and adding it.
                    if( strcmp( "VK_LAYER_KHRONOS_validation", layerProperties[i].layerName ) == 0 )
                        {
                            validationLayers.clear();
                            validationLayers.push_back( "VK_LAYER_KHRONOS_validation" );
                            spdlog::info( "" );
                            spdlog::info( "Selected validation layer:" );
                            spdlog::info( "\tVK_LAYER_KHRONOS_validation" );
                            break;
                        }

                    // Add "VK_LAYER_LUNARG_standard_validation" as a fallback if Khronos layer isn't found.
                    if( strcmp( "VK_LAYER_LUNARG_standard_validation", layerProperties[i].layerName ) == 0 )
                        {
                            validationLayers.push_back( "VK_LAYER_LUNARG_standard_validation" );
                            spdlog::info( "" );
                            spdlog::info( "Selected fallback validation layer:" );
                            spdlog::info( "\tVK_LAYER_LUNARG_standard_validation" );
                        }
                }
        }
    m_validationLayers = validationLayers;
    spdlog::info( "" );

    spdlog::info( "Creating Vulkan Instance..." );

    /* ---------------------------------------- Vulkan Instance --------------------------------------------------------- */
    {
        VkApplicationInfo appInfo =
            {
                .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext              = nullptr,
                .pApplicationName   = "Vulkan App",
                .applicationVersion = VK_MAKE_API_VERSION( 1, 0, 0, 0 ),
                .pEngineName        = "Vulkano",
                .engineVersion      = VK_MAKE_API_VERSION( 1, 0, 0, 0 ),
                .apiVersion         = VK_API_VERSION_1_3,
            };

        VkInstanceCreateInfo createInfo =
            {
                .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo        = &appInfo,
                .enabledLayerCount       = static_cast< uint32_t >( validationLayers.size() ),
                .ppEnabledLayerNames     = validationLayers.data(),
                .enabledExtensionCount   = static_cast< uint32_t >( extensions.size() ),
                .ppEnabledExtensionNames = extensions.data(),
            };

        VK_CHECK( vkCreateInstance( &createInfo, nullptr, &instance ), "Failed to create Vulkan instance" );

        function_set_t::Link( instance );
    }

    if( enableLayers )
        {
            spdlog::info( "Creating Vulkan Debug Callback..." );

            VkDebugReportCallbackCreateInfoEXT createInfo =
                {
                    .sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                    .flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
                    .pfnCallback = VulkanErrorMessage,
                };

            VK_CHECK( function_set_t::vkCreateDebugReportCallbackEXT( instance, &createInfo, nullptr, &m_vkDebugCallback ),
                      "Failed to create debug callback" );
        }

    spdlog::info( "" );

    return true;
}

void
voDeviceContext::Cleanup()
{
    swapChain.Cleanup( this );

    // Destroy Command Buffers
    vkFreeCommandBuffers( deviceInfo.logical, m_vkCommandPool, (uint32_t)m_vkCommandBuffers.size(), m_vkCommandBuffers.data() );
    vkDestroyCommandPool( deviceInfo.logical, m_vkCommandPool, nullptr );

    vkDestroyDevice( deviceInfo.logical, nullptr );

    if( enableLayers )
        {
            function_set_t::vkDestroyDebugReportCallbackEXT( instance, m_vkDebugCallback, nullptr );
        }

    vkDestroySurfaceKHR( instance, VkSurface, nullptr );
    vkDestroyInstance( instance, nullptr );
}

bool
voDeviceContext::CreateDevice()
{
    CreatePhysicalDevice();
    CreateLogicalDevice();

    return true;
}

const char *
VendorStr( unsigned int vendorID )
{
    switch( vendorID )
        {
            case 0x1002: return "AMD";
            case 0x1010: return "ImgTec";
            case 0x10DE: return "NVIDIA";
            case 0x13B5: return "ARM";
            case 0x5143: return "Qualcomm";
            case 0x8086: return "INTEL";
            default: return "UNKNOWN";
        }
}

void
voDeviceContext::CreatePhysicalDevice()
{
    /* ---------------------------------------- Enumerate Physical devices ---------------------------------------------- */
    {
        // Get the number of devices
        uint32_t numDevices = 0;
        VK_CHECK( vkEnumeratePhysicalDevices( instance, &numDevices, VK_NULL_HANDLE ),
                  "Failed to enumerate devices" );

        if( numDevices == 0 )
            {
                throw std::runtime_error( "No devices found!" );
            }

        // Query a list of devices
        std::vector< VkPhysicalDevice > physicalDevices( numDevices );
        VK_CHECK( vkEnumeratePhysicalDevices( instance, &numDevices, physicalDevices.data() ),
                  "Failed to enumerate devices" );

        // Acquire the properties of each device
        m_physicalDevices.resize( physicalDevices.size() );
        for( uint32_t i = 0; i < physicalDevices.size(); i++ )
            {
                m_physicalDevices[i].SetProperties( physicalDevices[i], VkSurface );
            }
    }

    /* ---------------------------------------- Select Physical devices ------------------------------------------------- */

    for( int i = 0; i < m_physicalDevices.size(); ++i )
        {
            const physical_device_properties_t & deviceProperties = m_physicalDevices[i];

            // Determine invalid devices
            if( deviceProperties.presentModes.empty() || deviceProperties.surfaceFormats.empty() )
                {
                    continue;
                }

            // Verify required extension support
            if( !deviceProperties.HasExtensionsSupport( (const char **)m_deviceExtensions.data(),
                                                        (int)m_deviceExtensions.size() ) )
                {
                    continue;
                }

            /* ---------------------------------------- Get graphics queue family ------------------------------------------- */

            int graphicsID = -1;
            for( int j = 0; j < deviceProperties.queueFamilyProperties.size(); ++j )
                {
                    const VkQueueFamilyProperties & props = deviceProperties.queueFamilyProperties[j];

                    if( props.queueCount == 0 )
                        {
                            continue;
                        }

                    if( props.queueFlags & VK_QUEUE_GRAPHICS_BIT )
                        {
                            graphicsID = j;
                            break;
                        }
                }

            if( graphicsID < 0 )
                {
                    // Device does not support graphics
                    continue;
                }

            /* ---------------------------------------- Get present queue family -------------------------------------------- */

            int presentID = -1;
            for( int j = 0; j < deviceProperties.queueFamilyProperties.size(); ++j )
                {
                    const VkQueueFamilyProperties & props = deviceProperties.queueFamilyProperties[j];

                    if( props.queueCount == 0 )
                        {
                            continue;
                        }

                    VkBool32 supportsPresentQueue = VK_FALSE;
                    VK_CHECK( vkGetPhysicalDeviceSurfaceSupportKHR( deviceProperties.physicalDevice, j, VkSurface,
                                                                    &supportsPresentQueue ),
                              "Failed to vkGetPhysicalDeviceSurfaceSupportKHR" );
                    if( supportsPresentQueue )
                        {
                            presentID = j;
                            break;
                        }
                }

            if( presentID < 0 )
                {
                    // Device does not support presentation
                    continue;
                }

            /* ---------------------------------------- Get first device ---------------------------------------------------- */

            queueIds            = { graphicsID, presentID };
            deviceInfo.physical = deviceProperties.physicalDevice;
            deviceInfo.index    = i;

            uint32_t apiMajor = VK_VERSION_MAJOR( deviceProperties.deviceProperties.apiVersion );
            uint32_t apiMinor = VK_VERSION_MINOR( deviceProperties.deviceProperties.apiVersion );
            uint32_t apiPatch = VK_VERSION_PATCH( deviceProperties.deviceProperties.apiVersion );

            uint32_t driverMajor = VK_VERSION_MAJOR( deviceProperties.deviceProperties.driverVersion );
            uint32_t driverMinor = VK_VERSION_MINOR( deviceProperties.deviceProperties.driverVersion );
            uint32_t driverPatch = VK_VERSION_PATCH( deviceProperties.deviceProperties.driverVersion );

            spdlog::info( "-------------------------------------------------" );
            spdlog::info( "Physical Device Chosen: {}", deviceProperties.deviceProperties.deviceName );
            spdlog::info( "API Version: {}.{}.{}", apiMajor, apiMinor, apiPatch );
            spdlog::info( "Driver Version: {}.{}.{}", driverMajor, driverMinor, driverPatch );
            spdlog::info( "Vendor ID: {}  {}", deviceProperties.deviceProperties.vendorID, VendorStr( deviceProperties.deviceProperties.vendorID ) );
            spdlog::info( "Device ID: {}", deviceProperties.deviceProperties.deviceID );
            spdlog::info( "" );

            // Found a valid device
            return;
        }

    throw std::runtime_error( "No Physical Device found!" );
}

bool
voDeviceContext::CreateLogicalDevice()
{
    std::vector< const char * > validationLayers {};
    if( enableLayers )
        {
            validationLayers = m_validationLayers;
        }

    float queuePriority = 1.0F;
    VkDeviceQueueCreateInfo queueCreateInfos[2] =
        {
            {
             .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .queueFamilyIndex = static_cast< uint32_t >( queueIds.graphicsFamily ),
             .queueCount       = 1,
             .pQueuePriorities = &queuePriority,
             },
            {
             .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .queueFamilyIndex = static_cast< uint32_t >( queueIds.presentationFamily ),
             .queueCount       = 1,
             .pQueuePriorities = &queuePriority,
             }
    };

    VkPhysicalDeviceFeatures deviceFeatures =
        {
            .samplerAnisotropy = VK_TRUE,
        };

    VkDeviceCreateInfo createInfo =
        {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount    = queueIds.IsGraphicsAndPresentationEqual() ? 1U : 2U,
            .pQueueCreateInfos       = queueCreateInfos,
            .enabledLayerCount       = static_cast< uint32_t >( validationLayers.size() ),
            .ppEnabledLayerNames     = validationLayers.data(),
            .enabledExtensionCount   = static_cast< uint32_t >( m_deviceExtensions.size() ),
            .ppEnabledExtensionNames = m_deviceExtensions.data(),
            .pEnabledFeatures        = &deviceFeatures,
        };

    VK_CHECK( vkCreateDevice( deviceInfo.physical, &createInfo, nullptr, &deviceInfo.logical ),
              "Failed to create logical device" );

    vkGetDeviceQueue( deviceInfo.logical, queueIds.graphicsFamily, 0, &m_vkGraphicsQueue );
    vkGetDeviceQueue( deviceInfo.logical, queueIds.presentationFamily, 0, &presentQueue );

    return true;
}

uint32_t
voDeviceContext::FindMemoryTypeIndex( uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
    VkPhysicalDeviceMemoryProperties * memProps = &GetPhysicalProperties()->memoryProperties;

    // Iterate over the memory types available on the device
    for( uint32_t i = 0; i < memProps->memoryTypeCount; ++i )
        {
            // Check if the memory type is allowed
            if( ( typeFilter & ( 1 << i ) ) == 0 )
                {
                    continue;
                }

            // Check if the memory type has the desired properties
            if( ( memProps->memoryTypes[i].propertyFlags & properties ) != properties )
                {
                    continue;
                }

            // Return the index of the memory type
            return i;
        }

    throw std::runtime_error( "Failed to find a suitable memory type!" );
}

bool
voDeviceContext::CreateCommandBuffers()
{
    /* ---------------------------------------- Command Pool ------------------------------------------------------------ */
    {
        VkCommandPoolCreateInfo poolInfo =
            {
                .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = static_cast< uint32_t >( queueIds.graphicsFamily ),
            };

        VK_CHECK( vkCreateCommandPool( deviceInfo.logical, &poolInfo, nullptr, &m_vkCommandPool ),
                  "Failed to create command pool" );
    }

    /* ---------------------------------------- Command Buffers --------------------------------------------------------- */
    {
        const int numBuffers = 16;
        m_vkCommandBuffers.resize( numBuffers );

        VkCommandBufferAllocateInfo allocInfo =
            {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = m_vkCommandPool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = static_cast< uint32_t >( m_vkCommandBuffers.size() ),
            };

        VK_CHECK( vkAllocateCommandBuffers( deviceInfo.logical, &allocInfo, m_vkCommandBuffers.data() ),
                  "Failed to allocate command buffers" );
    }

    return true;
}

VkCommandBuffer
voDeviceContext::CreateCommandBuffer( VkCommandBufferLevel level )
{
    VkCommandBufferAllocateInfo allocInfo =
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = m_vkCommandPool,
            .level              = level,
            .commandBufferCount = 1,
        };

    VkCommandBuffer cmdBuffer;
    VK_CHECK( vkAllocateCommandBuffers( deviceInfo.logical, &allocInfo, &cmdBuffer ),
              "Failed to create command buffer" );

    VkCommandBufferBeginInfo beginInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

    VK_CHECK( vkBeginCommandBuffer( cmdBuffer, &beginInfo ),
              "Failed to begin command buffer" );

    return cmdBuffer;
}

void
voDeviceContext::FlushCommandBuffer( VkCommandBuffer commandBuffer, VkQueue queue )
{
    if( commandBuffer == VK_NULL_HANDLE ) return;

    VK_CHECK( vkEndCommandBuffer( commandBuffer ),
              "Failed to end command buffer" );

    /* ---------------------------------------- Submit ------------------------------------------------------------------ */
    {
        voFence fence( this );

        VkSubmitInfo submitInfo =
            {
                .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers    = &commandBuffer,
            };

        VK_CHECK( vkQueueSubmit( queue, 1, &submitInfo, fence.GetFence() ),
                  "Failed to submit command buffer to queue!" );
    }

    vkFreeCommandBuffers( deviceInfo.logical, m_vkCommandPool, 1, &commandBuffer );
}

int
voDeviceContext::GetAligendUniformByteOffset( const int offset ) const
{
    // Get the physical device properties of the device associated with the deviceInfo index
    const VkPhysicalDeviceProperties * deviceProperties = &GetPhysicalProperties()->deviceProperties;

    // Get the minimum uniform buffer offset alignment of the physical device
    const int minByteOffsetAlignment = static_cast< int >( deviceProperties->limits.minUniformBufferOffsetAlignment );

    // Calculate the aligned uniform byte offset
    const int n             = ( offset + minByteOffsetAlignment - 1 ) / minByteOffsetAlignment;
    const int alignedOffset = n * minByteOffsetAlignment;

    return alignedOffset;
}
