#pragma once
#include "Log/LogCategory.h"
#include <string>
namespace gdf
{
#ifdef GDF_DEBUG
GDF_DECLARE_LOG_CATEGORY(gdfLog, LogLevel::All, LogLevel::Debug)
#else
GDF_DECLARE_LOG_CATEGORY(gdfLog, LogLevel::All, LogLevel::Info)
#endif

// last error

// call in main thread
void Initialize();
// call in main thread
void Cleanup();

bool GitRetrievedState();
bool GitIsDirty();
std::wstring GitHeadSHA1();
std::wstring GitAuthorName();
std::wstring GitAuthorEmail();
std::wstring GitCommitDateISO8601();
std::wstring GitCommitDate();
std::wstring GitCommitSubject();
std::wstring GitCommitBody();
std::wstring GitDescribe();
std::wstring GitTag();
std::wstring GitVerison();
std::wstring GitVerisonMajor();
std::wstring GitVerisonMinor();
std::wstring GitVerisonPatch();
} // namespace gdf
