#include "pch.h"

#include "Log.h"
#include "Win32Application.h"
#include "D3DTest.h"
#include "D3DTriangle.h"

#include "imgui.h"

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
    AllocConsole();
    Log::Init("DirectX 12");
    D3DTriangle app(800, 600, L"Name");
    Win32Application::Run(&app, instance, showCmd, L"Test");
}
    
