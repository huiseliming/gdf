#pragma once
#include <glm/glm.hpp>
#include <tiny_gltf.h>


namespace gdf
{

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct Model {
    std::vector<Vertex> vertices;
    static bool Load(std::string path, Model& model)
    {
        tinygltf::Model gltfmodel;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        loader.LoadASCIIFromFile(&gltfmodel, &err, &warn, path);
    }
};

} // namespace gdf