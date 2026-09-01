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

// 文件：GraphicsPipelineState.h
// 子系统：Graphics — 图形管线部分
// 职责：声明 GraphicsPipelineState 类——把'布局 + 字节码'真正创建成 GPU 可用的
//   管线状态对象（输入布局 + 顶点着色器 + 像素着色器）。
// 核心概念——管线状态（pipeline state）：
//   在 D3D11 里，'管线状态'是绑定到设备上下文的一组对象：输入布局告诉 GPU
//   如何读顶点缓冲，顶点/像素着色器告诉 GPU 如何变换与着色。本类持有这三者，
//   在被设置到上下文时（见 DeviceContext::setGraphicsPipelineState）一并绑定。
// 与 GraphicsPipelineLayout 的分工：
//   Layout 负责'分析与准备'（反射出输入元素、保存字节码）；State 负责'创建 GPU
//   对象'（用这些数据调用 CreateInputLayout/CreateVertexShader/CreatePixelShader）。
// 友元 DeviceContext：允许上下文直接访问私有 GPU 对象以进行绑定。
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 图形管线状态对象：持有 InputLayout、VS、PS 三个 GPU 对象。final 表示最终类。
	class GraphicsPipelineState final: public GraphicsResource
	{
	public:
		// 构造函数：依据 layout 提供的数据创建 InputLayout/VS/PS。实现见 .cpp。
		GraphicsPipelineState(const GraphicsPipelineStateDesc& desc, const GraphicsResourceDesc& gDesc);
	private:
		// 下面三个成员是真正的 GPU 对象（COM 智能指针管理）：
		//   m_vs  顶点着色器；m_ps  像素着色器；m_layout  输入布局（顶点缓冲与着色器输入之间的映射）。
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs{};
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps{};
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout{};
		// 友元：让 DeviceContext 能访问上述私有对象，以便在绘制时把它们绑定到管线。
		friend class DeviceContext;
	};
}

