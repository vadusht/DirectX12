#pragma once

#include "D3DApplication.h"

class D3DApplication;

class Win32Application
{
    Win32Application() = delete;
public:
    
    static int Run(D3DApplication* app, HINSTANCE instance, int showCmd, const std::wstring& windowName);
    static HWND GetHwnd() { return m_Hwnd; };

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_Hwnd;
};

