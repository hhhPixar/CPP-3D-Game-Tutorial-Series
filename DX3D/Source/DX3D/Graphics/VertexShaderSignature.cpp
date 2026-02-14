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

#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsUtils.h>
#include <d3dcompiler.h>
#include <ranges>

dx3d::VertexShaderSignature::VertexShaderSignature(const VertexShaderSignatureDesc& desc, const GraphicsResourceDesc& gDesc):
	GraphicsResource(gDesc), m_vsBinary(desc.vsBinary)
{
	if (!desc.vsBinary) DX3DLogThrowInvalidArg("No shader binary provided.");
	if (desc.vsBinary->getType() != ShaderType::VertexShader)
		DX3DLogThrowInvalidArg("The 'vsBinary' member is not a valid vertex shader binary.");

	auto vsData = m_vsBinary->getData();

	DX3DGraphicsLogThrowOnFail(D3DReflect(
		vsData.data,
		vsData.dataSize,
		IID_PPV_ARGS(&m_shaderReflection)),
		"D3DReflect failed.");

	D3D11_SHADER_DESC shaderDesc{};
	DX3DGraphicsLogThrowOnFail(m_shaderReflection->GetDesc(&shaderDesc),
		"ID3D11ShaderReflection::GetDesc failed.");

	m_numElements = shaderDesc.InputParameters;

	D3D11_SIGNATURE_PARAMETER_DESC params[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
	for (auto i : std::views::iota(0u, m_numElements))
	{
		DX3DGraphicsLogThrowOnFail(m_shaderReflection->GetInputParameterDesc(i, &params[i]),
			"ID3D11ShaderReflection::GetInputParameterDesc failed.");
	}

	for (auto i : std::views::iota(0u, m_numElements))
	{
		auto param = params[i];
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

dx3d::BinaryData dx3d::VertexShaderSignature::getShaderBinaryData() const noexcept
{
	return m_vsBinary->getData();
}

dx3d::BinaryData dx3d::VertexShaderSignature::getInputElementsData() const noexcept
{
	return
	{
		m_elements,
		m_numElements
	};
}
