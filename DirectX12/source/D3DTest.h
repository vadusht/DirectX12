#pragma once

#include "D3DApplication.h"

class D3DTest : public D3DApplication
{
public:
    D3DTest(uint32 width, uint32 height, const std::wstring& name);

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;

public:
    static const UINT s_SwapChainBufferCount = 2;

    CD3DX12_VIEWPORT m_Viewport;
    CD3DX12_RECT m_ScissorRect;
    ComPtr<IDXGISwapChain3> m_SwapChain;
    ComPtr<ID3D12Device> m_Device;
    ComPtr<ID3D12Resource> m_RenderTargets[s_SwapChainBufferCount];
    
    ComPtr<ID3D12RootSignature> m_RootSignature;
    ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_SrvHeap;
    ComPtr<ID3D12PipelineState> m_PipelineState;
    
    UINT m_RtvDescriptorSize;

    UINT m_FrameIndex;
    HANDLE m_FenceEvent;
    ComPtr<ID3D12Fence> m_Fence;
    UINT64 m_FenceValue;

    float m_ClearColor[4];

    bool InitDirect3D();
    bool LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};

