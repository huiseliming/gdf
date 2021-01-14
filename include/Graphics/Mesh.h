#pragma once
#include "Base/Common.h"
#include "Graphics/VulkanApi.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <memory>

namespace tinygltf
{
class Node;
class Model;
}

namespace gdf
{


struct Node;

struct Texture {
    VkDevice device;
    VkImage image;
    VkImageView imageView;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    uint32_t width, height;
    uint32_t mipLevels;
    uint32_t layerCount;
    VkDescriptorImageInfo Descriptor;
    VkSampler sampler;
};

struct Material {
    enum AplhaMode :uint32_t {kAlphaModeOpaque, kAlphaModeMask, kAlphaModeBlend };
    VkDevice device;
    AplhaMode alphaMode{kAlphaModeOpaque};
    //float alphaCutoff = 1.0f;
    //float metallicFactor = 1.0f;
    //float roughnessFactor = 1.0f;
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    Texture *baseColorTexture = nullptr;
    Texture *metallicRoughnessTexture = nullptr;
    Texture *normalTexture = nullptr;
    Texture *occlusionTexture = nullptr;
    Texture *emissiveTexture = nullptr;
    Texture *specularGlossinessTexture = nullptr;
    Texture *diffuseTexture = nullptr;
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
};

struct Primitive{
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t firstVertex;
    uint32_t vertexCount;
    Material*material;
    struct Dimensions {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
        glm::vec3 center;
        float radius;
    } dimensions;
};

struct Mesh {
    std::string name;
    std::vector<Primitive*> primitives;
    
};

struct Skin {
    std::string name;
    Node *skeletonRoot = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node *> joints;
};

struct Node {
    std::string name;
    Node *parent = nullptr;
    std::vector<Node *> children;
    uint32_t index = UINT32_MAX;
    glm::mat4 matrix;
    Mesh* mesh = nullptr;
    Skin *skin = nullptr;
    int32_t skinIndex = -1;
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};
    glm::mat4 localMatrix();
    glm::mat4 getMatrix();
};

struct GDF_EXPORT Vertex {

    enum class Component {
        Position, 
        Normal, 
        UV,
        Color, 
        Tangent, 
        Joint0, 
        Weight0
    };

    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 color;
    glm::vec4 tangent;
    glm::vec4 joint0;
    glm::vec4 weight0;
};

struct GDF_EXPORT Model {
    std::vector<Texture> textures;
    std::vector<Material> materials{{}};
    std::vector<Primitive *> primitives;
    std::vector<Node*> nodes;

    std::vector<Node*> linearNodes;

    struct {
        uint32_t count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertices;

    struct {
        uint32_t count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    } indices;

    //std::vector<Node*>
    void LoadNode(Node *parent, const tinygltf::Node &node, uint32_t nodeIndex, const tinygltf::Model &model, std::vector<uint32_t> &indexBuffer, std::vector<Vertex> &vertexBuffer, float globalscale);
    static Model* LoadFromFile(std::string path);
    ~Model();
};

} // namespace gdf