#include "vulkano/vo_model.hpp"
#include <cstring>
#include <iostream>

#include <assimp/Importer.hpp>

#include <assimp/scene.h>

#include <assimp/postprocess.h>

unsigned char
FloatToByte_n11( const float f )
{
    int i = (int)( f * 127 + 128 );
    return (unsigned char)i;
}

unsigned char
FloatToByte_01( const float f )
{
    int i = (int)( f * 255 );
    return (unsigned char)i;
}

void
FillFullScreenQuad( voModel & model )
{
    const int numVerts = 4;
    const int numIdxs  = 6;
    vert_t    screenVerts[numVerts];
    int       screenIndices[numIdxs];

    memset( screenVerts, 0, sizeof( vert_t ) * 4 );

    screenVerts[0].pos[0] = -1.0f;
    screenVerts[0].pos[1] = -1.0f;
    screenVerts[0].pos[2] = 0.0f;

    screenVerts[1].pos[0] = 1.0f;
    screenVerts[1].pos[1] = -1.0f;
    screenVerts[1].pos[2] = 0.0f;

    screenVerts[2].pos[0] = 1.0f;
    screenVerts[2].pos[1] = 1.0f;
    screenVerts[2].pos[2] = 0.0f;

    screenVerts[3].pos[0] = -1.0f;
    screenVerts[3].pos[1] = 1.0f;
    screenVerts[3].pos[2] = 0.0f;

    screenVerts[0].st[0] = 0.0f;
    screenVerts[0].st[1] = 1.0f;

    screenVerts[1].st[0] = 1.0f;
    screenVerts[1].st[1] = 1.0f;

    screenVerts[2].st[0] = 1.0f;
    screenVerts[2].st[1] = 0.0f;

    screenVerts[3].st[0] = 0.0f;
    screenVerts[3].st[1] = 0.0f;

    screenVerts[0].buff[0] = 255;
    screenVerts[1].buff[0] = 255;
    screenVerts[2].buff[0] = 255;
    screenVerts[3].buff[0] = 255;

    screenIndices[0] = 0;
    screenIndices[1] = 1;
    screenIndices[2] = 2;

    screenIndices[3] = 0;
    screenIndices[4] = 2;
    screenIndices[5] = 3;

    for( int i = 0; i < numVerts; i++ )
        {
            model.m_vertices.push_back( screenVerts[i] );
        }

    for( int screenIndice : screenIndices )
        {
            model.m_indices.push_back( screenIndice );
        }
}

void
FillCube( voModel & model )
{
    const int numIdxs  = 3 * 2 * 6;
    const int numVerts = 4 * 6;
    vert_t    cubeVerts[numVerts];
    int       cubeIdxs[numIdxs];

    memset( cubeVerts, 0, sizeof( vert_t ) * 4 * 6 );

    for( int face = 0; face < 6; face++ )
        {
            const int   dim0 = face / 2;
            const int   dim1 = ( dim0 + 1 ) % 3;
            const int   dim2 = ( dim0 + 2 ) % 3;
            const float val  = ( ( face & 1 ) == 0 ) ? -1.0f : 1.0f;

            cubeVerts[face * 4 + 0].pos[dim0] = val;
            cubeVerts[face * 4 + 0].pos[dim1] = val;
            cubeVerts[face * 4 + 0].pos[dim2] = val;

            cubeVerts[face * 4 + 1].pos[dim0] = val;
            cubeVerts[face * 4 + 1].pos[dim1] = -val;
            cubeVerts[face * 4 + 1].pos[dim2] = val;

            cubeVerts[face * 4 + 2].pos[dim0] = val;
            cubeVerts[face * 4 + 2].pos[dim1] = -val;
            cubeVerts[face * 4 + 2].pos[dim2] = -val;

            cubeVerts[face * 4 + 3].pos[dim0] = val;
            cubeVerts[face * 4 + 3].pos[dim1] = val;
            cubeVerts[face * 4 + 3].pos[dim2] = -val;

            cubeVerts[face * 4 + 0].st[0] = 0.0f;
            cubeVerts[face * 4 + 0].st[1] = 1.0f;

            cubeVerts[face * 4 + 1].st[0] = 1.0f;
            cubeVerts[face * 4 + 1].st[1] = 1.0f;

            cubeVerts[face * 4 + 2].st[0] = 1.0f;
            cubeVerts[face * 4 + 2].st[1] = 0.0f;

            cubeVerts[face * 4 + 3].st[0] = 0.0f;
            cubeVerts[face * 4 + 3].st[1] = 0.0f;

            cubeVerts[face * 4 + 0].norm[dim0] = FloatToByte_n11( val );
            cubeVerts[face * 4 + 1].norm[dim0] = FloatToByte_n11( val );
            cubeVerts[face * 4 + 2].norm[dim0] = FloatToByte_n11( val );
            cubeVerts[face * 4 + 3].norm[dim0] = FloatToByte_n11( val );

            cubeVerts[face * 4 + 0].tang[dim1] = FloatToByte_n11( val );
            cubeVerts[face * 4 + 1].tang[dim1] = FloatToByte_n11( val );
            cubeVerts[face * 4 + 2].tang[dim1] = FloatToByte_n11( val );
            cubeVerts[face * 4 + 3].tang[dim1] = FloatToByte_n11( val );

            cubeIdxs[face * 6 + 0] = face * 4 + 0;
            cubeIdxs[face * 6 + 1] = face * 4 + 1;
            cubeIdxs[face * 6 + 2] = face * 4 + 2;

            cubeIdxs[face * 6 + 3] = face * 4 + 0;
            cubeIdxs[face * 6 + 4] = face * 4 + 2;
            cubeIdxs[face * 6 + 5] = face * 4 + 3;
        }

    for( const auto & cubeVert : cubeVerts )
        {
            model.m_vertices.push_back( cubeVert );
        }

    for( int cubeIdx : cubeIdxs )
        {
            model.m_indices.push_back( cubeIdx );
        }
}

