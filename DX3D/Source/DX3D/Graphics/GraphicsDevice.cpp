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
// GraphicsDevice.cpp —— D3D11 图形设备的实现
// -----------------------------------------------------------------------------
// 本文件实现设备初始化（创建 D3D11 设备 + 即时上下文 + DXGI 链）与资源工厂方法。
// 重点理解 COM 对象的“父子链”回溯：D3D 设备 -> IDXGIDevice -> IDXGIAdapter ->
// IDXGIFactory，每一层用 QueryInterface / GetParent 逐级向上拿到。
// =============================================================================
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/GraphicsLogUtils.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/ConstantBuffer.h>
#include <DX3D/Graphics/Texture.h>
#include <DX3D/Graphics/Sampler.h>
#include <DX3D/Graphics/GraphicsPipelineLayout.h>

// 构造函数：初始化 D3D11 设备并回溯 DXGI 工厂链。
// 委托构造 Base(desc.base) 把日志器存好。
dx3d::GraphicsDevice::GraphicsDevice(const GraphicsDeviceDesc& desc): Base(desc.base)
{
	// featureLevel：创建后用来记录实际获得的特性等级（如 11_0）。这里不强制指定。
	D3D_FEATURE_LEVEL featureLevel{};
	// createDeviceFlags：传给 D3D11CreateDevice 的标志位，下面按需置位。
	UINT createDeviceFlags{};

#ifdef _DEBUG
	// 调试构建下开启 D3D11 调试层（Debug Layer），可捕获错误的 API 用法并输出到输出窗口。
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// D3D11CreateDevice：创建 D3D11 逻辑设备 + 即时上下文。
	//   NULL 适配器 -> 使用默认显卡；D3D11_DRIVER_TYPE_HARDWARE -> 硬件加速（GPU）；
	//   NULL/0 表示不指定特定特性等级，由运行时选择最高支持；
	//   D3D11_SDK_VERSION 要求使用 D3D11 SDK。成功后输出设备、上下文、实际特性等级。
	//   DX3DGraphicsLogThrowOnFail 检查 HRESULT，失败则记录日志并抛异常。
	DX3DGraphicsLogThrowOnFail(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
		NULL, 0, D3D11_SDK_VERSION,
		&m_d3dDevice, &featureLevel, &m_d3dContext),
		"Direct3D11 initialization failed.");

	// QueryInterface：在同一个 COM 对象上查询另一个接口。这里从 D3D 设备拿到 IDXGIDevice，
	// 它是 DXGI 体系中代表“设备”的接口，是通往适配器/工厂的入口。IID_PPV_ARGS 宏生成正确 IID 并取地址。
	DX3DGraphicsLogThrowOnFail(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&m_dxgiDevice)),
		"QueryInterface failed to retrieve IDXGIDevice.");

	// GetParent：DXGI 对象层次中向上取父对象。IDXGIDevice 的父是 IDXGIAdapter（显卡适配器）。
	DX3DGraphicsLogThrowOnFail(m_dxgiDevice->GetParent(IID_PPV_ARGS(&m_dxgiAdapter)),
		"GetParent failed to retrieve IDXGIAdapter.");

	// 再向上取父：适配器的父是 IDXGIFactory，它是创建交换链等 DXGI 对象的顶层工厂。
	DX3DGraphicsLogThrowOnFail(m_dxgiAdapter->GetParent(IID_PPV_ARGS(&m_dxgiFactory)),
		"GetParent failed to retrieve IDXGIFactory.");

}

// 析构函数：函数体为空。所有 ComPtr 成员在析构时自动调用 Release，安全释放底层 COM 对象。
dx3d::GraphicsDevice::~GraphicsDevice()
{
}

// 创建交换链：用 desc 与设备描述 make_shared 一个 SwapChain。资源构造时需要设备与工厂引用。
dx3d::RefPtr<dx3d::SwapChain> dx3d::GraphicsDevice::createSwapChain(const SwapChainDesc& desc)
{
	return std::make_shared<SwapChain>(desc, getGraphicsResourceDesc());
}

// 创建命令上下文（延迟上下文）：用于在独立线程/序列中累积绘制命令，最后由 executeCommandList 提交。
dx3d::RefPtr<dx3d::DeviceContext> dx3d::GraphicsDevice::createDeviceContext()
{
	return std::make_shared<DeviceContext>(getGraphicsResourceDesc());
}

