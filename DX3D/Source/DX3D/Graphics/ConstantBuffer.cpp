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

// ============================================================================
// ConstantBuffer.cpp —— ConstantBuffer 的实现
// 调用 ID3D11Device::CreateBuffer 创建常量缓冲。关键三件套：USAGE_DYNAMIC（CPU 可写）、
// BIND_CONSTANT_BUFFER（当 cbuffer 用）、CPU_ACCESS_WRITE（允许 CPU 写显存）。
// 之后每帧由 DeviceContext::updateConstantBuffer 用 Map+WriteDiscard 写入新数据。
// ============================================================================
#include <DX3D/Graphics/ConstantBuffer.h>

// 构造函数实现：创建动态常量缓冲。desc 含 bufferSize（字节数）与可选初值 buffer。
// gDesc 提供 device 等公共依赖；m_size 缓存字节数供 updateConstantBuffer 截断判断。
dx3d::ConstantBuffer::ConstantBuffer(const ConstantBufferDesc& desc, const GraphicsResourceDesc& gDesc): 
	GraphicsResource(gDesc), m_size(desc.bufferSize)
{
	// 参数校验：bufferSize 为 0 就抛异常（带日志）。常量缓冲必须有非零大小。
	if (!desc.bufferSize) DX3DLogThrowInvalidArg("Buffer size must be non-zero.");

	// 填写 D3D11 缓冲描述 D3D11_BUFFER_DESC：常量缓冲的关键三项——动态用法、常量绑定、CPU 可写。
	D3D11_BUFFER_DESC buffDesc{};
	// D3D11_USAGE_DYNAMIC：动态用法，允许 CPU 写入（GPU 仍可读），适合每帧更新的数据。
	buffDesc.Usage = D3D11_USAGE_DYNAMIC;
	// 缓冲字节数（应 16 字节对齐）。常量缓冲按 16 字节寄存器组织，故大小需 16 的倍数。
	buffDesc.ByteWidth = desc.bufferSize;
	// D3D11_BIND_CONSTANT_BUFFER：标记为常量缓冲，GPU 着色器按 cbuffer 读取它。
	buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// D3D11_CPU_ACCESS_WRITE：允许 CPU 写入这块 GPU 显存（配合 USAGE_DYNAMIC 使用）。
	buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// 初始数据：可选。若 desc.buffer 非空则创建时即填入初值；否则稍后由
	// DeviceContext::updateConstantBuffer 每帧用 Map + WriteDiscard 写入。
	D3D11_SUBRESOURCE_DATA initData{};
	// pSysMem 指向 CPU 端初始数据作为拷贝源（可能为 nullptr）。
	initData.pSysMem = desc.buffer;

	// 真正创建 GPU buffer：CreateBuffer 按 buffDesc 分配显存；第二个参数为初值指针，
	// desc.buffer 为空时传 nullptr 表示暂不填初值。输出 m_buffer。失败则记日志并抛异常。
	DX3DGraphicsLogThrowOnFail(m_device.CreateBuffer(&buffDesc, (desc.buffer)?&initData:nullptr, &m_buffer),
		"CreateBuffer failed.");
}
