#pragma once

class NonCopyable
{
private:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;

protected:
    NonCopyable() = default;
    virtual ~NonCopyable() = default;
};
