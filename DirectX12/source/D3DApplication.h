#pragma once

#include "pch.h"
#include "Win32Application.h"

class D3DApplication
{
public:
    D3DApplication(uint32 width, uint32 height, const std::wstring& name)
    {
        m_Width = width;
        m_Height = height;
    }

    virtual void OnInit() {};
    virtual void OnUpdate() {};
    virtual void OnRender() {};
    virtual void OnDestroy() {};

    uint32 GetWidth() const { return m_Width; }
    uint32 GetHeight() const { return m_Height; }

protected:
    uint32 m_Width;
    uint32 m_Height;
    float m_AspectRatio;
};