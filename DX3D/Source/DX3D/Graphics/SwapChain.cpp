/*MIT License

C++ 3D Game Tutorial Series (https://github.com/PardCode/CPP-3D-Game-Tutorial-Series)

Copyright (c) 2019-2026, PardCode

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

// =============================================================================
// SwapChain.cpp —— 交换链实现
// -----------------------------------------------------------------------------
// 本文件实现：交换链创建、尺寸查询、present 呈现、reloadBuffers 重建缓冲视图。
// 双缓冲机制：BufferCount=2，绘制在后台缓冲，present 翻到前台；FLIP_DISCARD
//   模式翻转后丢弃后台旧内容（高效）。
// =============================================================================
#include <DX3D/Graphics/SwapChain.h>

// 构造函数：用窗口句柄与尺寸创建交换链，并立即生成后台缓冲/深度缓冲的视图。
//   委托构造 GraphicsResource(gDesc) 把设备/工厂/日志存好，并保存尺寸 m_size。
dx3d::SwapChain::SwapChain(const SwapChainDesc& desc, const GraphicsResourceDesc& gDesc) : 
	GraphicsResource(gDesc), m_size(desc.winSize)
{
	// 必须提供有效窗口句柄，否则无法创建交换链。
	if (!desc.winHandle) DX3DLogThrowInvalidArg("No window handle provided.");

	// DXGI_SWAP_CHAIN_DESC：描述交换链参数（缓冲尺寸/格式/数量/用法/翻转模式等）。
	DXGI_SWAP_CHAIN_DESC dxgiDesc{};

	// 后台缓冲宽高，至少为 1（防止 0 尺寸导致创建失败）。
	dxgiDesc.BufferDesc.Width = std::max(1, desc.winSize.width);
	dxgiDesc.BufferDesc.Height = std::max(1, desc.winSize.height);
	// 像素格式：R8G8B8A8_UNORM，每通道 8 位无符号归一化（0..1），最常用。
	dxgiDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// 缓冲数量：2 实现双缓冲（前台 + 后台）。
	dxgiDesc.BufferCount = 2;
	// 用法：作为渲染目标输出，说明该缓冲供像素着色器写入。
	dxgiDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 输出窗口句柄（来自 Win32 HWND）。
	dxgiDesc.OutputWindow = static_cast<HWND>(desc.winHandle);
	// 多采样：Count=1 表示不使用 MSAA（多重采样抗锯齿）。
	dxgiDesc.SampleDesc.Count = 1;
	// 翻转效果：FLIP_DISCARD，翻转后丢弃后台旧内容，性能好。
	dxgiDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// 窗口模式（TRUE=窗口化，而非全屏独占）。
	dxgiDesc.Windowed = TRUE;

	// 用 DXGI 工厂创建交换链。m_device 是 D3D 设备引用，交换链内部会用到它做缓冲。
	DX3DGraphicsLogThrowOnFail(m_factory.CreateSwapChain(&m_device, &dxgiDesc, &m_swapChain),
		"CreateSwapChain failed.");

	// 创建后台缓冲的渲染目标视图与深度模板视图。
	reloadBuffers();
}

// 返回交换链尺寸。
dx3d::Rect dx3d::SwapChain::getSize() const noexcept
{
	return m_size;
}

// present：把后台缓冲内容翻到前台显示。vsync=true 与显示器刷新同步。
void dx3d::SwapChain::present(bool vsync)
{
	// Present：翻转缓冲。vsync 非零表示等待垂直同步（避免撕裂）；0 则立即翻页。
	auto hr = m_swapChain->Present(vsync, 0);
	if (FAILED(hr))
	{
		// 呈现失败仅记录错误，不抛异常。
		DX3DLogError("Present failed.");
		return;
	}
}

// reloadBuffers：创建/重建后台缓冲的渲染目标视图(RTV)与深度模板视图(DSV)。
//   在构造时调用；窗口尺寸变化后也应调用以匹配新尺寸。
void dx3d::SwapChain::reloadBuffers()
{
	// 取索引 0 的缓冲（后台缓冲）作为 2D 纹理。
	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer{};
	DX3DGraphicsLogThrowOnFail(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&buffer)),
		"GetBuffer failed.");
	// 基于该纹理创建渲染目标视图，供输出合并阶段作为像素写入目标。nullptr 表示使用默认描述。
	DX3DGraphicsLogThrowOnFail(m_device.CreateRenderTargetView(buffer.Get(), nullptr, &m_rtv),
		"CreateRenderTargetView failed.");

	// 深度缓冲纹理的描述：尺寸与后台缓冲一致。
	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Width = std::max(1, m_size.width);
	depthTexDesc.Height = std::max(1, m_size.height);
	// 格式：D24_UNORM_S8_UINT —— 24 位深度 + 8 位模板，标准深度缓冲格式。
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 绑定标志：作为深度/模板资源使用。
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.ArraySize = 1;

	// 创建深度缓冲纹理（nullptr 表示不提供初始数据，内容未定义）。
	DX3DGraphicsLogThrowOnFail(m_device.CreateTexture2D(&depthTexDesc, nullptr, &buffer),
		"CreateTexture2D failed.");
	// 基于该纹理创建深度/模板视图，供深度测试与模板测试使用。NULL 表示使用默认描述。
	DX3DGraphicsLogThrowOnFail(m_device.CreateDepthStencilView(buffer.Get(), NULL, &m_dsv),
		"CreateDepthStencilView failed.");
}

