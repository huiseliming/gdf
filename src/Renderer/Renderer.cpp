#include "Renderer/Renderer.h"
#include "Renderer/Git.h"

bool Renderer::GitRetrievedState()
{
    return GIT_RETRIEVED_STATE;
}
bool Renderer::GitIsDirty()
{
    return GIT_IS_DIRTY;
}
std::wstring Renderer::GitHeadSHA1()
{
    return GIT_HEAD_SHA1;
}
std::wstring Renderer::GitAuthorName()
{
    return GIT_AUTHOR_NAME;
}
std::wstring Renderer::GitAuthorEmail()
{
    return GIT_AUTHOR_EMAIL;
}
std::wstring Renderer::GitCommitDataISO8601()
{
    return GIT_COMMIT_DATE_ISO8601;
}
std::wstring Renderer::GitCommitData()
{
    return GitCommitDataISO8601();
}
std::wstring Renderer::GitCommitSubject()
{
    return GIT_COMMIT_SUBJECT;
}
std::wstring Renderer::GitCommitBody()
{
    return GIT_COMMIT_BODY;
}
std::wstring Renderer::GitDescribe()
{
    return GIT_DESCRIBE;
}
std::wstring Renderer::GitTag()
{
    return GIT_TAG;
}
std::wstring Renderer::GitVerison()
{
    return GIT_VERSION;
}
std::wstring Renderer::GitVerisonMajor()
{
    return GIT_VERSION_MAJOR;
}
std::wstring Renderer::GitVerisonMinor()
{
    return GIT_VERSION_MINOR;
}
std::wstring Renderer::GitVerisonPatch()
{
    return GIT_VERSION_PATCH;
}