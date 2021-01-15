#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
#define JSON_NOEXCEPTION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14
#include <nlohmann/json.hpp>
#include <tiny_gltf.h>
#include "Graphics/Mesh.h"
#include "Log/Logger.h"
#include "Graphics/Graphics.h"



namespace gdf
{
bool Texture::Create(tinygltf::Image &gltfImage, std::string path, VulkanDevice *device, VkQueue transferQueue)
{
    bool isKtx = false;
    // Image points to an external ktx file
    if (gltfImage.uri.find_last_of(".") != std::string::npos) {
        if (gltfImage.uri.substr(gltfImage.uri.find_last_of(".") + 1) == "ktx") {
            isKtx = true;
        }
    }
    if (!isKtx) {
        uint8_t *pBuffer = nullptr;
        VkDeviceSize bufferSize;
        std::vector<uint8_t> buffer;
        if (gltfImage.component == 3) {
            bufferSize = gltfImage.width * gltfImage.height * 4;
            buffer.resize(bufferSize);
            uint8_t *rgba = buffer.data();
            uint8_t *rgb = gltfImage.image.data();
            for (size_t i = 0; i < gltfImage.width * gltfImage.height; i++) {
                rgba[i] = rgb[i];
                rgba[i + 1] = rgb[i + 1];
                rgba[i + 2] = rgb[i + 2];
                rgba += 4;
                rgb += 3;
            }
            pBuffer = buffer.data();
        } else {
            pBuffer = gltfImage.image.data();
            bufferSize = gltfImage.image.size();
        }
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

        VkFormatProperties formatProperties;

        width = gltfImage.width;
        height = gltfImage.height;
        mipLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);
    }
    return true;
}

void Texture::Destroy()
{
}

