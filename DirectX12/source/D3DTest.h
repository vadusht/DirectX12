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

private:
    static const UINT s_SwapChainBufferCount = 2;

    ComPtr<IDXGISwapChain3> m_SwapChain;
    ComPtr<ID3D12Device> m_Device;
    ComPtr<ID3D12Resource> m_RenderTargets[s_SwapChainBufferCount];
    ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
    ComPtr<ID3D12CommandQueue> m_CommandQueue;
    ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    ComPtr<ID3D12PipelineState> m_PipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    UINT m_RtvDescriptorSize;

    UINT m_FrameIndex;
    HANDLE m_FenceEvent;
    ComPtr<ID3D12Fence> m_Fence;
    UINT64 m_FenceValue;

    bool InitDirect3D();
    bool LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};

