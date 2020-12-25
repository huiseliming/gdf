#pragma once
#include "Log/LogCategory.h"
#include <string>
namespace gdf
{
#ifdef GDF_DEBUG
DECLARE_LOG_CATEGORY(gdfLog, LogLevel::All, LogLevel::Debug)
#else
DECLARE_LOG_CATEGORY(gdfLog, LogLevel::All, LogLevel::Info)
#endif

// call in main thread
GDF_EXPORT void Initialize();
// call in main thread
GDF_EXPORT void Cleanup();

GDF_EXPORT bool GitRetrievedState();
GDF_EXPORT bool GitIsDirty();
GDF_EXPORT std::wstring GitHeadSHA1();
GDF_EXPORT std::wstring GitAuthorName();
GDF_EXPORT std::wstring GitAuthorEmail();
GDF_EXPORT std::wstring GitCommitDateISO8601();
GDF_EXPORT std::wstring GitCommitDate();
GDF_EXPORT std::wstring GitCommitSubject();
GDF_EXPORT std::wstring GitCommitBody();
GDF_EXPORT std::wstring GitDescribe();
GDF_EXPORT std::wstring GitTag();
GDF_EXPORT std::wstring GitVerison();
GDF_EXPORT std::wstring GitVerisonMajor();
GDF_EXPORT std::wstring GitVerisonMinor();
GDF_EXPORT std::wstring GitVerisonPatch();
} // namespace gdf
