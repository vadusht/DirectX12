#include "pch.h"

#include "Log.h"

int main()
{
    Log::Init("DirectX 12");
    DX_TRACE("TRACE");
    DX_INFO("INFO");
    DX_WARN("WARN");
    DX_ERROR("ERROR");
    DX_CRITICAL("CRITICAL");
}