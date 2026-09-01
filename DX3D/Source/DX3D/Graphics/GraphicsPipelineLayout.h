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

// 文件：GraphicsPipelineLayout.h
// 子系统：Graphics — 图形管线与着色器部分
// 职责：声明 GraphicsPipelineLayout 类——对顶点/像素着色器字节码做'反射'，
//   得到输入布局（顶点数据如何映射到着色器输入）与所需资源槽位数。
// 核心概念：
//   1) 输入布局（input layout）：顶点缓冲里是一串字节，GPU 怎么知道哪些字节是
//      位置、哪些是纹理坐标？答案是用'语义'（Semantic，如 POSITION/TEXCOORD/NORMAL）
//      标记每个字段。本类通过反射从着色器字节码里读出这些语义，生成
//      D3D11_INPUT_ELEMENT_DESC 数组，之后用来创建 ID3D11InputLayout。
//   2) 资源槽位（slots）：着色器通过'槽位号'绑定常量缓冲/纹理/采样器。反射能
//      查出着色器用到了哪些槽位，本类据此算出各类型需要的最大槽位数，供渲染时绑定。
// 上下游：由 GraphicsDevice::createGraphicsPipelineLayout 创建 → 交给
//   GraphicsPipelineState 据此创建真正的 GPU 对象（InputLayout/VS/PS）。
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <d3dcompiler.h>

namespace dx3d
{
	// 图形管线布局：持有 VS/PS 字节码，并保存反射得到的输入元素与槽位信息。
	// final 表示最终类，不可继承；继承 GraphicsResource 以访问 ID3D11Device 等。
	class GraphicsPipelineLayout final : public GraphicsResource
	{
	public:
		// 构造函数：接收 VS/PS 字节码，对二者做反射，提取输入元素与槽位信息。实现见 .cpp。
		GraphicsPipelineLayout(const GraphicsPipelineLayoutDesc& desc, const GraphicsResourceDesc& gDesc);
		// 返回顶点着色器字节码（指针 + 大小），供创建 ID3D11VertexShader 与 InputLayout。
		BinaryData getVSBinaryData() const noexcept;
		// 返回像素着色器字节码（指针 + 大小），供创建 ID3D11PixelShader。
		BinaryData getPSBinaryData() const noexcept;
		// 返回输入元素数组（D3D11_INPUT_ELEMENT_DESC* + 元素个数），供创建 ID3D11InputLayout。
		BinaryData getInputElementsData() const noexcept;
		// 下面三个方法返回反射算出的各资源最大槽位数，渲染器据此决定要绑定多少个槽。
		ui32 getMaxTextureSlots() const noexcept;
		ui32 getMaxSamplerSlots() const noexcept;
		ui32 getMaxConstantBufferSlots() const noexcept;

	private:
		// 私有辅助：对单个着色器字节码做反射——填充输入元素（仅 VS）与更新槽位上限。
		void processShaderBinary(ShaderBinary& binary);
	private:
		// 顶点/像素着色器字节码（RefPtr 即 std::shared_ptr，共享所有权）。
		RefPtr<ShaderBinary> m_vsBinary{};
		RefPtr<ShaderBinary> m_psBinary{};
		// 着色器反射对象：下标 0 对应 VS、1 对应 PS（与 ShaderType 枚举值一致）。
		// ID3D11ShaderReflection 可查询着色器的输入参数与绑定的资源。
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_reflections[2]{};

		// 输入元素描述数组：每个元素描述一个顶点字段（语义、格式、对齐等）。
		// D3D11_STANDARD_VERTEX_ELEMENT_COUNT 是 D3D11 预设的顶点元素上限。
		D3D11_INPUT_ELEMENT_DESC m_elements[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
		// 实际的输入元素个数（= 顶点着色器的输入参数个数）。
		ui32 m_numElements{};
		// 下面三个成员记录各资源所需的最大槽位号 +1（即需要绑定的槽位数）。
		ui32 m_maxTextureSlots{};
		ui32 m_maxSamplerSlots{};
		ui32 m_maxBufferSlots{};
	};
}

