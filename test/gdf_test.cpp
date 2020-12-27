#define CATCH_CONFIG_RUNNER
#include "Log//LogCategory.h"
#include "Log//LogLevel.h"
#include "Log/Logger.h"
#include "gdf.h"
#include <catch2/catch.hpp>

using namespace gdf;

GDF_DECLARE_LOG_CATEGORY(TestCategory, LogLevel::All, LogLevel::All)
GDF_DEFINE_LOG_CATEGORY(TestCategory)

TEST_CASE("LogCategory - Define", "[gdf][LogCategory]")
{
    auto &testCategory = TestCategory::instance();
    REQUIRE(testCategory.displayName_ == "TestCategory");
    REQUIRE(testCategory.runtimeLevel_ == LogLevel::All);
    testCategory.runtimeLevel_ = LogLevel::Error;
    REQUIRE(testCategory.runtimeLevel_ == LogLevel::Error);
}

class TestSink : public LogSink
{
public:
    virtual void Log(const LogCategory &category,
                     const LogLevel level,
                     const std::string_view message)
    {
        lastLog_ =
            fmt::format("[{:s}][{:s}] {:s}", category.displayName_, std::to_string(level), message);
    }
    void Exception()
    {
    }

    std::string &lastLog()
    {
        return lastLog_;
    }

private:
    std::string lastLog_;
};

GDF_DECLARE_LOG_CATEGORY(FatalCategory, LogLevel::All, LogLevel::Fatal)
GDF_DEFINE_LOG_CATEGORY(FatalCategory)

TEST_CASE("LogCategory - Fatal compilation level", "[gdf][LogCategory]")
{
    TestSink testSink;
    gdf::Logger::instance().RegisterSink(&testSink);
    GDF_LOG(FatalCategory, LogLevel::Fatal, "Fatal");
    GDF_LOG(FatalCategory, LogLevel::Error, "Error");
    GDF_LOG(FatalCategory, LogLevel::Warning, "Warning");
    GDF_LOG(FatalCategory, LogLevel::Info, "Info");
    GDF_LOG(FatalCategory, LogLevel::Verbose, "Verbose");
    gdf::Logger::instance().DeregisterSink(&testSink);
    REQUIRE(testSink.lastLog() == "[FatalCategory][Fatal] Fatal");
}
GDF_DECLARE_LOG_CATEGORY(ErrorCategory, LogLevel::All, LogLevel::Error)
GDF_DEFINE_LOG_CATEGORY(ErrorCategory)

TEST_CASE("LogCategory - Error compilation level", "[gdf][LogCategory]")
{
    TestSink testSink;
    gdf::Logger::instance().RegisterSink(&testSink);
    GDF_LOG(ErrorCategory, LogLevel::Fatal, "Fatal");
    GDF_LOG(ErrorCategory, LogLevel::Error, "Error");
    GDF_LOG(ErrorCategory, LogLevel::Warning, "Warning");
    GDF_LOG(ErrorCategory, LogLevel::Info, "Info");
    GDF_LOG(ErrorCategory, LogLevel::Verbose, "Verbose");
    gdf::Logger::instance().DeregisterSink(&testSink);
    REQUIRE(testSink.lastLog() == "[ErrorCategory][Error] Error");
}
GDF_DECLARE_LOG_CATEGORY(WarningCategory, LogLevel::All, LogLevel::Warning)
GDF_DEFINE_LOG_CATEGORY(WarningCategory)

TEST_CASE("LogCategory - Warning compilation level", "[gdf][LogCategory]")
{
    TestSink testSink;
    gdf::Logger::instance().RegisterSink(&testSink);
    GDF_LOG(WarningCategory, LogLevel::Fatal, "Fatal");
    GDF_LOG(WarningCategory, LogLevel::Error, "Error");
    GDF_LOG(WarningCategory, LogLevel::Warning, "Warning");
    GDF_LOG(WarningCategory, LogLevel::Info, "Info");
    GDF_LOG(WarningCategory, LogLevel::Verbose, "Verbose");
    gdf::Logger::instance().DeregisterSink(&testSink);
    REQUIRE(testSink.lastLog() == "[WarningCategory][Warning] Warning");
}
GDF_DECLARE_LOG_CATEGORY(InfoCategory, LogLevel::All, LogLevel::Info)
GDF_DEFINE_LOG_CATEGORY(InfoCategory)

TEST_CASE("LogCategory - Info compilation level", "[gdf][LogCategory]")
{
    TestSink testSink;
    gdf::Logger::instance().RegisterSink(&testSink);
    GDF_LOG(InfoCategory, LogLevel::Fatal, "Fatal");
    GDF_LOG(InfoCategory, LogLevel::Error, "Error");
    GDF_LOG(InfoCategory, LogLevel::Warning, "Warning");
    GDF_LOG(InfoCategory, LogLevel::Info, "Info");
    GDF_LOG(InfoCategory, LogLevel::Verbose, "Verbose");
    gdf::Logger::instance().DeregisterSink(&testSink);
    REQUIRE(testSink.lastLog() == "[InfoCategory][Info] Info");
}
GDF_DECLARE_LOG_CATEGORY(VerboseCategory, LogLevel::All, LogLevel::Verbose)
GDF_DEFINE_LOG_CATEGORY(VerboseCategory)

