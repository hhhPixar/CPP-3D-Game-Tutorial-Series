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

// 文件：ShaderBinary.h
// 子系统：Graphics（图形）— 着色器部分
// 职责：声明 ShaderBinary 类，封装'编译后的着色器字节码'。
// 核心概念——着色器字节码（shader bytecode）：
//   GPU 无法直接执行 HLSL 文本，需要先用 D3DCompile 把 HLSL 源码编译成
//   二进制字节码，存放在 ID3DBlob（DirectX 通用数据块）里。之后字节码会被
//   传给 CreateVertexShader / CreatePixelShader 创建真正的 GPU 着色器对象
//   （那一步由 GraphicsPipelineState 完成）。本类只负责'持有字节码并对外
//   提供数据'，不做反射、不创建 GPU 对象。
// 上下游：由 GraphicsDevice::compileShader 创建 → 交给 GraphicsPipelineLayout
//   做反射（提取输入签名与资源槽位）→ 交给 GraphicsPipelineState 创建 GPU 对象。
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 着色器字节码资源。final 表示'最终类'，不可被继承。
	// 继承自 GraphicsResource，从而能访问 ID3D11Device 等底层 D3D 对象与日志。
	class ShaderBinary final: public GraphicsResource
	{
	public:
		// 构造函数：依据 ShaderCompileDesc（HLSL 源码、入口函数名、着色器类型）
		//   调用 D3DCompile 编译出字节码。实现见 ShaderBinary.cpp。
		ShaderBinary(const ShaderCompileDesc& desc, const GraphicsResourceDesc& gDesc);
		// 返回编译后的字节码（数据指针 + 字节数），供创建 GPU 着色器或做反射使用。
		BinaryData getData() const noexcept;
		// 返回本字节码对应的着色器类型：顶点着色器(VertexShader) 或 像素着色器(PixelShader)。
		ShaderType getType() const noexcept;
	private:
		// ID3DBlob：DirectX 通用二进制数据块接口，保存'编译后的着色器字节码'。
		// ComPtr 是 COM 智能指针，自动管理引用计数，析构时自动调用 Release()。
		Microsoft::WRL::ComPtr<ID3DBlob> m_blob{};
		// 记录这份字节码是顶点着色器还是像素着色器（见 ShaderType 枚举）。
		ShaderType m_type{};
	};
}

