#pragma once
#include <Base/Common.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>



namespace gdf
{




struct GDF_EXPORT Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct GDF_EXPORT Mesh {
    std::vector<Vertex> vertices;
    static std::vector<Mesh> LoadFromFile(std::string path);
};

} // namespace gdf