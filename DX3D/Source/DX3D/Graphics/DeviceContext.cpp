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
// DeviceContext.cpp —— 绘制命令上下文的实现
// -----------------------------------------------------------------------------
// 本文件实现“录制绘制命令”的各个步骤。渲染一帧的典型顺序：
//   clearAndSetBackBuffer -> setViewportSize -> setGraphicsPipelineState ->
//   setVertexBuffer / setIndexBuffer -> setConstantBuffers / setTextures /
//   setSamplers -> updateConstantBuffer -> drawIndexedTriangleList。
// 之后由 GraphicsDevice.executeCommandList 把这些命令提交给即时上下文。
// 注意：本类使用延迟上下文（deferred context），命令先被录制，不立即执行。
// =============================================================================
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/ConstantBuffer.h>
#include <DX3D/Graphics/Texture.h>
#include <DX3D/Graphics/Sampler.h>
#include <ranges>

// 构造函数：创建一个延迟上下文（deferred context）。
//   延迟上下文不直接提交命令给 GPU，而是把命令录制成命令列表，最后回放。
//   CreateDeferredContext(0,...)：0 表示不使用多帧延迟（无上下文类型标志）。
dx3d::DeviceContext::DeviceContext(const GraphicsResourceDesc& gDesc): GraphicsResource(gDesc)
{
	DX3DGraphicsLogThrowOnFail(m_device.CreateDeferredContext(0, &m_context),
		"CreateDeferredContext failed.");
}

// 清屏并绑定后台缓冲：每帧绘制开始时调用。
//   color：背景清屏色（RGBA）。先把 Vec4 拆成 C 数组。
//   rtv：渲染目标视图（后台缓冲），dsv：深度/模板视图。
void dx3d::DeviceContext::clearAndSetBackBuffer(const SwapChain& swapChain, const Vec4& color)
{
	f32 fColor[] = { color.x,color.y,color.z,color.w };
	// 直接访问 swapChain 的私有成员 m_rtv/m_dsv（DeviceContext 与 SwapChain 无友元，
	//   这里依赖它们是同一 GraphicsResource 家族 + 这里通过 const 引用访问；
	//   实际访问的是 SwapChain 声明了 friend class DeviceContext，见 SwapChain.h）。
	auto rtv = swapChain.m_rtv.Get();
	auto dsv = swapChain.m_dsv.Get();

	// ClearRenderTargetView：用单一颜色清空渲染目标（后台缓冲）。
	m_context->ClearRenderTargetView(rtv, fColor);
	// ClearDepthStencilView：清空深度缓冲（置远值 1.0=最远）与模板缓冲（置 0）。
	//   D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL 表示同时清两者。
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	// OMSetRenderTargets：把渲染目标和深度/模板视图绑定到输出合并阶段（Output Merger）。
	//   之后像素着色器的输出会写入这个渲染目标，深度测试用这个 dsv。
	m_context->OMSetRenderTargets(1, &rtv, dsv);
}

// 设置图形管线状态：绑定输入布局 + 顶点/像素着色器。
//   这些状态决定顶点如何被解释、变换、着色。
void dx3d::DeviceContext::setGraphicsPipelineState(const GraphicsPipelineState& pipeline)
{
	// IASetInputLayout：设置输入布局，告诉 IA 阶段如何从顶点缓冲读取各属性（位置/法线等）。
	m_context->IASetInputLayout(pipeline.m_layout.Get());
	// VSSetShader：绑定顶点着色器（负责把顶点从模型空间变换到裁剪空间）。后两参数为类实例与实例数量，置空/0。
	m_context->VSSetShader(pipeline.m_vs.Get(), nullptr, 0);
	// PSSetShader：绑定像素着色器（决定每个像素的最终颜色）。
	m_context->PSSetShader(pipeline.m_ps.Get(), nullptr, 0);
}

// 绑定顶点缓冲到输入装配阶段。
//   stride = 单个顶点的字节大小（用于跨步寻址）；offset = 起始字节偏移（这里从头开始）。
void dx3d::DeviceContext::setVertexBuffer(const VertexBuffer& buffer)
{
	auto stride = buffer.m_vertexSize;
	auto buf = buffer.m_buffer.Get();
	auto offset = 0u;
	// IASetVertexBuffers：把顶点缓冲绑定到输入槽 0。1 表示绑定 1 个缓冲。
	m_context->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
}

