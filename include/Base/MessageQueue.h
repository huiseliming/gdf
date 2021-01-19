#pragma once
#include "Base/Common.h"
#include <functional>
#include <mutex>
#include <vector>

namespace gdf
{

class MessageCollector
{
    MessageCollector(size_t maxNum = 1024) : maxNum_(maxNum), frontOffset_(0)
    {
        msgs_.resize(maxNum_ * 3);
    }

    size_t AddMessage(std::string_view msg)
    {
        std::scoped_lock<std::mutex> lock(sync);
        taskQueue.emplace_back([&, msg] {
            msgs_[frontOffset_] = msg;
            frontOffset_++;
            assert(frontOffset_ <= maxNum_);
            if (frontOffset_ >= maxNum_) {
                frontOffset_ = 0;
            }
        });
    }

    void Update()
    {
        std::vector<std::function<void()>> tmp;
        {
            std::scoped_lock<std::mutex> lock(sync);
            tmp.swap(taskQueue);
        }
        for (auto &task : tmp) {
            task();
        }
    }

    std::string_view At(size_t fromLastOffset)
    {
        return msgs_[frontOffset_ - 1 - fromLastOffset < 0 ? frontOffset_ - 1 - fromLastOffset < 0 + maxNum_
                                                           : frontOffset_ - 1 - fromLastOffset < 0];
    }

    std::vector<std::string_view> &msgs()
    {
        return msgs_;
    }

    size_t frontOffset()
    {
        return frontOffset_;
    }

public:
    std::mutex sync;
    std::vector<std::function<void()>> taskQueue;

    size_t frontOffset_;
    std::vector<std::string_view> msgs_;
    size_t maxNum_;
};

} // namespace gdf