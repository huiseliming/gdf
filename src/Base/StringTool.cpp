#include "Base/StringTool.h"


namespace String{

    std::wstring ConvertWstring(std::string in)
    {
        mbstate_t state = {};
        const char* mbstr = in.c_str();
        size_t len = 1 + mbsrtowcs(NULL, &mbstr, 0, &state);
        std::wstring wstr(len,0);
        mbsrtowcs(&wstr[0], &mbstr, len, &state);
        wstr.resize(len - 1);
        return wstr;
    }
    
    std::string ConvertString(std::wstring in)
    {
        mbstate_t state = {};
        const wchar_t* wstr = in.c_str();
        size_t len = 1 + wcsrtombs(NULL, &wstr, 0, &state);
        std::string mbstr(len,0);
        wcsrtombs(&mbstr[0], &wstr, len, &state);
        mbstr.resize(len - 1);
        return mbstr;
    }

}