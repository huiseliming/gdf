#pragma once
#include <set>
namespace gdf
{
class Window;

struct Keyboard {
    int32_t unicodePoint;
    int32_t repeatKey;

    std::set<int32_t> pressedKeys;
    std::set<int32_t> releasedKeys;
    std::set<int32_t> heldKeys;

    friend class Window;

private:
    void preProcessing(const Window &)
    {
        unicodePoint = 0;
        repeatKey = 0;

        pressedKeys.clear();
        releasedKeys.clear();
    }
    void postProcessing(const Window &window)
    {
    }
};
} // namespace gdf