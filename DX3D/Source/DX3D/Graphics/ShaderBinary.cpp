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

// 文件：ShaderBinary.cpp
// 子系统：Graphics — 着色器部分
// 职责：实现 ShaderBinary 类——把 HLSL 源码编译成着色器字节码。
// 编译流程：
//   1) 校验 ShaderCompileDesc 字段（源码、入口名等不能为空，否则抛异常）。
//   2) 设置编译选项（Debug 构建加 D3DCOMPILE_DEBUG，便于着色器调试）。
//   3) 准备 ShaderInclude，让 HLSL 的 #include 能找到 Common.hlsl 等公共头。
//   4) 调用 D3DCompile 编译：成功则字节码写入 m_blob；失败则从 errorBlob 取错误信息。
// 编译得到的字节码后续会传给 CreateVertexShader / CreatePixelShader 创建 GPU 对象。
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsUtils.h>
#include <d3dcompiler.h>
#include <DX3D/Graphics/ShaderInclude.h>

// 构造函数：编译一份着色器。
// desc  ：含 HLSL 源码、源文件名、入口函数名（如 VSMain）、着色器类型。
// gDesc：图形资源公共描述（提供 ID3D11Device、日志等）。
// 成员初始化列表先把基类 GraphicsResource 与 m_type 初始化好，函数体再调用 D3DCompile。
dx3d::ShaderBinary::ShaderBinary(const ShaderCompileDesc& desc, const GraphicsResourceDesc& gDesc): 
	GraphicsResource(gDesc), m_type(desc.shaderType)
{
	// 参数校验：任一必要字段为空则抛出 std::invalid_argument 类异常。
	if (!desc.shaderSourceName) DX3DLogThrowInvalidArg("No shader source name provided.");
	if (!desc.shaderSourceCode) DX3DLogThrowInvalidArg("No shader source code provided.");
	if (!desc.shaderSourceCodeSize) DX3DLogThrowInvalidArg("No shader source code size provided.");
	if (!desc.shaderEntryPoint) DX3DLogThrowInvalidArg("No shader entry point provided.");

	// 编译选项位掩码（UINT），默认 0。Debug 构建下加入 D3DCOMPILE_DEBUG，
	// 把行号/符号等调试信息编进字节码，便于 PIX/调试器定位着色器代码。
	UINT compileFlags{};

#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
#endif

	// 着色器 #include 处理器：实现 ID3DInclude 接口；D3DCompile 遇到 #include 时
	// 回调它读取被包含的文件（如 Common.hlsl）。此处为栈上局部对象，编译期间使用。
	ShaderInclude shaderInclude{};

	// errorBlob：编译失败时存放错误/警告文本，成功时为空（ComPtr 自动管理生命周期）。
	// 下面 D3DCompile 把 HLSL 文本编译成字节码。参数依次为：源码指针、源码字节数、
	//   源文件名（用于错误定位与 #include 的相对路径）、宏定义(nullptr)、include 处理器、
	//   入口函数名、编译目标（如 'vs_5_0' / 'ps_5_0'，由 GetShaderModelTarget 按类型给出）、
	//   编译选项、次级选项(0)、输出字节码(&m_blob)、错误信息(&errorBlob)。
	// DX3DGraphicsCheckShaderCompile：检查返回值 HRESULT，失败时把 errorBlob 文本写日志并抛异常。
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob{};
	DX3DGraphicsCheckShaderCompile(
		D3DCompile(
			desc.shaderSourceCode,
			desc.shaderSourceCodeSize,
			desc.shaderSourceName,
			nullptr,
			&shaderInclude,
			desc.shaderEntryPoint,
			dx3d::GraphicsUtils::GetShaderModelTarget(desc.shaderType),
			compileFlags,
			0,
			&m_blob,
			&errorBlob
		),
		errorBlob.Get()
	);
}

// 返回编译后的字节码（只读数据指针 + 字节数）。
// 调用方用这些数据去创建 GPU 着色器对象，或做反射（D3DReflect）。
dx3d::BinaryData dx3d::ShaderBinary::getData() const noexcept
{
	return
	{
		m_blob->GetBufferPointer(),
		m_blob->GetBufferSize()
	};
}

// 返回着色器类型，供调用方判断这是顶点着色器还是像素着色器。
dx3d::ShaderType dx3d::ShaderBinary::getType() const noexcept
{
	return m_type;
}
