#include "pch.h"
#include "D3DTriangle.h"

#include "imgui.h"
#include "examples/imgui_impl_dx12.h"
D3DTriangle::D3DTriangle(uint32 width, uint32 height, const std::wstring& name)
    : D3DApplication(width, height, name),
    m_FrameIndex(0),
    m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_RtvDescriptorSize(0)
{
}


void D3DTriangle::OnInit()
{
    if (!InitDirect3D())
    {

    }

    if (!LoadAssets())
    {

    }
}


bool D3DTriangle::InitDirect3D()
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

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    result = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
    if (FAILED(result))
    {
        DX_ERROR("CreateCommandQueue");
        return false;
    }

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

    result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
    if (FAILED(result))
    {
        DX_ERROR("CreateCommandAllocator");
        return false;
    }

    return true;
}

bool D3DTriangle::LoadAssets()
{
    HRESULT result = {};

    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(result))
        {
            DX_ERROR("D3D12SerializeRootSignature");
            return false;
        }

        result = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            DX_ERROR("CreateRootSignature");
            return false;
        }
    }

    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        result = D3DCompileFromFile(L"C:\\Users\\Vadym\\Desktop\\DirectX12\\DirectX12\\source\\VS.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
        if(FAILED(result))
        {
            DX_ERROR("Vertex Shader");
            return false;
        }
        result = D3DCompileFromFile(L"C:\\Users\\Vadym\\Desktop\\DirectX12\\DirectX12\\source\\PS.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);
        if (FAILED(result))
        {
            DX_ERROR("Pixel Shader");
            return false;
        }

        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_RootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        result = m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
        if (FAILED(result))
        {
            DX_ERROR("CreateGraphicsPipelineState");
            return false;
        }
    }



    m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_PipelineState.Get(), IID_PPV_ARGS(&m_CommandList));
    if (FAILED(result))
    {
        DX_ERROR("CreateCommandList");
        return false;
    }

    result = m_CommandList->Close();
    if (FAILED(result))
    {
        DX_ERROR("Close");
        return false;
    }

    {
        Vertex triangleVertices[] =
        {
               { { -0.5f,  -0.5f, 0.0f },  { 1.0f, 0.0f, 0.0f, 1.0f } },
               { { -0.5f,   0.5f, 0.0f },  { 0.0f, 1.0f, 0.0f, 1.0f } },
               { {  0.5f,   0.5f, 0.0f },  { 0.0f, 0.0f, 1.0f, 1.0f } },
          /*     { { -0.5f,  -0.5f, 0.0f },  { 1.0f, 0.0f, 0.0f, 1.0f } },
               { {  0.5f,   0.5f, 0.0f },  { 0.0f, 0.0f, 1.0f, 1.0f } },*/
               { {  0.5f,  -0.5f, 0.0f },  { 0.5f, 0.5f, 0.5f, 1.0f } }
        };
        
        unsigned int triangleIndices[] =
        {
            0, 1, 2,
            0, 2, 3
        };

        const UINT indexBufferSize = sizeof(triangleIndices);

        const UINT vertexBufferSize = sizeof(triangleVertices);

        result = m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_IndexBuffer));

        if (FAILED(result))
        {
            DX_ERROR("CreateComittedResource Index");
            return false;
        }


        result = m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_VertexBuffer));

        if (FAILED(result))
        {
            DX_ERROR("CreateComittedResource Pos");
            return false;
        }

        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);
        result = m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
        if (FAILED(result))
        {
            DX_ERROR("Map");
            return false;
        }
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_VertexBuffer->Unmap(0, nullptr);


        result = m_IndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
        if (FAILED(result))
        {
            DX_ERROR("Map");
            return false;
        }
        memcpy(pVertexDataBegin, triangleIndices, sizeof(triangleIndices));
        m_IndexBuffer->Unmap(0, nullptr);

        m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
        m_IndexBufferView.SizeInBytes = indexBufferSize;
        m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

        m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
        m_VertexBufferView.StrideInBytes = sizeof(Vertex);
        m_VertexBufferView.SizeInBytes = vertexBufferSize;
    }


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


void D3DTriangle::OnUpdate()
{

};
void D3DTriangle::OnRender()
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

void D3DTriangle::PopulateCommandList()
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

    m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RtvDescriptorSize);
    m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);

   

    const float clearColor[] = { m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3] };
    m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    
    m_CommandList->IASetIndexBuffer(&m_IndexBufferView);
    m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    //m_CommandList->DrawInstanced(6, 1, 0, 0);
   

    ID3D12DescriptorHeap* ppHeaps[] = { m_SrvHeap.Get() };
    m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList.Get());

    


    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    result = m_CommandList->Close();

    if (FAILED(result))
    {
        DX_ERROR("Close");
    }
}

void D3DTriangle::WaitForPreviousFrame()
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

void D3DTriangle::OnDestroy()
{
    WaitForPreviousFrame();

    CloseHandle(m_FenceEvent);
}