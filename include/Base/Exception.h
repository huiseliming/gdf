#pragma once
#include <exception>
#include <sstream>
#include <string>

#define EXCEPT(...) Exception(__LINE__, __FILE__, ##__VA_ARGS__)
#define THROW_EXCEPT(...) throw EXCEPT(__VA_ARGS__)

class Exception : public std::exception
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
        oss << "[Type       ] " << GetType() << std::endl
            << "[Description] " << message_ << std::endl
            << GetOriginString() << std::endl;
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
        oss << "[File] " << file_ << std::endl << "[Line] " << line_;
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
