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

// 文件：GraphicsPipelineLayout.cpp
// 子系统：Graphics — 图形管线与着色器部分
// 职责：实现 GraphicsPipelineLayout——对着色器字节码做'反射'（reflection），
//   自动得到输入布局与所需资源槽位，免去手写输入元素描述的繁琐。
// 核心概念——着色器反射（shader reflection）：
//   D3DReflect 能从编译后的字节码里'反向查出'着色器的结构：它需要哪些输入
//   参数（语义 POSITION/TEXCOORD/NORMAL…）、绑定了哪些资源（cbuffer/texture/sampler）
//   及其槽位号。本文件据此生成输入元素数组并统计槽位上限。
// 关键产物：
//   - m_elements[]：输入元素描述（顶点缓冲 → 着色器输入的映射）。
//   - m_maxBufferSlots/m_maxTextureSlots/m_maxSamplerSlots：各资源所需槽位数。
#include <DX3D/Graphics/GraphicsPipelineLayout.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsUtils.h>
#include <d3dcompiler.h>
#include <ranges>

// 构造函数：接收 VS/PS 字节码，先校验类型，再对二者做反射。
// desc  ：含 vsBinary、psBinary；gDesc：图形资源公共描述。
dx3d::GraphicsPipelineLayout::GraphicsPipelineLayout(const GraphicsPipelineLayoutDesc& desc, const GraphicsResourceDesc& gDesc) : 
	GraphicsResource(gDesc), 
	m_vsBinary(desc.vsBinary), 
	m_psBinary(desc.psBinary)
{
	// 校验：VS/PS 字节码不能为空，且类型必须分别是 VertexShader/PixelShader。
	if (!desc.vsBinary) DX3DLogThrowInvalidArg("No shader binary provided.");
	if (desc.vsBinary->getType() != ShaderType::VertexShader)
		DX3DLogThrowInvalidArg("The 'vsBinary' member is not a valid vertex shader binary.");
	if (!desc.psBinary) DX3DLogThrowInvalidArg("No shader binary provided.");
	if (desc.psBinary->getType() != ShaderType::PixelShader)
		DX3DLogThrowInvalidArg("The 'psBinary' member is not a valid pixel shader binary.");

	// 对顶点着色器与像素着色器分别做反射（填充输入元素、统计槽位）。
	processShaderBinary(*m_vsBinary);
	processShaderBinary(*m_psBinary);
}

// 返回顶点着色器字节码，供创建 ID3D11VertexShader 与输入布局。
dx3d::BinaryData dx3d::GraphicsPipelineLayout::getVSBinaryData() const noexcept
{
	return m_vsBinary->getData();
}

// 返回像素着色器字节码，供创建 ID3D11PixelShader。
dx3d::BinaryData dx3d::GraphicsPipelineLayout::getPSBinaryData() const noexcept
{
	return m_psBinary->getData();
}


// 返回输入元素数组及其个数，供 ID3D11Device::CreateInputLayout 使用。
dx3d::BinaryData dx3d::GraphicsPipelineLayout::getInputElementsData() const noexcept
{
	return
	{
		m_elements,
		m_numElements
	};
}

// 下面三个方法返回各资源所需的最大槽位数，渲染器据此绑定相应数量的资源。
dx3d::ui32 dx3d::GraphicsPipelineLayout::getMaxTextureSlots() const noexcept
{
	return m_maxTextureSlots;
}

dx3d::ui32 dx3d::GraphicsPipelineLayout::getMaxSamplerSlots() const noexcept
{
	return m_maxSamplerSlots;
}

dx3d::ui32 dx3d::GraphicsPipelineLayout::getMaxConstantBufferSlots() const noexcept
{
	return m_maxBufferSlots;
}

