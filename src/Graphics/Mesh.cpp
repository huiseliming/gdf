#include "Graphics/Mesh.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
#define JSON_NOEXCEPTION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14
#include <nlohmann/json.hpp>
#include <tiny_gltf.h>

 bool gdf::Model::Load(std::string path, Model &model)
{
    tinygltf::Model gltfmodel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    loader.LoadASCIIFromFile(&gltfmodel, &err, &warn, path);
    return true;
}