TEST_CASE("LogCategory - Verbose compilation level", "[gdf][LogCategory]")
{
    TestSink testSink;
    gdf::Logger::instance().RegisterSink(&testSink);
    GDF_LOG(VerboseCategory, LogLevel::Fatal, "Fatal");
    GDF_LOG(VerboseCategory, LogLevel::Error, "Error");
    GDF_LOG(VerboseCategory, LogLevel::Warning, "Warning");
    GDF_LOG(VerboseCategory, LogLevel::Info, "Info");
    GDF_LOG(VerboseCategory, LogLevel::Verbose, "Verbose");
    gdf::Logger::instance().DeregisterSink(&testSink);
    REQUIRE(testSink.lastLog() == "[VerboseCategory][Verbose] Verbose");
}

GDF_DECLARE_LOG_CATEGORY(RuntimeCategory, LogLevel::All, LogLevel::All)
GDF_DEFINE_LOG_CATEGORY(RuntimeCategory)

TEST_CASE("LogCategory - Runtime log levels", "[gdf][LogCategory]")
{
    TestSink testSink;
    gdf::Logger::instance().RegisterSink(&testSink);
    SECTION("Verbose")
    {
        RuntimeCategory::instance().runtimeLevel_ = LogLevel::Verbose;
        GDF_LOG(RuntimeCategory, LogLevel::Fatal, "Fatal");
        GDF_LOG(RuntimeCategory, LogLevel::Error, "Error");
        GDF_LOG(RuntimeCategory, LogLevel::Warning, "Warning");
        GDF_LOG(RuntimeCategory, LogLevel::Info, "Info");
        GDF_LOG(RuntimeCategory, LogLevel::Verbose, "Verbose");
        REQUIRE(testSink.lastLog() == "[RuntimeCategory][Verbose] Verbose");
    }
    SECTION("Info")
    {
        RuntimeCategory::instance().runtimeLevel_ = LogLevel::Info;
        GDF_LOG(RuntimeCategory, LogLevel::Fatal, "Fatal");
        GDF_LOG(RuntimeCategory, LogLevel::Error, "Error");
        GDF_LOG(RuntimeCategory, LogLevel::Warning, "Warning");
        GDF_LOG(RuntimeCategory, LogLevel::Info, "Info");
        GDF_LOG(RuntimeCategory, LogLevel::Verbose, "Verbose");
        REQUIRE(testSink.lastLog() == "[RuntimeCategory][Info] Info");
    }
    SECTION("Warning")
    {
        RuntimeCategory::instance().runtimeLevel_ = LogLevel::Warning;
        GDF_LOG(RuntimeCategory, LogLevel::Fatal, "Fatal");
        GDF_LOG(RuntimeCategory, LogLevel::Error, "Error");
        GDF_LOG(RuntimeCategory, LogLevel::Warning, "Warning");
        GDF_LOG(RuntimeCategory, LogLevel::Info, "Info");
        GDF_LOG(RuntimeCategory, LogLevel::Verbose, "Verbose");
        REQUIRE(testSink.lastLog() == "[RuntimeCategory][Warning] Warning");
    }
    SECTION("Error")
    {
        RuntimeCategory::instance().runtimeLevel_ = LogLevel::Error;
        GDF_LOG(RuntimeCategory, LogLevel::Fatal, "Fatal");
        GDF_LOG(RuntimeCategory, LogLevel::Error, "Error");
        GDF_LOG(RuntimeCategory, LogLevel::Warning, "Warning");
        GDF_LOG(RuntimeCategory, LogLevel::Info, "Info");
        GDF_LOG(RuntimeCategory, LogLevel::Verbose, "Verbose");
        REQUIRE(testSink.lastLog() == "[RuntimeCategory][Error] Error");
    }
    SECTION("Fatal")
    {
        RuntimeCategory::instance().runtimeLevel_ = LogLevel::Fatal;
        GDF_LOG(RuntimeCategory, LogLevel::Fatal, "Fatal");
        GDF_LOG(RuntimeCategory, LogLevel::Error, "Error");
        GDF_LOG(RuntimeCategory, LogLevel::Warning, "Warning");
        GDF_LOG(RuntimeCategory, LogLevel::Info, "Info");
        GDF_LOG(RuntimeCategory, LogLevel::Verbose, "Verbose");
        REQUIRE(testSink.lastLog() == "[RuntimeCategory][Fatal] Fatal");
    }
    gdf::Logger::instance().DeregisterSink(&testSink);
}

int main(int argc, char *argv[])
{
    gdf::Initialize();
    int result = Catch::Session().run(argc, argv);
    gdf::Cleanup();
    return result;
}