// 绑定索引缓冲到输入装配阶段。
//   DXGI_FORMAT_R32_UINT：每个索引为 32 位无符号整数。第二个参数 0 表示字节偏移从 0 开始。
void dx3d::DeviceContext::setIndexBuffer(const IndexBuffer& buffer)
{
	m_context->IASetIndexBuffer(buffer.m_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

// 设置视口：决定渲染结果映射到渲染目标的哪块矩形区域。
//   深度范围 MinDepth=0..MaxDepth=1 是标准配置。
void dx3d::DeviceContext::setViewportSize(const Rect& size)
{
	D3D11_VIEWPORT vp{};
	vp.Width = static_cast<f32>(size.width);
	vp.Height = static_cast<f32>(size.height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	// RSSetViewports：光栅化阶段使用此视口，把裁剪空间坐标映射到屏幕像素。
	m_context->RSSetViewports(1, &vp);
}

// 绑定常量缓冲到顶点与像素着色器阶段。
//   超过最大槽位数的会忽略并告警；用 iota 生成 0..n-1 的索引遍历。
void dx3d::DeviceContext::setConstantBuffers(const std::span<ConstantBuffer*>& buffers)
{
	if (buffers.size() > MaxConstantBuffersPerStage)
	{
		DX3DLogWarning("Number of buffers exceeds {}. Extra buffers will be ignored.", MaxConstantBuffersPerStage)
	}
	auto numBuffers = static_cast<UINT>(std::min(buffers.size(), MaxConstantBuffersPerStage));
	for (auto i : std::views::iota(0u, numBuffers))
	{
		// 把非空缓冲的底层指针填入预分配数组；空指针槽位置空（解绑）。
		if (buffers[i]) m_constantBuffers[i] = (buffers[i]->m_buffer.Get());
		else  m_constantBuffers[i] = {};
	}
	// 绑定到顶点着色器阶段，从槽 0 开始，共 numBuffers 个。
	m_context->VSSetConstantBuffers(0, numBuffers, m_constantBuffers.data());
	// 同样绑定到像素着色器阶段。
	m_context->PSSetConstantBuffers(0, numBuffers, m_constantBuffers.data());
}

// 绑定纹理（作为着色器资源视图 SRV）到顶点与像素着色器阶段供采样读取。
void dx3d::DeviceContext::setTextures(const std::span<Texture*>& textures)
{
	if (textures.size() > MaxTexturesPerStage)
	{
		DX3DLogWarning("Number of textures exceeds {}. Extra textures will be ignored.", MaxTexturesPerStage)
	}
	auto numTextures = static_cast<UINT>(std::min(textures.size(), MaxTexturesPerStage));
	for (auto i : std::views::iota(0u, numTextures))
	{
		if (textures[i]) m_srv[i] = (textures[i]->m_srv.Get());
		else m_srv[i] = {};
	}
	// VSSetShaderResources / PSSetShaderResources：把 SRV 绑定到着色器，使其能读纹理。
	m_context->VSSetShaderResources(0, numTextures, m_srv.data());
	m_context->PSSetShaderResources(0, numTextures, m_srv.data());
}

// 绑定采样器到顶点与像素着色器阶段，定义纹理采样规则（过滤/寻址等）。
void dx3d::DeviceContext::setSamplers(const std::span<Sampler*>& samplers)
{
	if (samplers.size() > MaxSamplersPerStage)
	{
		DX3DLogWarning("Number of samplers exceeds {}. Extra samplers will be ignored.", MaxSamplersPerStage)
	}
	auto numSamplers = static_cast<UINT>(std::min(samplers.size(), MaxSamplersPerStage));
	for (auto i : std::views::iota(0u, numSamplers))
	{
		if (samplers[i]) m_samplers[i] = (samplers[i]->m_sampler.Get());
		else m_samplers[i] = {};
	}
	m_context->VSSetSamplers(0, numSamplers, m_samplers.data());
	m_context->PSSetSamplers(0, numSamplers, m_samplers.data());
}

// 更新常量缓冲：把 CPU 端 data 拷贝到 GPU 缓冲。
//   概念——GPU 同步：GPU 可能正在读旧数据，所以用 D3D11_MAP_WRITE_DISCARD
//   “丢弃”旧内容并返回一块可写的新区域，避免等待 GPU（不阻塞）。
void dx3d::DeviceContext::updateConstantBuffer(const ConstantBuffer& buffer, const std::span<const std::byte>& data)
{
	auto dataSize = static_cast<ui32>(data.size());
	if (!dataSize)
	{
		// 没有数据则直接报错返回，不做任何操作。
		DX3DLogError("No data passed to updateConstantBuffer.");
		return;
	}
	if (dataSize > buffer.m_size)
	{
		// 数据比缓冲大：截断，避免越界写入显存。
		DX3DLogWarning("Buffer size ({} bytes) exceeds the constant buffer limit ({} bytes). Extra bytes will be ignored.", dataSize,  buffer.m_size);
	}

	// 取实际要写入的字节数（不超过缓冲容量）。
	dataSize = std::min(dataSize , buffer.m_size);

	auto buf = buffer.m_buffer.Get();
	// mapped：存放映射后 CPU 可写指针与行距等信息。
	D3D11_MAPPED_SUBRESOURCE mapped{};

	// Map：把 GPU 资源临时映射为 CPU 可写。D3D11_MAP_WRITE_DISCARD 表示丢弃旧内容，
	//   返回一块新区域写入，不与 GPU 读取产生冲突（关键的性能/同步手段）。
	auto hr = m_context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
	{
		// 映射失败仅记录日志返回，不中断程序。
		DX3DLogError("ID3D11DeviceContext::Map failed.");
		return;
	}
	// 把数据拷到映射得到的 CPU 可写内存；之后 GPU 读取新内容。
	std::memcpy(mapped.pData, data.data(), dataSize);
	// Unmap：结束映射，告诉驱动写入完成，资源可被 GPU 重新访问。
	m_context->Unmap(buf, 0);
}

// 用顶点列表绘制三角形（无索引）。
void dx3d::DeviceContext::drawTriangleList(ui32 vertexCount, ui32 startVertexLocation)
{
	// 设置图元拓扑为三角形列表：每 3 个顶点组成一个独立三角形。
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Draw：发出绘制调用。vertexCount=绘制多少顶点；startVertexLocation=顶点缓冲偏移。
	m_context->Draw(vertexCount, startVertexLocation);
}

// 用索引缓冲绘制三角形（推荐方式，可复用顶点）。
void dx3d::DeviceContext::drawIndexedTriangleList(ui32 indexCount, ui32 startVertexIndex, ui32 startIndexLocation)
{
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// DrawIndexed：indexCount=读取多少索引；startIndexLocation=索引缓冲起始偏移；
	//   startVertexIndex=给顶点缓冲加一个统一偏移（用于把多套顶点数据组合在一起）。
	m_context->DrawIndexed(indexCount, startIndexLocation, startVertexIndex);
}