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

// 文件：GraphicsPipelineState.cpp
// 子系统：Graphics — 图形管线部分
// 职责：实现 GraphicsPipelineState 构造函数——用 layout 提供的数据创建三个 GPU 对象。
// 关键 D3D11 调用：
//   CreateInputLayout  用输入元素描述 + VS 字节码创建'输入布局'（顶点缓冲→着色器输入的映射）。
//   CreateVertexShader 用 VS 字节码创建顶点着色器对象。
//   CreatePixelShader  用 PS 字节码创建像素着色器对象。
// 说明：CreateInputLayout 需要 VS 字节码，是因为输入布局要和顶点着色器的输入签名匹配。
//   第三个参数 nullptr 表示不需要类链接（class linkage，仅用于接口继承的着色器）。
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsPipelineLayout.h>


// 构造函数：从 layout 取出字节码与输入元素，创建 InputLayout/VS/PS 三个 GPU 对象。
// desc  ：含对 layout 的引用；gDesc：图形资源公共描述（提供 m_device）。
dx3d::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& desc, 
	const GraphicsResourceDesc& gDesc):
	GraphicsResource(gDesc)
{

	// 从 layout 取出：VS 字节码、PS 字节码、输入元素数组。这三份数据由反射得到。
	auto vs = desc.layout.getVSBinaryData();
	auto ps = desc.layout.getPSBinaryData();
	auto vsInputElements = desc.layout.getInputElementsData();

	// 创建输入布局：参数为输入元素数组、元素个数、VS 字节码(及其大小)、输出 m_layout。
	// 输入布局决定 GPU 如何从顶点缓冲字节流中取出 POSITION/TEXCOORD/NORMAL 等字段。
	DX3DGraphicsLogThrowOnFail(
		m_device.CreateInputLayout(
			static_cast<const D3D11_INPUT_ELEMENT_DESC*>(vsInputElements.data), 
			static_cast<ui32>(vsInputElements.dataSize), 
			vs.data, 
			vs.dataSize, 
			&m_layout),
		"CreateInputLayout failed.");

	// 创建顶点着色器：用 VS 字节码创建 ID3D11VertexShader，存入 m_vs。
	DX3DGraphicsLogThrowOnFail(
		m_device.CreateVertexShader(vs.data, vs.dataSize, nullptr, &m_vs),
		"CreateVertexShader failed.");

	// 创建像素着色器：用 PS 字节码创建 ID3D11PixelShader，存入 m_ps。
	DX3DGraphicsLogThrowOnFail(
		m_device.CreatePixelShader(ps.data, ps.dataSize, nullptr, &m_ps),
		"CreatePixelShader failed.");
}
