#pragma once
#include "gdf_export.h"
#include <exception>
#include <sstream>
#include <string>

#define EXCEPT(...) Exception(__LINE__, __FILE__, ##__VA_ARGS__)
#define THROW_EXCEPT(...) throw EXCEPT(__VA_ARGS__)
namespace gdf
{
class GDF_EXPORT Exception : public std::exception
{
public:
    Exception(int line, const char *file) noexcept : line_(line), file_(file)
    {
    }
    explicit Exception(int line, const char *file, char const *const message) noexcept
        : line_(line), file_(file), message_(message)
    {
    }
    const char *what() const noexcept override
    {
        std::ostringstream oss;
        oss << "[Type       ] " << GetType() << "\n"
            << "[Description] " << message_ << "\n"
            << GetOriginString();
        whatBuffer_ = oss.str();
        return whatBuffer_.c_str();
    }

    virtual const char *GetType() const noexcept
    {
        return "Exception";
    }

    std::string GetOriginString() const noexcept
    {
        std::ostringstream oss;
        oss << "[File] " << file_ << "\n[Line] " << line_;
        return oss.str();
    }

private:
    int line_;
    std::string file_;
    std::string message_;

protected:
    mutable std::string whatBuffer_;
    ;
};
} // namespace gdf