#pragma once

#include <stdint.h>
#include <comdef.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

inline std::string WStringToAnsi(const std::wstring& source)
{
    CHAR buffer[512];
    WideCharToMultiByte(CP_ACP, 0, &source[0], -1, buffer, 512, NULL, NULL);
    return std::string(buffer);
}

#define CHECK_HRESULT(x)                                                                                    \
{                                                                                                           \
    HRESULT result = (x);                                                                                   \
    _com_error error(result);                                                                               \
    std::wstring message = error.ErrorMessage();                                                            \
    if(FAILED(result))                                                                                      \
    {                                                                                                       \
        DX_CRITICAL("{} failed in {}; line {}; error: {}", #x, __FILE__, __LINE__, WStringToAnsi(message)); \
    }                                                                                                       \
}

