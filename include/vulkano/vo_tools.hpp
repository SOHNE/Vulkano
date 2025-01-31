#ifndef VULKANO_TOOLS_H
#define VULKANO_TOOLS_H

#include <vulkan/vulkan.h>


static VkShaderStageFlagBits
GetShaderStageFlag( const int & id  )
{
    VkShaderStageFlagBits stage { };
    switch ( id )
    {
        default:
#define STG( I, R, ... ) case I: { stage = VK_SHADER_STAGE_ ##R## _BIT ##__VA_ARGS__; } break
        STG( 0, VERTEX );
        STG( 1, TESSELLATION_CONTROL );
        STG( 2, TESSELLATION_EVALUATION );
        STG( 3, GEOMETRY );
        STG( 4, FRAGMENT );
        STG( 5, COMPUTE );
        STG( 6, RAYGEN, _NV);
        STG( 7, ANY_HIT, _NV);
        STG( 8, CLOSEST_HIT, _NV);
        STG( 9, MISS, _NV);
        STG( 10, INTERSECTION, _NV);
        STG( 11, CALLABLE, _NV);
        STG( 12, TASK, _NV);
        STG( 13, MESH, _NV);
#undef STG
    }

    return stage;
}

#endif // VULKANO_TOOLS_H