void Model::tinygltfLoadImage(tinygltf::Model gltfModel, VulkanDevice *device, VkQueue transferQueue)
{
    for (tinygltf::Image &gltfImage : gltfModel.images) {
        Texture texture;
        texture.Create(gltfImage, path, device ,transferQueue);
        textures.push_back(texture);
    }
}
void Model::tinygltfLoadNode(Node *parent,
                             const tinygltf::Node &node,
                             uint32_t nodeIndex,
                             const tinygltf::Model &model,
                             std::vector<uint32_t> &indexBuffer,
                             std::vector<Vertex> &vertexBuffer,
                             float globalscale)
{
    Node *newNode = new Node{};
    newNode->name = node.name;
    if (parent) {
        newNode->parent = parent;
    } else {
        nodes.push_back(newNode);
    }
    for (size_t i = 0; i < node.children.size(); i++)
        tinygltfLoadNode(newNode, model.nodes[node.children[i]], node.children[i], model, indexBuffer, vertexBuffer, globalscale);
    if (parent)  parent->children.push_back(newNode);
    newNode->children = {};
    newNode->index = nodeIndex;
    newNode->matrix = glm::mat4(1.0f);
    newNode->mesh = nullptr;
    if (node.mesh > -1) {
        const tinygltf::Mesh& mesh=  model.meshes[node.mesh];
        Mesh *newMesh = new Mesh;
        newMesh->name = mesh.name;
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive &primitive = mesh.primitives[i];
            if (primitive.indices <= 0) //不处理非索引的顶点
                continue;
            uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 minPos{};
            glm::vec3 maxPos{};
            bool hasSkin = false;
            {   // Vertices
                const float *bufferPos = nullptr;
                const float *bufferNormals = nullptr;
                const float *bufferTexCoords = nullptr;
                const float *bufferColors = nullptr;
                const float *bufferTangents = nullptr;
                const uint16_t *bufferJoints = nullptr;
                const float *bufferWeights = nullptr;
                uint32_t numColorComponents;

                size_t byteStridePos = 0;
                size_t byteStrideNormals = 0;
                size_t byteStrideTexCoords = 0;
                size_t byteStrideColors = 0;
                size_t byteStrideTangents = 0;
                size_t byteStrideJoints = 0;
                size_t byteStrideWeights = 0;

                // Position
                assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
                const tinygltf::Accessor &accessorPos = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView &bufferViewPos = model.bufferViews[accessorPos.bufferView];
                byteStridePos = bufferViewPos.byteStride;
                bufferPos = reinterpret_cast<const float *>(&(model.buffers[bufferViewPos.buffer].data[accessorPos.byteOffset + bufferViewPos.byteOffset]));
                minPos = glm::vec3(accessorPos.minValues[0], accessorPos.minValues[1], accessorPos.minValues[2]);
                maxPos = glm::vec3(accessorPos.maxValues[0], accessorPos.maxValues[1], accessorPos.maxValues[2]);

#define VERTEX_COMPONENT_X(ComponentName, ComponentByteStride, pBuffer)                                                    \
if (primitive.attributes.find(ComponentName) != primitive.attributes.end()) {                                              \
    const tinygltf::Accessor &accessorNormal = model.accessors[primitive.attributes.find(ComponentName)->second];          \
    const tinygltf::BufferView &bufferViewNormal = model.bufferViews[accessorNormal.bufferView];                           \
    ComponentByteStride = bufferViewNormal.byteStride;                                                                     \
    pBuffer = reinterpret_cast<const float *>(                                                                             \
        &(model.buffers[bufferViewNormal.buffer].data[accessorNormal.byteOffset + bufferViewNormal.byteOffset]));          \
}
                // Normal
                VERTEX_COMPONENT_X("NORMAL", byteStrideNormals, bufferNormals);
                // Texcoord
                VERTEX_COMPONENT_X("TEXCOORD_0", byteStrideTexCoords, bufferTexCoords);
                // Color
                if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor &accessorColor = model.accessors[primitive.attributes.find("COLOR_0")->second];
                    const tinygltf::BufferView &bufferViewColor = model.bufferViews[accessorColor.bufferView];
                    byteStrideColors = bufferViewColor.byteStride;
                    // Color buffer are either of type vec3 or vec4
                    numColorComponents = accessorColor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
                    bufferColors = reinterpret_cast<const float *>( &(model.buffers[bufferViewColor.buffer].data[accessorColor.byteOffset + bufferViewColor.byteOffset]));
                }
                // Tangent
                VERTEX_COMPONENT_X("TANGENT", byteStrideTangents, bufferTangents);
                // Joints
                if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor &accessorJoint = model.accessors[primitive.attributes.find("JOINTS_0")->second];
                    const tinygltf::BufferView &bufferViewJoint = model.bufferViews[accessorJoint.bufferView];
                    byteStrideJoints = bufferViewJoint.byteStride;
                    bufferJoints = reinterpret_cast<const uint16_t *>(&(model.buffers[bufferViewJoint.buffer].data[accessorJoint.byteOffset + bufferViewJoint.byteOffset]));
                }
                // Weights
                VERTEX_COMPONENT_X("WEIGHTS_0", byteStrideWeights, bufferWeights);
#undef VERTEX_COMPONENT_X
                hasSkin = (bufferJoints && bufferWeights);
                // write into 
                vertexCount = static_cast<uint32_t>(accessorPos.count);
                for (size_t i = 0; i < accessorPos.count; i++) {
                    Vertex vert{};
#define VERTEX_COMPONENT_OFFSET(Start, ByteStride, Count)\
        (ByteStride == 0 ? (const float *)(((uint8_t *)Start) + (i * Count)) : (const float *)(((uint8_t *)Start) + (ByteStride * i)))
                    vert.pos = glm::make_vec3(VERTEX_COMPONENT_OFFSET(bufferPos, byteStridePos, 3));
                    vert.normal = glm::normalize(glm::vec3(bufferNormals
                        ? glm::make_vec3(VERTEX_COMPONENT_OFFSET(bufferNormals,  byteStrideNormals, 3))
                        : glm::vec3(0.0f)));
                    vert.uv = bufferTexCoords 
                        ? glm::make_vec2(VERTEX_COMPONENT_OFFSET(bufferTexCoords, byteStrideTexCoords, 2))
                        : glm::vec3(0.0f);
                    if (bufferColors) {
                        switch (numColorComponents) {
                        case 3:
                            vert.color = glm::vec4(glm::make_vec3(VERTEX_COMPONENT_OFFSET(bufferColors, byteStrideColors, 3)), 1.0f);
                        case 4:
                            vert.color = glm::make_vec4(VERTEX_COMPONENT_OFFSET(bufferColors, byteStrideColors, 4));
                        }
                    } else {
                        vert.color = glm::vec4(1.0f);
                    }
                    vert.tangent = bufferTangents 
                        ? glm::vec4(glm::make_vec4(VERTEX_COMPONENT_OFFSET(bufferTangents, byteStrideTangents, 4))) 
                        : glm::vec4(0.0f);
                    vert.joint0 = hasSkin 
                        ? glm::vec4(glm::make_vec4(VERTEX_COMPONENT_OFFSET(bufferJoints, byteStrideJoints, 4))) 
                        : glm::vec4(0.0f);
                    vert.weight0 = hasSkin 
                        ? glm::make_vec4(VERTEX_COMPONENT_OFFSET(bufferWeights, byteStrideWeights, 4)) 
                        : glm::vec4(0.0f);
                    vertexBuffer.push_back(vert);
#undef VERTEX_COMPONENT_OFFSET
                }
            }

            {   // Indices
                const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    uint32_t *indices = (uint32_t *)(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                    for (size_t i = 0; i < accessor.count; i++) 
                        indexBuffer.push_back(indices[i] + vertexStart);
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    uint16_t *indices = (uint16_t *)(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                    for (size_t i = 0; i < accessor.count; i++)
                        indexBuffer.push_back(indices[i] + vertexStart);
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    uint8_t *indices = (uint8_t *)(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                    for (size_t i = 0; i < accessor.count; i++)
                        indexBuffer.push_back(indices[i] + vertexStart);
                    break;
                }
                default:
                    GDF_LOG(GraphicsLog, LogLevel::Error, "Index component type {} not supported!", accessor.componentType);
                    return;
                }
                //
                Primitive *newPrimitive = new Primitive();
                newPrimitive->firstVertex = vertexStart;
                newPrimitive->vertexCount = vertexCount;
                newPrimitive->firstIndex = indexStart;
                newPrimitive->indexCount = indexCount;
                newPrimitive->material = &(primitive.material > -1 ? materials[primitive.material] : materials.back());
                newPrimitive->dimensions.max = maxPos;
                newPrimitive->dimensions.min = minPos;
                newMesh->primitives.push_back(newPrimitive);
            }
            newNode->mesh = newMesh;
        }
    }
    //newNode->skin = ;
    newNode->skinIndex = node.skin;
    // Generate local node matrix
    if (node.translation.size() == 3) {
        newNode->translation = glm::make_vec3(node.translation.data());
    } else {
        newNode->translation = glm::vec3(0.0f);
    }
    if (node.rotation.size() == 4) {
        newNode->rotation = glm::make_quat(node.rotation.data());
    } else {
        newNode->translation = glm::vec3(0.0f);
    }
    if (node.scale.size() == 3) {
        newNode->scale = glm::make_vec3(node.scale.data());
    } else {
        newNode->scale = glm::vec3(1.0f);
    }

    linearNodes.emplace_back(newNode);
}


Model *Model::LoadFromFile(std::string filename)
{
    Model *model = new Model;
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string err;
    std::string warn;
    auto ret = gltfContext.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);
    if (!ret)          GDF_LOG(GraphicsLog, LogLevel::Error, "Failed to parse glTF!");
    if (!err.empty())  GDF_LOG(GraphicsLog, LogLevel::Error, "Model load error :{}", err);
    if (!warn.empty()) GDF_LOG(GraphicsLog, LogLevel::Error, "Model load warn :{}", warn);

    size_t pos = filename.find_last_of('/');
    model->path = filename.substr(0, pos);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;

    const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        const tinygltf::Node& node = gltfModel.nodes[scene.nodes[i]];
        model->tinygltfLoadNode(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, vertexBuffer, 1.0f);
    }

    return model;
}

Model::~Model()
{
    for (auto node : nodes)
        delete node;
    nodes.clear();

}

} // namespace gdf