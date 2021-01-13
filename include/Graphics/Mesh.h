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

struct GDF_EXPORT Model {
    std::vector<Vertex> vertices;
    static bool Load(std::string path, Model &model);
};

} // namespace gdf