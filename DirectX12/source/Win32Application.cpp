#include "pch.h"
#include "Win32Application.h"


HWND Win32Application::m_Hwnd = nullptr;

int Win32Application::Run(D3DApplication* app, HINSTANCE instance, int showCmd, const std::wstring& windowName)
{
    WNDCLASSEX windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"D3DApplicationClass";
    RegisterClassEx(&windowClass);
   
    RECT windowRect = { 0, 0, static_cast<LONG>(app->GetWidth()), static_cast<LONG>(app->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    m_Hwnd = CreateWindow(
        windowClass.lpszClassName,
        windowName.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        instance,
        nullptr
    );
    

    AllocConsole();
    app->OnInit();

    ShowWindow(m_Hwnd, showCmd);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            app->OnUpdate();
            app->OnRender();
        }
    }

    app->OnDestroy();

    return static_cast<int>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //D3DApplication* pSample = reinterpret_cast<D3DApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
