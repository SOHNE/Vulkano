#ifndef VULKANO_SHADER_H
#define VULKANO_SHADER_H

#include <unordered_map>
#include "vo_api.hpp"
#include "vo_common.hpp"

class voDeviceContext;

/**
 * @class voShader
 * @brief A class that encapsulates a Vulkan Shader.
 *
 * @details The voShader class is responsible for managing a Vulkan Shader.
 * A shader is a program that runs on the GPU and is used to perform rendering effects.
 * The class provides functionalities for loading a shader, cleaning up the shader, and creating a shader module.
 *
 * @code
 * voDeviceContext deviceContext;
 * voShader shader;
 *
 * // Load a shader
 * const char* name = "shaderName";
 * shader.Load(&deviceContext, name);
 *
 * // Cleanup
 * shader.Cleanup(&deviceContext);
 * @endcode
 */
class VO_API voShader
{
  public:
    enum ShaderStage_t
    {
        SHADER_STAGE_VERTEX = 0,
        SHADER_STAGE_TESSELLATION_CONTROL,
        SHADER_STAGE_TESSELLATION_EVALUATION,
        SHADER_STAGE_GEOMETRY,
        SHADER_STAGE_FRAGMENT,
        SHADER_STAGE_COMPUTE,
        SHADER_STAGE_RAYGEN,
        SHADER_STAGE_ANY_HIT,
        SHADER_STAGE_CLOSEST_HIT,
        SHADER_STAGE_MISS,
        SHADER_STAGE_INTERSECTION,
        SHADER_STAGE_CALLABLE,
        SHADER_STAGE_TASK,
        SHADER_STAGE_MESH,

        SHADER_STAGE_NUM,
    };

    struct voModules_t
    {
        VkShaderStageFlagBits stage;
        VkShaderModule module;
    };

  public:
    voShader()  = default;
    ~voShader() = default;

    /**
     * @brief Loads the shader with the given name
     *
     * @param device The Vulkan device context
     * @param name The name of the shader to load
     *
     * @return True if the shader was loaded successfully, false otherwise
     */
    bool Load( voDeviceContext * device, const char * name );

    /**
     * @brief Cleans up the shader
     *
     * @param device The Vulkan device context
     */
    void Cleanup( voDeviceContext * device );

  private:
    /**
     * @brief Creates a Vulkan shader module
     *
     * @param vkDevice The Vulkan device
     * @param code The shader code
     * @param size The size of the shader code
     *
     * @return The created VkShaderModule object
     */
    static VkShaderModule CreateShaderModule( VkDevice vkDevice, const char * code, int size );

  public:
    std::unordered_map< ShaderStage_t, voModules_t > modules {};
};

#endif //VULKANO_SHADER_H
