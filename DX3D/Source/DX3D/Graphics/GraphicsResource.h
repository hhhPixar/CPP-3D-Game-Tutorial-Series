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
// GraphicsResource.h —— 所有 GPU 资源的公共基类与描述结构
// -----------------------------------------------------------------------------
// 职责：定义 GraphicsResourceDesc（创建任何资源时都要传的“环境信息”），
//   以及 GraphicsResource 基类——所有 GPU 资源（SwapChain/DeviceContext/
//   VertexBuffer/Texture/...）都继承它，从而统一持有：设备、DXGI 工厂、日志，
//   以及指向 GraphicsDevice 的 shared_ptr（防止设备被提前析构）。
// 概念——生命周期：资源构造时拿设备/工厂的引用，并把设备 shared_ptr 存住，
//   保证只要还有资源存活，设备就不会被销毁。
// =============================================================================
#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Graphics/GraphicsLogUtils.h>

#include <d3d11.h>
#include <wrl.h>

namespace dx3d
{
	// GraphicsResourceDesc：创建任何图形资源时传入的环境信息。
	//   base：BaseDesc（含日志器引用）。
	//   graphicsDevice：指向 GraphicsDevice 的 shared_ptr（强引用，保活设备）。
	//   device / factory：D3D 设备与 DXGI 工厂的引用，资源创建底层 COM 对象时直接用。
	struct GraphicsResourceDesc
	{
		BaseDesc base;
		std::shared_ptr<const GraphicsDevice> graphicsDevice;
		ID3D11Device& device;
		IDXGIFactory& factory;
	};

	// GraphicsResource：所有 GPU 资源的基类。继承 Base 取得日志能力。
	//   构造时从 desc 拷贝设备 shared_ptr 与设备/工厂引用，使子类可直接用 m_device 创建底层资源。
	class GraphicsResource : public Base
	{
	public:
		// 构造函数：委托 Base(desc.base) 存日志器，并把设备/工厂引用与 shared_ptr 存为成员。
		explicit GraphicsResource(const GraphicsResourceDesc& desc):
			Base(desc.base),
			m_graphicsDevice(desc.graphicsDevice),
			m_device(desc.device),
			m_factory(desc.factory)
		{
		}

	protected:
		// 指向 GraphicsDevice 的 shared_ptr（const 表示资源不会修改设备）。
		//   作用是保活设备：只要本资源存活，设备就不会被析构。
		std::shared_ptr<const GraphicsDevice> m_graphicsDevice;
		// D3D11 设备引用：子类用它创建底层 COM 资源（缓冲、纹理、视图等）。
		ID3D11Device& m_device;
		// DXGI 工厂引用：子类（如 SwapChain）用它创建 DXGI 对象（交换链）。
		IDXGIFactory& m_factory;
	};
}