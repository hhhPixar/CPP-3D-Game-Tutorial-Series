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

// 文件：ShaderInclude.h
// 子系统：Graphics — 着色器部分
// 职责：实现 ID3DInclude 回调接口，让 HLSL 源码里的 #include 指令能找到
//   被包含的文件（例如公共头 Common.hlsl）。
// 核心概念——ID3DInclude：
//   D3DCompile 在编译 HLSL 时遇到 #include 会回调此接口的 Open() 去读取文件
//   内容，编译完成后调用 Close() 释放。本类用标准文件流读取磁盘文件。
// 使用方式：在 ShaderBinary.cpp 中作为栈上局部对象传给 D3DCompile 的 pInclude 参数。
// 注意：本文件不在 dx3d 命名空间内，是一个全局类。
#pragma once
#include <d3dcompiler.h>
#include <fstream>

// 着色器 #include 处理器。实现 Direct3D 的 ID3DInclude 回调接口。
// ID3DInclude 要求实现 Open（打开/读取被包含文件）与 Close（释放）两个方法。
class ShaderInclude : public ID3DInclude
{
public:
	ShaderInclude(){}
	// Open：D3DCompile 遇到 #include 时回调此方法。
	// IncludeType：包含类型（本地/系统）；pFileName：要包含的文件名；
	// pParentData：包含此 #include 的父文件数据（此处未用）；
	// ppData/pBytes：输出参数，返回文件内容的指针与字节数。
	// 成功返回 S_OK，失败返回 E_FAIL（如打不开文件）。
	virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName,
		LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		// 以输入流打开被包含的文件；打开失败则直接返回 E_FAIL。
		std::ifstream shaderStream(pFileName);
		if (!shaderStream) return E_FAIL;
		// 用 istreambuf_iterator 把整个文件内容读进 std::string（逐字符，保留原始内容）。
		std::string shaderCode{
			std::istreambuf_iterator<char>(shaderStream),
			std::istreambuf_iterator<char>()
		};
		// 在堆上分配一块缓冲区（+1 给结尾空字符），把文件内容拷贝过去。
		// 这块内存的所有权交给出参 ppData，之后由 Close() 负责释放。
		char* shaderCodePtr = new char[shaderCode.size() + 1];
		memcpy(shaderCodePtr, shaderCode.c_str(), shaderCode.size() + 1);
		*ppData = shaderCodePtr;
		*pBytes = static_cast<UINT>(shaderCode.size());
		return S_OK;
	}
	// Close：编译器用完 #include 的内容后回调此方法释放内存。
	// pData 就是当初 Open 里 new[] 出来的指针，这里用 delete[] 释放。
	virtual HRESULT Close(LPCVOID pData)
	{
		delete[] static_cast<const char*>(pData);
		return S_OK;
	}
};