void
FillTriangle( voModel & model )
{
    const int               numVerts = 3;
    const int               numIdxs  = 3;
    std::vector< vert_t >   triangleVerts( numVerts );
    std::vector< uint32_t > triangleIdxs( numIdxs );

    // Define vertex positions
    triangleVerts[0].pos[0] = -1.0f; // x
    triangleVerts[0].pos[1] = 1.0f;  // y
    triangleVerts[0].pos[2] = 0.0f;  // z

    triangleVerts[1].pos[0] = 1.0f; // x
    triangleVerts[1].pos[1] = 1.0f; // y
    triangleVerts[1].pos[2] = 0.0f; // z

    triangleVerts[2].pos[0] = 0.0f;  // x
    triangleVerts[2].pos[1] = -1.0f; // y
    triangleVerts[2].pos[2] = 0.0f;  // z

    // Define texture coordinates
    triangleVerts[0].st[0] = 0.0f;
    triangleVerts[0].st[1] = 0.0f;

    triangleVerts[1].st[0] = 1.0f;
    triangleVerts[1].st[1] = 0.0f;

    triangleVerts[2].st[0] = 0.5f;
    triangleVerts[2].st[1] = 1.0f;

    // Define normals (pointing out of the screen)
    for( auto & triangleVert : triangleVerts )
        {
            triangleVert.norm[0] = FloatToByte_n11( 0.0f );
            triangleVert.norm[1] = FloatToByte_n11( 0.0f );
            triangleVert.norm[2] = FloatToByte_n11( 1.0f );
            triangleVert.norm[3] = 0; // Ensure fourth component is initialized
        }

    // Define tangents
    for( auto & triangleVert : triangleVerts )
        {
            triangleVert.tang[0] = FloatToByte_n11( 1.0f );
            triangleVert.tang[1] = FloatToByte_n11( 0.0f );
            triangleVert.tang[2] = FloatToByte_n11( 0.0f );
            triangleVert.tang[3] = 0; // Ensure fourth component is initialized
        }

    // Optional: Initialize buff to zero or some default value
    for( auto & triangleVert : triangleVerts )
        {
            triangleVert.buff[0] = 0;
            triangleVert.buff[1] = 0;
            triangleVert.buff[2] = 0;
            triangleVert.buff[3] = 0;
        }

    // Define indices
    triangleIdxs[0] = 0;
    triangleIdxs[1] = 1;
    triangleIdxs[2] = 2;

    // Add vertices to model using insert
    model.m_vertices.insert(
        model.m_vertices.end(),
        triangleVerts.begin(),
        triangleVerts.end() );

    // Add indices to model using insert
    model.m_indices.insert(
        model.m_indices.end(),
        triangleIdxs.begin(),
        triangleIdxs.end() );
}

bool
voModel::MakeVBO( voDeviceContext * device )
{
    VkCommandBuffer vkCommandBuffer = device->m_vkCommandBuffers[0];

    int bufferSize;

    // Create Vertex Buffer
    bufferSize = (int)( sizeof( m_vertices[0] ) * m_vertices.size() );
    if( !m_vertexBuffer.Allocate( device, m_vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) )
        {
            printf( "failed to allocate vertex buffer!\n" );
            assert( 0 );
            return false;
        }

    // Create Index Buffer
    bufferSize = (int)( sizeof( m_indices[0] ) * m_indices.size() );
    if( !m_indexBuffer.Allocate( device, m_indices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) )
        {
            printf( "failed to allocate index buffer!\n" );
            assert( 0 );
            return false;
        }

    m_isVBO = true;
    return true;
}

