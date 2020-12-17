#pragma once
#include <memory>
#include <mutex>

template<typename T>
class Singleton
{
private:
    Singleton<T>(const Singleton<T>&) = delete;
    Singleton<T>& operator = (const Singleton<T>&) = delete;
    Singleton<T>(Singleton<T>&&) = delete;
    Singleton<T>& operator = (Singleton<T>&&) = delete;

    Singleton<T>() = default;

public:
    virtual ~Singleton<T>() = default;

    static T& Instance()
    {
        return *sm_instance;
    }

    template<typename ...Args>
    static void Create(Args&& ...args)
    {
        sm_instance.reset(new T(std::forward<Args>(args)...));
    }

    static void Destroy()
    {
        sm_instance.reset();
    }

private: 
    static std::unique_ptr<T> sm_instance;

};

template<typename T>
std::unique_ptr<T> Singleton<T>::sm_instance;