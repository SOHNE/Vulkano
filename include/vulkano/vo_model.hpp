#ifndef VULKANO_MODEL_H
#define VULKANO_MODEL_H

#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include "vo_buffer.hpp"
#include "vo_deviceContext.hpp"

class aiScene;
class aiNode;
class aiMesh;
class aiMaterial;

/**
 * @struct vert_t
 * @brief Contains the position, texCoords, normals, tangents, buffs for drawable verts.
 */
struct vert_t
{
    using AttrDesc = std::array< VkVertexInputAttributeDescription, 5 >;
    // Attributes
    vec3  pos;  ///< 12 bytes - 3D vector for position
    vec2  st;   ///< 8 bytes  - 2D vector for texture coordinates
    ivec4 norm; ///< 4 bytes  - 4D vector for normals (packed as bytes)
    ivec4 tang; ///< 4 bytes  - 4D vector for tangents (packed as bytes)
    ivec4 buff; ///< 4 bytes  - 4D vector for buffer data (packed as bytes)

    /** @brief Get the binding description */
    static VkVertexInputBindingDescription
    GetBindingDescription()
    {
        return VkVertexInputBindingDescription {
            .binding   = 0,                          // Which stream index to read from
            .stride    = sizeof( vert_t ),           // Number of bytes from one entry to the next
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX // How to move between data entries
        };
    }

    /** @brief Get the attribute descriptions */
    static AttrDesc
    GetAttributeDescriptions()
    {
        return AttrDesc {
            {
#define CREATE_ATTR_DESC( L, B, F, O ) \
    ( VkVertexInputAttributeDescription ) { .location = ( L ), .binding = ( B ), .format = ( F ), .offset = offsetof( vert_t, O ) }
             CREATE_ATTR_DESC( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, pos ),
             CREATE_ATTR_DESC( 1, 0, VK_FORMAT_R32G32_SFLOAT, st ),
             CREATE_ATTR_DESC( 2, 0, VK_FORMAT_R8G8B8A8_UNORM, norm ),
             CREATE_ATTR_DESC( 3, 0, VK_FORMAT_R8G8B8A8_UNORM, tang ),
             CREATE_ATTR_DESC( 4, 0, VK_FORMAT_R8G8B8A8_UNORM, buff ),
#undef CREATE_ATTR_DESC
             }
        };
    }
}; // vert_t

struct material_t
{
    // Color properties
    float diffuse[3] {};
    float specular[3] {};
    float ambient[3] {};
    float emissive[3] {};
    float shininess {};
    float opacity {};

    // Texture types enumeration
    enum TextureType
    {
        DIFFUSE = 0,
        SPECULAR,
        NORMAL,
        HEIGHT,
        AMBIENT,
        EMISSIVE,
        METALNESS,
        ROUGHNESS,
        MAX_TEXTURE_TYPES
    };

    // Texture information
    struct TextureInfo
    {
        bool        exists = false;
        std::string path;
        // Potential texture handle if pre-loaded
        // TextureHandle handle = nullptr;
    } textures[MAX_TEXTURE_TYPES];

    // PBR properties
    float metallic  = 0.0f;
    float roughness = 1.0f;

    // Material flags
    struct Flags
    {
        bool isTransparent : 1;
        bool isDoubleSided : 1;
        bool hasAlphaTest  : 1;
    } flags {};

    // Reset method
    void
    Reset()
    {
        memset( diffuse, 0, sizeof( diffuse ) );
        memset( specular, 0, sizeof( specular ) );
        memset( ambient, 0, sizeof( ambient ) );
        memset( emissive, 0, sizeof( emissive ) );
        shininess = 0.0f;
        opacity   = 1.0f;
        metallic  = 0.0f;
        roughness = 1.0f;

        // Reset texture info
        for( auto & tex : textures )
            {
                tex.exists = false;
                tex.path.clear();
            }

        // Reset flags
        *( reinterpret_cast< uint32_t * >( &flags ) ) = 0;
    }
};

/**
 * @class voModel
 * @brief A class that encapsulates a 3D Model.
 *
 * @details The voModel class is responsible for managing a 3D Model.
 * It provides functionalities for creating a model, making a Vertex Buffer Object (VBO),
 * cleaning up the model, and drawing the model using indexed drawing.
 *
 * @code
 * voDeviceContext deviceContext;
 * voModel model;
 *
 * // Make a Vertex Buffer Object (VBO)
 * model.MakeVBO(&deviceContext);
 *
 * // Draw the model using indexed drawing
 * model.DrawIndexed(cmdBuffer);
 *
 * // Cleanup
 * model.Cleanup(deviceContext);
 * @endcode
 * 
 * @see `voDeviceContext`, `voBuffer`
 */
class VO_API voModel
{
  public:
    enum class LoadFlags
    {
        None             = 0,
        Triangulate      = 1 << 0,
        SmoothNormals    = 1 << 1,
        GenerateTangents = 1 << 2,
        OptimizeMesh     = 1 << 3,
        Default          = Triangulate | SmoothNormals | GenerateTangents
    };

  public:
    voModel()  = default;
    ~voModel() = default;

    std::vector< vert_t >       m_vertices {};
    std::vector< unsigned int > m_indices {};
    std::vector< material_t >   m_materials {};

    void MakeCube();
    bool MakeVBO( voDeviceContext * device );

    // GPU Data
    bool     m_isVBO { false };
    voBuffer m_vertexBuffer {};
    voBuffer m_indexBuffer {};

    /**
     * @brief Load a 3D model from a file
     * @param filepath Path to the 3D model file
     * @param device Rendering device context
     * @param loadFlags Processing flags for model import
     * @return bool Success of model loading
     */
    bool LoadFromFile(
        const std::string & filepath,
        voDeviceContext *   device,
        LoadFlags           loadFlags = LoadFlags::Default );

    void Cleanup( voDeviceContext & deviceContext ) const;
    void DrawIndexed( VkCommandBuffer vkCommandBuffer );

  private:
    /**
     * @brief Process a single mesh from the scene
     * @param mesh Assimp mesh to process
     * @param scene Full scene context
     * @param device Rendering device context
     * @return bool Success of mesh processing
     */
    bool ProcessMesh(
        aiMesh *          mesh,
        const aiScene *   scene,
        voDeviceContext * device );

    /**
     * @brief Recursively process nodes in the scene graph
     * @param node Current scene node
     * @param scene Full scene context
     * @param device Rendering device context
     * @return bool Success of node processing
     */
    bool ProcessNode(
        aiNode *          node,
        const aiScene *   scene,
        voDeviceContext * device );

    /**
     * @brief Load materials from the scene
     * @param scene Assimp scene
     * @param modelDirectory Directory of the model file
     * @return Vector of processed materials
     */
    std::vector< material_t > ProcessMaterials(
        const aiScene *     scene,
        const std::string & modelDirectory );

    // Utility method for resolving texture paths
    std::string ResolveTexturePath(
        const std::string & modelDirectory,
        const std::string & texturePath );
};

void FillCube( voModel & model );
void FillTriangle( voModel & model );
void FillFullScreenQuad( voModel & model );

/**
 * @struct voRenderModel
 * @brief Holds the model information and the render related info, such as world position and world orientation.
 */
struct voRenderModel
{
    voModel * model;         ///< The vao buffer to draw
    uint32_t  uboByteOffset; ///< The byte offset into the uniform buffer
    uint32_t  uboByteSize;   ///< how much space we consume in the uniform buffer
    vec3      pos;           ///< CGLM vec3 for position
    versor    orient;        ///< CGLM versor (quaternion) for orientation
};

#endif //VULKANO_MODEL_H
