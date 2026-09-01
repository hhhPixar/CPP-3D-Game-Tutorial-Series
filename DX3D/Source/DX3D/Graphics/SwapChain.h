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
// SwapChain.h —— 交换链（双缓冲与呈现）
// -----------------------------------------------------------------------------
// 职责：封装 IDXGISwapChain，管理“前台/后台缓冲”。后台缓冲供 GPU 绘制，
//   前台缓冲交给显示器；present() 把后台内容翻到前台（双缓冲避免画面撕裂）。
// 关键概念——双缓冲：GPU 在后台缓冲画，画完一次性换页到前台，用户看不到中间过程。
//   rtv（渲染目标视图）/ dsv（深度模板视图）：分别是对后台缓冲和深度缓冲的“视图”，
//   供输出合并阶段绑定使用。
// friend class DeviceContext：让 DeviceContext 能直接访问私有 rtv/dsv 来清屏与绑定。
// =============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// SwapChain：交换链，继承 GraphicsResource 以拿到设备/工厂引用与日志。final 禁止再继承。
	class SwapChain final: public GraphicsResource
	{
	public:
		// 构造函数：desc 提供窗口句柄与尺寸，gDesc 提供设备/工厂/日志。
		// 内部创建 IDXGISwapChain 并生成后台缓冲/深度缓冲的视图。
		SwapChain(const SwapChainDesc& desc, const GraphicsResourceDesc& gDesc);
		// 返回交换链尺寸（宽高）。
		Rect getSize() const noexcept;

		// present：把后台缓冲呈现到屏幕。
		//   vsync=true 时与显示器刷新率同步，避免撕裂但降低帧率；false 则尽快翻页。
		void present(bool vsync = false);
	private:
		// reloadBuffers：根据 m_size 重新创建后台缓冲的渲染目标视图与深度模板视图。
		//   窗口尺寸变化后调用以重建这些视图。
		void reloadBuffers();
	private:
		// 底层 DXGI 交换链对象，管理前后台缓冲与翻转机制。
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain{};
		// 渲染目标视图（RenderTargetView）：指向后台缓冲，像素着色器最终写入此处。
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv{};
		// 深度/模板视图（DepthStencilView）：用于深度测试（决定遮挡关系）与模板测试。
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv{};
		// 交换链当前尺寸（像素）。
		Rect m_size{};

		// 友元：允许 DeviceContext 直接访问 m_rtv/m_dsv 等私有成员来清屏与绑定目标。
		friend class DeviceContext;
	};
}

