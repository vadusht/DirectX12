#pragma once
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <wrl.h>

#include "Log.h"
#include "Core.h"

using Microsoft::WRL::ComPtr;