bool
voModel::LoadFromFile(
    const std::string & filepath,
    voDeviceContext *   device,
    LoadFlags           loadFlags )
{
    // Reset existing model data
    Cleanup( *device );

    // Determine Assimp processing flags based on LoadFlags
    unsigned int assimpFlags = 0;
    if( static_cast< int >( loadFlags ) & static_cast< int >( LoadFlags::Triangulate ) )
        assimpFlags |= aiProcess_Triangulate;
    if( static_cast< int >( loadFlags ) & static_cast< int >( LoadFlags::SmoothNormals ) )
        assimpFlags |= aiProcess_GenSmoothNormals;
    if( static_cast< int >( loadFlags ) & static_cast< int >( LoadFlags::GenerateTangents ) )
        assimpFlags |= aiProcess_CalcTangentSpace;
    if( static_cast< int >( loadFlags ) & static_cast< int >( LoadFlags::OptimizeMesh ) )
        assimpFlags |= aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    // Additional standard processing
    assimpFlags |= aiProcess_JoinIdenticalVertices |
                   aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
                   aiProcess_FlipUVs;

    // Create Assimp importer
    Assimp::Importer importer;
    const aiScene *  scene = importer.ReadFile( filepath, assimpFlags );

    if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
        {
            std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

    // Extract model directory for texture resolution
    std::string modelDirectory = filepath.substr( 0, filepath.find_last_of( "/\\" ) );

    // Process materials first
    m_materials = ProcessMaterials( scene, modelDirectory );

    // Process scene graph
    bool result = ProcessNode( scene->mRootNode, scene, device );

    // Create vertex and index buffers
    return result && MakeVBO( device );
}

bool
voModel::ProcessNode(
    aiNode *          node,
    const aiScene *   scene,
    voDeviceContext * device )
{
    // Process all meshes in this node
    for( unsigned int i = 0; i < node->mNumMeshes; i++ )
        {
            aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh( mesh, scene, device );
        }

    // Recursively process child nodes
    for( unsigned int i = 0; i < node->mNumChildren; i++ )
        {
            ProcessNode( node->mChildren[i], scene, device );
        }

    return true;
}

bool
voModel::ProcessMesh(
    aiMesh *          mesh,
    const aiScene *   scene,
    voDeviceContext * device )
{
    // Track starting vertex index for this mesh
    const auto baseVertex = static_cast< uint32_t >( m_vertices.size() );

    // Process vertices
    for( unsigned int i = 0; i < mesh->mNumVertices; i++ )
        {
            vert_t vertex {};

            // Position (always exists)
            vertex.pos[0] = mesh->mVertices[i].x;
            vertex.pos[1] = mesh->mVertices[i].y;
            vertex.pos[2] = mesh->mVertices[i].z;

            // Texture coordinates (channel 0)
            if( mesh->mTextureCoords[0] )
                {
                    vertex.st[0] = mesh->mTextureCoords[0][i].x;
                    vertex.st[1] = mesh->mTextureCoords[0][i].y;
                }
            else // Handle missing UVs
                {
                    vertex.st[0] = 0.0f;
                    vertex.st[1] = 0.0f;
                }

            // Normals (if available)
            if( mesh->HasNormals() )
                {
                    // Ensure normals are normalized before packing
                    aiVector3D normal = mesh->mNormals[i];
                    normal.Normalize();

                    vertex.norm[0] = static_cast< int8_t >( normal.x * 127.0f );
                    vertex.norm[1] = static_cast< int8_t >( normal.y * 127.0f );
                    vertex.norm[2] = static_cast< int8_t >( normal.z * 127.0f );
                }
            else // Fallback normal
                {
                    vertex.norm[0] = 0;
                    vertex.norm[1] = 0;
                    vertex.norm[2] = 0;
                }

            m_vertices.push_back( vertex );
        }

    // Process indices with base vertex offset
    for( unsigned int i = 0; i < mesh->mNumFaces; i++ )
        {
            const aiFace & face = mesh->mFaces[i];

            // Validate face type (should be triangles if using aiProcess_Triangulate)
            if( face.mNumIndices != 3 )
                {
                    std::cerr << "Warning: Non-triangular face detected in mesh ("
                              << face.mNumIndices << " indices)\n";
                    continue;
                }

            for( unsigned int j = 0; j < face.mNumIndices; j++ )
                {
                    m_indices.push_back( baseVertex + face.mIndices[j] );
                }
        }

    return true;
}

std::vector< material_t >
voModel::ProcessMaterials(
    const aiScene *     scene,
    const std::string & modelDirectory )
{
    std::vector< material_t > materials;
    materials.reserve( scene->mNumMaterials );

    for( unsigned int i = 0; i < scene->mNumMaterials; ++i )
        {
            aiMaterial * aiMat = scene->mMaterials[i];
            material_t   material;
            material.Reset();

            aiColor3D color( 0.f, 0.f, 0.f );
            float     value;

            // Diffuse Color
            if( aiMat->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS )
                {
                    material.diffuse[0] = color.r;
                    material.diffuse[1] = color.g;
                    material.diffuse[2] = color.b;
                }

            // Specular Color
            if( aiMat->Get( AI_MATKEY_COLOR_SPECULAR, color ) == AI_SUCCESS )
                {
                    material.specular[0] = color.r;
                    material.specular[1] = color.g;
                    material.specular[2] = color.b;
                }

            // Ambient Color
            if( aiMat->Get( AI_MATKEY_COLOR_AMBIENT, color ) == AI_SUCCESS )
                {
                    material.ambient[0] = color.r;
                    material.ambient[1] = color.g;
                    material.ambient[2] = color.b;
                }

            // Emissive Color
            if( aiMat->Get( AI_MATKEY_COLOR_EMISSIVE, color ) == AI_SUCCESS )
                {
                    material.emissive[0] = color.r;
                    material.emissive[1] = color.g;
                    material.emissive[2] = color.b;
                }

            // Shininess
            if( aiMat->Get( AI_MATKEY_SHININESS, value ) == AI_SUCCESS )
                {
                    material.shininess = value;
                }

            // Opacity
            if( aiMat->Get( AI_MATKEY_OPACITY, value ) == AI_SUCCESS )
                {
                    material.opacity             = value;
                    material.flags.isTransparent = ( value < 1.0f );
                }

            // PBR Properties (if available)
            if( aiMat->Get( AI_MATKEY_METALLIC_FACTOR, value ) == AI_SUCCESS )
                {
                    material.metallic = value;
                }

            if( aiMat->Get( AI_MATKEY_ROUGHNESS_FACTOR, value ) == AI_SUCCESS )
                {
                    material.roughness = value;
                }

            // Texture Processing
            const struct
            {
                aiTextureType           assimpType;
                material_t::TextureType materialType;
            } textureTypeMap[] = {
                {          aiTextureType_DIFFUSE,   material_t::DIFFUSE},
                {         aiTextureType_SPECULAR,  material_t::SPECULAR},
                {           aiTextureType_HEIGHT,    material_t::HEIGHT},
                {          aiTextureType_NORMALS,    material_t::NORMAL},
                {          aiTextureType_AMBIENT,   material_t::AMBIENT},
                {         aiTextureType_EMISSIVE,  material_t::EMISSIVE},
                {        aiTextureType_METALNESS, material_t::METALNESS},
                {aiTextureType_DIFFUSE_ROUGHNESS, material_t::ROUGHNESS}
            };

            for( const auto & mapping : textureTypeMap )
                {
                    if( aiMat->GetTextureCount( mapping.assimpType ) > 0 )
                        {
                            aiString texturePath;
                            if( AI_SUCCESS == aiMat->GetTexture( mapping.assimpType, 0, &texturePath ) )
                                {
                                    material.textures[mapping.materialType].exists = true;
                                    material.textures[mapping.materialType].path =
                                        ResolveTexturePath(
                                            modelDirectory,
                                            texturePath.C_Str() );
                                }
                        }
                }

            // Additional material flags
            int twoSided = 0;
            if( AI_SUCCESS == aiMat->Get( AI_MATKEY_TWOSIDED, twoSided ) )
                {
                    material.flags.isDoubleSided = ( twoSided != 0 );
                }

            materials.push_back( material );
        }

    return materials;
}

std::string
voModel::ResolveTexturePath(
    const std::string & modelDirectory,
    const std::string & texturePath )
{
    // Construct the full path for the texture
    return modelDirectory + "/" + texturePath;
}

void
voModel::Cleanup( voDeviceContext & deviceContext ) const
{
    if( !m_isVBO ) return;

    m_vertexBuffer.Cleanup( &deviceContext );
    m_indexBuffer.Cleanup( &deviceContext );
}

void
voModel::DrawIndexed( VkCommandBuffer vkCommandBUffer )
{
    // Bind the model
    VkBuffer     vertexBuffers[] = { m_vertexBuffer.vkBuffer };
    VkDeviceSize offsets[]       = { 0 };
    vkCmdBindVertexBuffers( vkCommandBUffer, 0, 1, vertexBuffers, offsets );
    vkCmdBindIndexBuffer( vkCommandBUffer, m_indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32 );

    // Issue draw command
    vkCmdDrawIndexed( vkCommandBUffer, (uint32_t)m_indices.size(), 1, 0, 0, 0 );
}
