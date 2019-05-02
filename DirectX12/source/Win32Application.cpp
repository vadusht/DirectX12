#include "pch.h"
#include "Win32Application.h"
#include "D3DTest.h"
#include "D3DTriangle.h"
#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx12.h"

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();

    D3DTest* appTest = dynamic_cast<D3DTest*>(app);

    ImGui_ImplWin32_Init(m_Hwnd);
    ImGui_ImplDX12_Init(appTest->m_Device.Get(), 3,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        appTest->m_SrvHeap->GetCPUDescriptorHandleForHeapStart(),
        appTest->m_SrvHeap->GetGPUDescriptorHandleForHeapStart());
    bool show_demo_window = false;
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
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            
            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            

            appTest->OnUpdate();
            appTest->OnRender();

           
        }
    }


    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    app->OnDestroy();

    return static_cast<int>(msg.wParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
