#pragma once
#include <memory>
#include <mutex>

template <typename T>
class Singleton
{
private:
    Singleton<T>(const Singleton<T> &) = delete;
    Singleton<T> &operator=(const Singleton<T> &) = delete;
    Singleton<T>(Singleton<T> &&) = delete;
    Singleton<T> &operator=(Singleton<T> &&) = delete;

    Singleton<T>() = default;

public:
    virtual ~Singleton<T>() = default;

    static T &Instance()
    {
        return *pInstance_;
    }

    template <typename... Args>
    static void Create(Args &&...args)
    {
        pInstance_.reset(new T(std::forward<Args>(args)...));
    }

    static void Destroy()
    {
        pInstance_.reset();
    }

private:
    static std::unique_ptr<T> pInstance_;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::pInstance_;