// 编译着色器：把 HLSL 源码编译为 GPU 可执行的字节码（ShaderBinary）。
dx3d::RefPtr<dx3d::ShaderBinary> dx3d::GraphicsDevice::compileShader(const ShaderCompileDesc& desc)
{
	return std::make_shared<ShaderBinary>(desc, getGraphicsResourceDesc());
}

// 创建图形管线状态：将着色器、输入布局等绑定关系打包成一个可整体设置的状态对象。
dx3d::RefPtr<dx3d::GraphicsPipelineState> dx3d::GraphicsDevice::createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc)
{
	return std::make_shared<GraphicsPipelineState>(desc, getGraphicsResourceDesc());
}

// 创建顶点缓冲：把顶点数据上传到显存，供输入装配阶段读取。
dx3d::RefPtr<dx3d::VertexBuffer> dx3d::GraphicsDevice::createVertexBuffer(const VertexBufferDesc& desc)
{
	return std::make_shared<VertexBuffer>(desc, getGraphicsResourceDesc());
}

// 创建图形管线布局：由顶点/像素着着色器字节码派生输入签名，供管线状态使用。
dx3d::RefPtr<dx3d::GraphicsPipelineLayout> dx3d::GraphicsDevice::createGraphicsPipelineLayout(const GraphicsPipelineLayoutDesc& desc)
{
	return std::make_shared<GraphicsPipelineLayout>(desc, getGraphicsResourceDesc());
}

// 创建常量缓冲：用于向着色器传递矩阵、光照参数等小量统一数据。
dx3d::RefPtr<dx3d::ConstantBuffer> dx3d::GraphicsDevice::createConstantBuffer(const ConstantBufferDesc& desc)
{
	return std::make_shared<ConstantBuffer>(desc, getGraphicsResourceDesc());
}

// 创建索引缓冲：存储顶点索引，决定顶点如何装配成三角形（可复用顶点、节省显存）。
dx3d::RefPtr<dx3d::IndexBuffer> dx3d::GraphicsDevice::createIndexBuffer(const IndexBufferDesc& desc)
{
	return std::make_shared<IndexBuffer>(desc, getGraphicsResourceDesc());
}

// 创建 2D 纹理：把像素数据上传到显存，供像素着色器采样。
dx3d::RefPtr<dx3d::Texture> dx3d::GraphicsDevice::createTexture(const TextureDesc& desc)
{
	return std::make_shared<Texture>(desc, getGraphicsResourceDesc());
}

// 创建采样器：定义纹理采样规则（过滤、寻址、各向异性等）。
dx3d::RefPtr<dx3d::Sampler> dx3d::GraphicsDevice::createSampler(const SamplerDesc& desc)
{
	return std::make_shared<Sampler>(desc, getGraphicsResourceDesc());
}

// 提交命令列表：把延迟上下文累积的命令“打包”后在即时上下文上回放，真正发给 GPU。
// 这是“延迟渲染”模式的核心：先在 DeviceContext 录制，再统一提交，便于多线程与状态一致性。
void dx3d::GraphicsDevice::executeCommandList(DeviceContext& context)
{
	// 命令列表对象，用于接收 FinishCommandList 的结果。
	Microsoft::WRL::ComPtr<ID3D11CommandList> list{};
	// FinishCommandList：结束延迟上下文的录制，输出一个 ID3D11CommandList。
	//   第一个参数 false 表示不分应用播放的命令边界（不需要恢复上下文状态）。
	auto hr = context.m_context->FinishCommandList(false, &list);
	if (FAILED(hr))
	{
		// 失败仅记录错误日志并返回（不抛异常），由调用方决定后续处理。
		DX3DLogError("FinishCommandList failed.");
		return;
	}
	// 在即时上下文上回放命令列表。第二个参数 false 表示回放后不恢复上下文旧状态。
	m_d3dContext->ExecuteCommandList(list.Get(), false);
}

// 生成传给所有子资源的 GraphicsResourceDesc。
//   {m_logger} -> BaseDesc（日志器）；shared_from_this() -> 指向本设备的 shared_ptr（安全，
//   因为本类继承 enable_shared_from_this）；其余两个是设备与工厂的引用（解引用 ComPtr 取裸引用）。
// noexcept：承诺不抛异常，仅做聚合。
dx3d::GraphicsResourceDesc dx3d::GraphicsDevice::getGraphicsResourceDesc() const noexcept
{
	return { {m_logger}, shared_from_this(), *m_d3dDevice.Get(), *m_dxgiFactory.Get() };
}
