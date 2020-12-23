#include "Base/Window.h"
#include "Log/Logger.h"
#include "Git.h"

LOG_DEFINE_CATEGORY(General, LogLevel::All, LogLevel::Info)
namespace Runtime
{
void Initialize()
{
    std::wcout << L"Runtime Initialize\n";
    ::glfwInit();
    Logger::Create();
    //LOG(General, LogLevel::Info, "Runtime Initialize\n");
}

void Cleanup()
{
    Logger::Destroy();
    ::glfwTerminate();
    //LOG(General, LogLevel::Info, "Runtime Initialize\n");
    std::wcout << L"Runtime Cleanup\n";
}

bool GitRetrievedState()
{
    return GIT_RETRIEVED_STATE;
}
bool GitIsDirty()
{
    return GIT_IS_DIRTY;
}
std::wstring GitHeadSHA1()
{
    return GIT_HEAD_SHA1;
}
std::wstring GitAuthorName()
{
    return GIT_AUTHOR_NAME;
}
std::wstring GitAuthorEmail()
{
    return GIT_AUTHOR_EMAIL;
}
std::wstring GitCommitDateISO8601()
{
    return GIT_COMMIT_DATE_ISO8601;
}
std::wstring GitCommitDate()
{
    return GitCommitDateISO8601();
}
std::wstring GitCommitSubject()
{
    return GIT_COMMIT_SUBJECT;
}
std::wstring GitCommitBody()
{
    return GIT_COMMIT_BODY;
}
std::wstring GitDescribe()
{
    return GIT_DESCRIBE;
}
std::wstring GitTag()
{
    return GIT_TAG;
}
std::wstring GitVerison()
{
    return GIT_VERSION;
}
std::wstring GitVerisonMajor()
{
    return GIT_VERSION_MAJOR;
}
std::wstring GitVerisonMinor()
{
    return GIT_VERSION_MINOR;
}
std::wstring GitVerisonPatch()
{
    return GIT_VERSION_PATCH;
}




} // namespace Runtime
