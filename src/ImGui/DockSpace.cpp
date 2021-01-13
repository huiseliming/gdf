#include "ImGui/DockSpace.h"

#include "imgui.h"

namespace gdf
{
void ImGuiDockSpace()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}
} // namespace gdf