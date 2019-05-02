#include "pch.h"
#include "D3DTest.h"

#include "imgui.h"
#include "examples/imgui_impl_dx12.h"
D3DTest::D3DTest(uint32 width, uint32 height, const std::wstring& name)
    : D3DApplication(width, height, name),
    m_FrameIndex(0),
    m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_RtvDescriptorSize(0)
{
}


void D3DTest::OnInit()
{
    if (!InitDirect3D())
    {

    }

    if (!LoadAssets())
    {

    }
}


bool D3DTest::InitDirect3D()
{
    UINT dxgiFactoryFlags = 0;
    HRESULT result = {};


#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    if (SUCCEEDED(result))
    {
        debugController->EnableDebugLayer();

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
    else
    {
        DX_ERROR("D3D12GetDebugInterface error");
    }
#endif
    ComPtr<IDXGIFactory4> factory;

    result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    if (FAILED(result))
    {
        DX_ERROR("CreateDXGIFactory2 error");
        return false;
    }

    result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
    if (FAILED(result))
    {
        DX_WARN("D3D12Create Device");

        ComPtr<IDXGIAdapter> warpAdapter;
        result = factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

        if (FAILED(result))
        {
            DX_ERROR("EnumWarpAdapter");
            return false;
        }
        else
        {
            result = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));\

            if (FAILED(result))
            {
                DX_ERROR("D3D12CreateDevice");
                return false;
            }
        }
    }


    CreateCommandObjects(m_Device.Get());

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = s_SwapChainBufferCount;
    swapChainDesc.Width = m_Width;
    swapChainDesc.Height = m_Height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    result = factory->CreateSwapChainForHwnd(
        m_CommandQueue.Get(),
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain);
    if (FAILED(result))
    {
        DX_ERROR("CreateSwapChainForHwnd");
        return false;
    }

    result = factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(result))
    {
        DX_WARN("MakeWindowAssociation");
    }

    result = swapChain.As(&m_SwapChain);
    if (FAILED(result))
    {
        DX_ERROR("As");
        return false;
    }
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = s_SwapChainBufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        result = m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap));
        if (FAILED(result))
        {
            DX_ERROR("CreateDescriptorHeap");
            return false;
        }

        m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_SrvHeap)) != S_OK)
            return false;
    }

    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT n = 0; n < s_SwapChainBufferCount; n++)
        {
            result = m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n]));
            if (FAILED(result))
            {
                DX_ERROR("GetBuffer");
                return false;
            }
            m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RtvDescriptorSize);
        }
    }


    return true;
}

bool D3DTest::LoadAssets()
{
    HRESULT result = {};



    {
        result = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
        if (FAILED(result))
        {
            DX_ERROR("CreateFence");
            return false;
        }
        m_FenceValue = 1;

        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            DX_ERROR("CreateEvent");
            return false;
        }
    }

    return true;
}


void D3DTest::OnUpdate()
{

};
void D3DTest::OnRender()
{
    HRESULT result = {};

    ImGui::ColorEdit4("Clear Color", m_ClearColor);

    PopulateCommandList();

    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    result = m_SwapChain->Present(1, 0);

    if (FAILED(result))
    {
        DX_ERROR("Present");
    }

    WaitForPreviousFrame();
}

void D3DTest::PopulateCommandList()
{
    HRESULT result = {};

    result = m_CommandAllocator->Reset();

    if (FAILED(result))
    {
        DX_ERROR("Rest");
    }

    result = m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get());
    if (FAILED(result))
    {
        DX_ERROR("Reset");
    }

    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RtvDescriptorSize);


    ID3D12DescriptorHeap* ppHeaps[] = { m_SrvHeap.Get() };
    m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
  

    const float clearColor[] = { m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3] };
    m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList.Get());

    


    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    result = m_CommandList->Close();

    if (FAILED(result))
    {
        DX_ERROR("Close");
    }
}

void D3DTest::WaitForPreviousFrame()
{
    HRESULT result = {};

    const UINT64 fence = m_FenceValue;
    result = m_CommandQueue->Signal(m_Fence.Get(), fence);

    if (FAILED(result))
    {
        DX_ERROR("Signal");
    }
    m_FenceValue++;

    if (m_Fence->GetCompletedValue() < fence)
    {
        result = m_Fence->SetEventOnCompletion(fence, m_FenceEvent);
        if (FAILED(result))
        {
            DX_ERROR("SetEventOnCompletion");
        }
        else
        {
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
    }

    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void D3DTest::OnDestroy()
{
    WaitForPreviousFrame();

    CloseHandle(m_FenceEvent);
}