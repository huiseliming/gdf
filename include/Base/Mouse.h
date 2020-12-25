#pragma once
#include "Base/Common.h"
#include <functional>
#include <glm/vec2.hpp>
#include <set>
#include <vector>

namespace gdf
{
class Window;

struct GDF_EXPORT Mouse {
    bool inWindow;
    bool oldInWindow;

    glm::vec2 position;
    glm::vec2 oldPosition;

    glm::vec2 offset;

    std::set<int32_t> pressedButtons;
    std::set<int32_t> releasedButtons;
    std::set<int32_t> heldButtons;

    std::vector<std::function<void(const Window &, const Mouse &)>> mouseEntredCallbacks_;
    std::vector<std::function<void(const Window &, const Mouse &)>> mouseExitedCallbacks_;

    bool IsPressed(int32_t button)
    {
        return pressedButtons.find(button) != std::end(pressedButtons);
    }

    bool IsReleased(int32_t button)
    {
        return releasedButtons.find(button) != std::end(releasedButtons);
    }

    bool IsHeld(int32_t button)
    {
        return heldButtons.find(button) != std::end(heldButtons);
    }

    friend class Window;

private:
    void preProcessing(const Window &)
    {
        oldInWindow = inWindow;
        oldPosition = position;
        offset = {0, 0};
        pressedButtons.clear();
        releasedButtons.clear();
    }
    void postProcessing(const Window &window)
    {
        if (inWindow && !oldInWindow) {
            for (auto &mouseEntredCallback : mouseEntredCallbacks_)
                mouseEntredCallback(window, *this);
        } else if (!inWindow && oldInWindow) {
            for (auto &mouseExitedCallback : mouseExitedCallbacks_)
                mouseExitedCallback(window, *this);
        }
    }
};
} // namespace gdf