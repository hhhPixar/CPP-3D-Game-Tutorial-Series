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

// 文件：VertexShaderSignature.cpp
// 子系统：Graphics — 着色器部分
// 职责：实现 VertexShaderSignature——对顶点着色器字节码做反射，生成输入元素描述。
// 流程：D3DReflect 取反射接口 → GetDesc 取输入参数个数 → 逐个 GetInputParameterDesc
//   取语义信息（名字/索引/组件类型/掩码）→ 转成 D3D11_INPUT_ELEMENT_DESC 数组。
// 说明：这与 GraphicsPipelineLayout 中 VS 的反射逻辑一致，只是此处只针对顶点着色器。
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsUtils.h>
#include <d3dcompiler.h>
#include <ranges>

// 构造函数：对 VS 字节码做反射并生成输入元素。desc 含 vsBinary；gDesc 公共描述。
dx3d::VertexShaderSignature::VertexShaderSignature(const VertexShaderSignatureDesc& desc, const GraphicsResourceDesc& gDesc):
	GraphicsResource(gDesc), m_vsBinary(desc.vsBinary)
{
	// 校验：字节码不能为空，且必须是顶点着色器类型。
	if (!desc.vsBinary) DX3DLogThrowInvalidArg("No shader binary provided.");
	if (desc.vsBinary->getType() != ShaderType::VertexShader)
		DX3DLogThrowInvalidArg("The 'vsBinary' member is not a valid vertex shader binary.");

	// 取出顶点着色器字节码（指针 + 大小），作为反射的输入。
	auto vsData = m_vsBinary->getData();

	// D3DReflect：对字节码做反射，得到 ID3D11ShaderReflection，用于查询输入参数。
	DX3DGraphicsLogThrowOnFail(D3DReflect(
		vsData.data,
		vsData.dataSize,
		IID_PPV_ARGS(&m_shaderReflection)),
		"D3DReflect failed.");

	// GetDesc：取得着色器整体描述，其中 InputParameters = 输入参数（顶点字段）个数。
	D3D11_SHADER_DESC shaderDesc{};
	DX3DGraphicsLogThrowOnFail(m_shaderReflection->GetDesc(&shaderDesc),
		"ID3D11ShaderReflection::GetDesc failed.");

	// 输入参数个数 = 顶点着色器需要的字段数（POSITION/TEXCOORD/NORMAL 等）。
	m_numElements = shaderDesc.InputParameters;

	// 临时数组，存放每个输入参数的语义信息。
	D3D11_SIGNATURE_PARAMETER_DESC params[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
	// 遍历每个输入参数，GetInputParameterDesc 取其语义描述。
	// std::views::iota(0u, n) 生成 [0, n) 的整数序列（C++20 ranges）。
	for (auto i : std::views::iota(0u, m_numElements))
	{
		DX3DGraphicsLogThrowOnFail(m_shaderReflection->GetInputParameterDesc(i, &params[i]),
			"ID3D11ShaderReflection::GetInputParameterDesc failed.");
	}

	// 把每个参数转成 D3D11_INPUT_ELEMENT_DESC（输入布局的一个元素）。
	for (auto i : std::views::iota(0u, m_numElements))
	{
		auto param = params[i];
		// 元素各字段：语义名、语义索引、数据格式(由组件类型+掩码算出)、输入槽(0)、
		//   对齐方式(D3D11_APPEND_ALIGNED_ELEMENT=自动紧凑排列)、输入分类(逐顶点数据)、
		//   实例化步长(0=不用实例化)。
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

// 返回所持的顶点着色器字节码（指针 + 大小）。
dx3d::BinaryData dx3d::VertexShaderSignature::getShaderBinaryData() const noexcept
{
	return m_vsBinary->getData();
}

// 返回反射得到的输入元素数组及其个数，供创建 ID3D11InputLayout。
dx3d::BinaryData dx3d::VertexShaderSignature::getInputElementsData() const noexcept
{
	return
	{
		m_elements,
		m_numElements
	};
}
