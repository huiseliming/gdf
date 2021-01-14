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
std::vector<Model> Model::LoadFromFile(std::string path)
{
    std::vector<Model> models;
    tinygltf::Model gltfmodel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    auto ret = loader.LoadASCIIFromFile(&gltfmodel, &err, &warn, path);
    if (!ret) GDF_LOG(GraphicsLog, LogLevel::Error, "Failed to parse glTF!");
    if (!err.empty()) GDF_LOG(GraphicsLog, LogLevel::Error, "Model load error :{}", err);
    if (!warn.empty()) GDF_LOG(GraphicsLog, LogLevel::Error, "Model load warn :{}", warn);

    for (uint32_t i = 0; i < gltfmodel.meshes.size(); i++){
        const tinygltf::Accessor &accessor = gltfmodel.accessors[gltfmodel.meshes[i].primitives[0].attributes["POSITION"]];
        const tinygltf::BufferView &bufferView = gltfmodel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = gltfmodel.buffers[bufferView.buffer];
        const float *positions = reinterpret_cast<const float *>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
        for (size_t i = 0; i < accessor.count; ++i) {
            std::cout << "(" << positions[i * 3 + 0] << ", " << positions[i * 3 + 1] << ", " << positions[i * 3 + 2] << ")\n";
        }
    }

    return models;
}

} // namespace gdf