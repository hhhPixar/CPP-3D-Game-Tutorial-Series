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
// GraphicsDevice.h —— Direct3D 11 图形设备核心（引擎的“GPU 入口”）
// -----------------------------------------------------------------------------
// 职责：封装 D3D11 逻辑设备（ID3D11Device）与 DXGI 基础设施（IDXGIFactory /
// IDXGIAdapter / IDXGIDevice）。它是整个图形子系统的“工厂”：所有 GPU 资源
// （交换链、上下文、着色器、管线、顶点/索引/常量缓冲、纹理、采样器）都由它创建。
// 架构位置：处于图形层最底层，上层 Display/Game 通过它拿到资源与执行能力。
// 关键概念：
//   - 设备 vs 上下文：设备负责“创建资源”，上下文负责“执行命令”，二者分离。
//   - ComPtr：Microsoft::WRL::ComPtr 是 COM 对象的智能指针，自动管理引用计数，
//     析构时自动 Release，避免内存/资源泄漏。
//   - enable_shared_from_this：让对象能安全地返回指向自身的 shared_ptr，避免
//     一份对象被两份无关 shared_ptr 管理（会导致二次析构）。子类创建资源时需要
//     把自身 shared_ptr 传给资源，使其能反向持有设备的强引用。
// =============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <d3d11.h>
#include <wrl.h>

namespace dx3d
{
	// GraphicsDevice：D3D11 设备封装。final 表示不可再被继承。
	// 继承 Base（提供日志能力）与 std::enable_shared_from_this<GraphicsDevice>
	// （使其能安全返回自身的 shared_ptr，供子资源持有设备引用）。
	class GraphicsDevice final: public Base, public std::enable_shared_from_this<GraphicsDevice>
	{
	public:
		// 构造函数：根据 desc 初始化 D3D11 设备、即时上下文，并向上回溯拿到 DXGI 适配器/工厂。
		// explicit 防止隐式转换。
		explicit GraphicsDevice(const GraphicsDeviceDesc& desc);
		// 析构函数：ComPtr 成员会自动释放底层 COM 对象，故函数体为空。
		virtual ~GraphicsDevice() override;

		// ---- 资源工厂方法：每个都 std::make_shared 一个资源，并把“设备描述”传进去 ----
		// 创建交换链（前后台缓冲 + 呈现），负责把渲染结果显示到窗口。
		RefPtr<SwapChain> createSwapChain(const SwapChainDesc& desc);
		// 创建命令上下文（延迟上下文 deferred context），用于累积绘制命令。
		RefPtr<DeviceContext> createDeviceContext();
		// 编译 HLSL 着色器源码为可被 GPU 使用的字节码（ShaderBinary）。
		RefPtr<ShaderBinary> compileShader(const ShaderCompileDesc& desc);
		// 创建图形管线状态对象（绑定顶点着色器/像素着色器/输入布局等）。
		RefPtr<GraphicsPipelineState> createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc);
		// 创建顶点缓冲（显存中存放顶点数据的数组）。
		RefPtr<VertexBuffer> createVertexBuffer(const VertexBufferDesc& desc);
		// 创建图形管线布局（由顶点/像素着色器字节码派生的输入签名布局）。
		RefPtr<GraphicsPipelineLayout> createGraphicsPipelineLayout(const GraphicsPipelineLayoutDesc& desc);
		// 创建常量缓冲（GPU 可读的小块数据，用于向着色器传递矩阵、光照等参数）。
		RefPtr<ConstantBuffer> createConstantBuffer(const ConstantBufferDesc& desc);
		// 创建索引缓冲（顶点索引数组，决定顶点如何拼成三角形）。
		RefPtr<IndexBuffer> createIndexBuffer(const IndexBufferDesc& desc);
		// 创建 2D 纹理（显存中的图像数据，供像素着色器采样）。
		RefPtr<Texture> createTexture(const TextureDesc& desc);
		// 创建采样器（控制纹理如何被采样：过滤方式、寻址模式等）。
		RefPtr<Sampler> createSampler(const SamplerDesc& desc);


		// 提交命令列表：把延迟上下文累积的命令回放到即时上下文执行（真正发给 GPU）。
		void executeCommandList(DeviceContext& context);
	private:
		// 生成传给子资源的 GraphicsResourceDesc：包含设备引用、DXGI 工厂引用、
		// 以及指向自身的 shared_ptr（通过 shared_from_this 安全获取）。
		GraphicsResourceDesc getGraphicsResourceDesc() const noexcept;
	private:
		// D3D11 逻辑设备：创建各类 GPU 资源的“工厂”。ComPtr 自动管理其 COM 生命周期。
		Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice{};
		// 即时上下文（immediate context）：直接向 GPU 发出命令的通道，executeCommandList 在此回放。
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext{};
		// DXGI 设备接口：由 D3D 设备 QueryInterface 获得，用于向上获取适配器。
		Microsoft::WRL::ComPtr<IDXGIDevice> m_dxgiDevice{};
		// DXGI 适配器：代表一块显卡（硬件），由 dxgiDevice->GetParent 获取。
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_dxgiAdapter{};
		// DXGI 工厂：创建交换链等 DXGI 对象的顶层工厂，由 adapter->GetParent 获取。
		Microsoft::WRL::ComPtr<IDXGIFactory> m_dxgiFactory{};
	};
}

