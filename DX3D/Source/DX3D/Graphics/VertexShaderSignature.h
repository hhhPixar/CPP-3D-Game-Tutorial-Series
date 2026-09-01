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

// 文件：VertexShaderSignature.h
// 子系统：Graphics — 着色器部分
// 职责：声明 VertexShaderSignature 类——对顶点着色器字节码做反射，提取其'输入签名'
//   （即顶点着色器需要哪些输入：POSITION/TEXCOORD/NORMAL…），生成输入元素描述。
// 说明：本类与 GraphicsPipelineLayout 中'VS 部分'的反射逻辑相同，但只处理顶点着色器、
//   不统计资源槽位。可视为'只生成输入布局元素'的轻量版反射工具。
// 核心概念——输入签名 / 语义（semantic）：
//   顶点着色器的输入用'语义'标注每个字段，反射能读出语义名、索引、组件类型，
//   据此自动生成 D3D11_INPUT_ELEMENT_DESC，免去手写输入布局描述。
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <d3dcompiler.h>

namespace dx3d
{
	// 顶点着色器输入签名描述。final 表示最终类；继承 GraphicsResource 以访问设备等。
	class VertexShaderSignature final: public GraphicsResource
	{
	public:
		// 构造函数：对 VS 字节码做反射，生成输入元素数组。实现见 .cpp。
		VertexShaderSignature(const VertexShaderSignatureDesc& desc, const GraphicsResourceDesc& gDesc);
		// 返回所持的顶点着色器字节码（指针 + 大小）。
		BinaryData getShaderBinaryData() const noexcept;
		// 返回反射得到的输入元素数组及其个数，供创建 ID3D11InputLayout。
		BinaryData getInputElementsData() const noexcept;
	private:
		// 所持的顶点着色器字节码（RefPtr 即 std::shared_ptr，共享所有权）。
		RefPtr<ShaderBinary> m_vsBinary{};
		// 着色器反射对象，用于查询顶点着色器的输入参数（语义、组件类型等）。
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> m_shaderReflection{};
		// 输入元素数组：每个元素描述一个顶点字段（语义、格式、对齐等）。
		D3D11_INPUT_ELEMENT_DESC m_elements[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
		ui32 m_numElements{};
	};
}

