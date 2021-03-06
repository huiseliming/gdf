#pragma once
#include "Base/Common.h"
#include "Graphics/VulkanApi.h"
#include "Resource.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <vector>

namespace tinygltf
{
class Node;
class Model;
struct Image;
} // namespace tinygltf

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
    bool Create(tinygltf::Image &gltfImage, std::string path, VulkanDevice *device, VkQueue transferQueue);
    void Destroy();
};

struct Material {
    enum AplhaMode : uint32_t
    {
        kAlphaModeOpaque,
        kAlphaModeMask,
        kAlphaModeBlend
    };
    VkDevice device;
    // float alphaCutoff = 1.0f;
    // float metallicFactor = 1.0f;
    // float roughnessFactor = 1.0f;
    AplhaMode alphaMode{kAlphaModeOpaque};
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

struct Primitive {
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t firstVertex;
    uint32_t vertexCount;
    Material *material;
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
    std::vector<Primitive *> primitives;
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
    Mesh *mesh = nullptr;
    Skin *skin = nullptr;
    int32_t skinIndex = -1;
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};
    glm::mat4 localMatrix();
    glm::mat4 getMatrix();
};

struct Vertex {

    enum class Component
    {
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

struct Model {
    std::string path;
    std::vector<Texture> textures;
    std::vector<Material> materials{{}};
    std::vector<Primitive *> primitives;
    std::vector<Node *> nodes;
    std::vector<Node *> linearNodes;

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

    // std::vector<Node*>
    void tinygltfLoadImage(tinygltf::Model gltfModel, VulkanDevice *device, VkQueue transferQueue);
    void tinygltfLoadNode(Node *parent,
                          const tinygltf::Node &node,
                          uint32_t nodeIndex,
                          const tinygltf::Model &model,
                          std::vector<uint32_t> &indexBuffer,
                          std::vector<Vertex> &vertexBuffer,
                          float globalscale);

    static Model *LoadFromFile(std::string filename);

    ~Model();
};

} // namespace gdf