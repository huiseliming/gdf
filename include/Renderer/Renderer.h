#pragma once
#include <string>
#include "Singleton.h"

class Renderer : public Singleton<Renderer>
{
public:
    
private:

public:
    static bool GitRetrievedState();
    static bool GitIsDirty();
    static std::wstring GitHeadSHA1();
    static std::wstring GitAuthorName();
    static std::wstring GitAuthorEmail();
    static std::wstring GitCommitDataISO8601();
    static std::wstring GitCommitData();
    static std::wstring GitCommitSubject();
    static std::wstring GitCommitBody();
    static std::wstring GitDescribe();
    static std::wstring GitTag();
    static std::wstring GitVerison();
    static std::wstring GitVerisonMajor();
    static std::wstring GitVerisonMinor();
    static std::wstring GitVerisonPatch();
};
