#include "vulkano/vo_shader.hpp"

#include <vulkan/vulkan_core.h>
#include "vo_utilities.hpp"
#include "vulkano/vo_deviceContext.hpp"
#include "vulkano/vo_tools.hpp"

bool
voShader::Load( voDeviceContext * device, const char * name )
{
    // Mount the shaders file fileExtensions table
    const char * fileExtensions[SHADER_STAGE_NUM] {};
#define EXT( S, N ) fileExtensions[SHADER_STAGE_##S] = N
    EXT( VERTEX, "vert" );
    EXT( TESSELLATION_CONTROL, "tess" );
    EXT( TESSELLATION_EVALUATION, "tval" );
    EXT( GEOMETRY, "geom" );
    EXT( FRAGMENT, "frag" );
    EXT( COMPUTE, "comp" );
    EXT( RAYGEN, "rgen" );
    EXT( ANY_HIT, "ahit" );
    EXT( CLOSEST_HIT, "chit" );
    EXT( MISS, "miss" );
    EXT( INTERSECTION, "rint" );
    EXT( CALLABLE, "call" );
    EXT( TASK, "task" );
    EXT( MESH, "mesh" );
#undef EXT

    // Seach for shaders
    for( int i = 0; i < SHADER_STAGE_NUM; i++ )
        {
            std::string nameSpirv = fmt::format( "data/shaders/spirv/{}.{}.spirv", name, fileExtensions[i] );
            if( !FileExisits( nameSpirv ) ) continue;

            if( auto fileDataOpt = GetFileData( nameSpirv ) )
                {
                    const auto & code = *fileDataOpt;
                    auto id           = static_cast< ShaderStage_t >( i );

                    VkShaderModule module = voShader::CreateShaderModule(
                        device->deviceInfo.logical,
                        reinterpret_cast< const char * >( code.data() ),
                        static_cast< int >( code.size() ) );

                    VkShaderStageFlagBits stage = GetShaderStageFlag( i );
                    modules[id]               = { stage, module };
                }
        }

    return true;
}

void
voShader::Cleanup( voDeviceContext * device )
{
    for( const auto & [key, moduleData] : modules )
        {
            vkDestroyShaderModule( device->deviceInfo.logical, moduleData.module, nullptr );
        }
    modules.clear();
}

VkShaderModule
voShader::CreateShaderModule( VkDevice vkDevice, const char * code, const int size )
{
    VkShaderModuleCreateInfo createInfo =
        {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = static_cast< size_t >( size ),
            .pCode    = reinterpret_cast< const uint32_t * >( code ),
        };

    VkShaderModule shaderModule {};
    VK_CHECK( vkCreateShaderModule( vkDevice, &createInfo, nullptr, &shaderModule ),
              "Failed to create shader module" );

    return shaderModule;
}
