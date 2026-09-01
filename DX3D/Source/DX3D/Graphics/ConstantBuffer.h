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
// ConstantBuffer.h —— 常量缓冲（Constant Buffer / cbuffer）
// 所属子系统：Direct3D 11 图形渲染 / GPU 缓冲资源。
// 职责：存"全局着色器常量"——相机矩阵、物体世界矩阵、光照参数等。CPU 每帧写入，
//       GPU 顶点/像素着色器读取。是 CPU 与 GPU 之间传递小批量参数数据的通道。
// 架构位置：继承自 GraphicsResource；由 GraphicsDevice::createConstantBuffer 创建，
//           由 DeviceContext::setConstantBuffers 绑定到着色器、由 updateConstantBuffer 写入。
// 关键概念：用 D3D11_USAGE_DYNAMIC + D3D11_CPU_ACCESS_WRITE 让 CPU 可写；每帧用
//           Map(D3D11_MAP_WRITE_DISCARD) 写入——"丢弃旧内容"避免 CPU 等 GPU 的同步停顿。
//           D3D11 要求常量缓冲大小是 16 字节的整数倍。
// ============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 常量缓冲：CPU 每帧写、GPU 着色器读的"参数包"（相机矩阵、世界矩阵、光照等）。
	// 一个常量缓冲通常对应一组着色器常量（shader 里的一个 cbuffer）。final 表示不可再被继承。
	class ConstantBuffer final: public GraphicsResource
	{
	public:
		// 构造：依 desc（buffer 初始数据指针、bufferSize 字节数）创建动态常量缓冲；
		// gDesc 提供 device 等公共依赖。bufferSize 应为 16 的整数倍（D3D11 要求）。
		ConstantBuffer(const ConstantBufferDesc& desc,const GraphicsResourceDesc& gDesc);
	private:
		// 底层 D3D11 buffer（ComPtr 智能指针自动释放 COM 对象）。
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer{};
		// 缓冲字节数；updateConstantBuffer 写入时会按此大小截断，防止越界。
		ui32 m_size{};
		// 友元：DeviceContext 需直接访问 m_buffer / m_size 来绑定与更新，故授予私有访问权限。
		friend class DeviceContext;
	};

}