// 对单个着色器字节码做反射。
// binary：VS 或 PS 字节码。函数内：用 D3DReflect 取反射接口 → GetDesc 取整体
//   描述 → 若是 VS 则读输入参数生成输入元素；遍历绑定资源统计槽位上限。
void dx3d::GraphicsPipelineLayout::processShaderBinary(ShaderBinary& binary)
{
	auto data = binary.getData();
	// 取该着色器类型对应的反射对象引用（下标 = ShaderType 枚举值）。
	// 下面 D3DReflect 对字节码做反射，得到 ID3D11ShaderReflection 查询接口。
	auto& reflection = m_reflections[static_cast<ui32>(binary.getType())];

	DX3DGraphicsLogThrowOnFail(D3DReflect(
		data.data,
		data.dataSize,
		IID_PPV_ARGS(&reflection)),
		"D3DReflect failed.");

	// GetDesc：取得着色器的整体描述（输入参数个数、绑定资源个数等）。
	D3D11_SHADER_DESC shaderDesc{};
	DX3DGraphicsLogThrowOnFail(reflection->GetDesc(&shaderDesc),
		"ID3D11ShaderReflection::GetDesc failed.");

	// 只有顶点着色器才有'输入参数'（来自顶点缓冲），因此只在 VS 时生成输入元素。
	// 像素着色器的输入来自光栅化，不参与输入布局，故跳过。
	if (binary.getType() == ShaderType::VertexShader)
	{
		// 输入参数个数 = 顶点着色器需要的顶点字段数（POSITION、TEXCOORD、NORMAL 等）。
		m_numElements = shaderDesc.InputParameters;
		// 临时数组，存放每个输入参数的语义信息（名字、索引、组件类型、掩码）。
		D3D11_SIGNATURE_PARAMETER_DESC params[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
		// 遍历每个输入参数，用 GetInputParameterDesc 取其语义描述。
		// std::views::iota(0u, n) 生成 [0, n) 的整数序列（C++20 ranges）。
		for (auto i : std::views::iota(0u, m_numElements))
		{
			DX3DGraphicsLogThrowOnFail(reflection->GetInputParameterDesc(i, &params[i]),
				"ID3D11ShaderReflection::GetInputParameterDesc failed.");
		}
		// 再遍历一次，把每个参数转成 D3D11_INPUT_ELEMENT_DESC（输入布局的一个元素）。
		for (auto i : std::views::iota(0u, m_numElements))
		{
			auto param = params[i];
			// 输入元素各字段：语义名、语义索引、数据格式(由组件类型+掩码算出)、输入槽(0)、
			//   对齐方式(D3D11_APPEND_ALIGNED_ELEMENT=自动紧凑排列)、输入分类(逐顶点数据)、
			//   实例化步长(0 表示不用实例化)。
			m_elements[i] = {
				param.SemanticName,
				param.SemanticIndex,
				GraphicsUtils::GetDXGIFormatFromMask(param.ComponentType,param.Mask),
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			};
		}
	}

	// 下面这个独立块：遍历着色器绑定的所有资源，统计各类型用到的最大槽位号。
	{
		// desc 描述一个被绑定的资源（类型、绑定点=槽位号等）。
		D3D11_SHADER_INPUT_BIND_DESC desc{};
		// 遍历所有绑定资源，逐个查询其绑定信息。
		for (auto i : std::views::iota(0u, shaderDesc.BoundResources))
		{
			DX3DGraphicsLogThrowOnFail(reflection->GetResourceBindingDesc(i, &desc),
				"ID3D11ShaderReflection::GetInputParameterDesc failed.");
			// 按资源类型更新对应槽位上限：+1 是因为 BindPoint 是从 0 起的索引，
			// 需要'最大索引+1'个槽。std::max 保证多次反射取最大值（VS/PS 都会被反射）。
			if (desc.Type == D3D_SIT_CBUFFER)
				m_maxBufferSlots = std::max(m_maxBufferSlots, desc.BindPoint + 1);
			if (desc.Type == D3D_SIT_TEXTURE)
				m_maxTextureSlots = std::max(m_maxTextureSlots, desc.BindPoint + 1);
			if (desc.Type == D3D_SIT_SAMPLER)
				m_maxSamplerSlots = std::max(m_maxSamplerSlots, desc.BindPoint + 1);
		}
	}
}
