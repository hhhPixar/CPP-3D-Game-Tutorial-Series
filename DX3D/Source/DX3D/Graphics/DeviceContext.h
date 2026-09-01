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
// DeviceContext.h —— 绘制命令上下文（命令录制器）
// -----------------------------------------------------------------------------
// 职责：封装 ID3D11DeviceContext，是“真正执行绘制”的对象。注意本实现使用
//   延迟上下文（deferred context）：先在这里累积一系列 setXxx / drawXxx 命令，
//   最后由 GraphicsDevice.executeCommandList 把命令列表回放到即时上下文。
// 概念：设备 vs 上下文 —— 设备只“创建”资源，上下文负责“设置状态 + 发出绘制”。
//   渲染一帧 = 设置管线/缓冲/纹理/采样器/视口 + 清屏 + 调用 drawXxx。
// std::span：表示“一段连续元素的视图”，不持有内存，适合把数组/向量传给函数。
// std::array：编译期固定大小的数组，这里用它预存即将提交给 API 的槽位数组。
// =============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Math/Vec4.h>
#include <span>
#include <array>

namespace dx3d
{
	// DeviceContext：绘制命令录制器。继承 GraphicsResource 以拿到设备/工厂引用与日志。
	// final 表示不可再继承。friend class GraphicsDevice 让设备能访问私有 m_context 以录制命令列表。
	class DeviceContext final: public GraphicsResource
	{
	public:
		// 构造函数：gDesc 提供设备/工厂/日志；内部创建一个延迟上下文。
		explicit DeviceContext(const GraphicsResourceDesc& gDesc);
		// 清屏并绑定后台缓冲：用 color 清空渲染目标，清空深度/模板，并把后台缓冲设为输出目标。
		void clearAndSetBackBuffer(const SwapChain& swapChain, const Vec4& color);
		// 设置图形管线状态：绑定输入布局 + 顶点着色器 + 像素着色器。
		void setGraphicsPipelineState(const GraphicsPipelineState& pipeline);
		// 绑定顶点缓冲到输入装配阶段，供后续 draw 读取顶点。
		void setVertexBuffer(const VertexBuffer& buffer);
		// 绑定索引缓冲到输入装配阶段，供 drawIndexed 读取。
		void setIndexBuffer(const IndexBuffer& buffer);
		// 设置视口大小：决定渲染结果映射到屏幕的区域（通常等于后台缓冲尺寸）。
		void setViewportSize(const Rect& size);
		// 绑定常量缓冲：同时绑定到顶点与像素着色器阶段，用于传递矩阵/光照等参数。
		void setConstantBuffers(const std::span<ConstantBuffer*>& buffers);
		// 绑定纹理：以着色器资源视图(SRV)形式绑定到顶点/像素着色器阶段供采样。
		void setTextures(const std::span<Texture*>& textures);
		// 绑定采样器：定义纹理采样规则，同时绑定到顶点/像素着色器阶段。
		void setSamplers(const std::span<Sampler*>& samplers);
		// 更新常量缓冲内容：把 data 拷贝到 GPU 可读缓冲。使用 Map/Unmap + DISCARD。
		void updateConstantBuffer(const ConstantBuffer& buffer, const std::span<const std::byte>& data);
		// 用顶点列表绘制三角形（无索引）。vertexCount=顶点数；startVertexLocation=起始顶点偏移。
		void drawTriangleList(ui32 vertexCount, ui32 startVertexLocation);
		// 用索引缓冲绘制三角形。indexCount=索引数；startVertexIndex 给顶点缓冲加一个统一偏移；
		// startIndexLocation=从索引缓冲哪个位置开始读。
		void drawIndexedTriangleList(ui32 indexCount, ui32 startVertexIndex, ui32 startIndexLocation);
	public:
		// 每个着色器阶段（VS/PS）可同时绑定的常量缓冲最大槽位数（D3D11 限制，通常为 14）。
		static constexpr std::size_t MaxConstantBuffersPerStage{ D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT };
		// 每个着色器阶段可同时绑定的采样器最大槽位数。
		static constexpr std::size_t MaxSamplersPerStage{ D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT };
		// 每个着色器阶段可同时绑定的纹理(SRV)最大槽位数。
		static constexpr std::size_t MaxTexturesPerStage{ D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT };
	private:
		// 底层 D3D11 设备上下文（本实现为延迟上下文）。
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context{};
		// 预分配的常量缓冲指针数组，待提交给 VSSetConstantBuffers/PSSetConstantBuffers。
		std::array<ID3D11Buffer*,MaxConstantBuffersPerStage> m_constantBuffers{};
		// 预分配的着色器资源视图(SRV)数组，待提交给 VSSetShaderResources/PSSetShaderResources。
		std::array<ID3D11ShaderResourceView*,MaxTexturesPerStage> m_srv{};
		// 预分配的采样器状态数组，待提交给 VSSetSamplers/PSSetSamplers。
		std::array<ID3D11SamplerState*,MaxSamplersPerStage> m_samplers{};

		friend class GraphicsDevice;
	};
}

