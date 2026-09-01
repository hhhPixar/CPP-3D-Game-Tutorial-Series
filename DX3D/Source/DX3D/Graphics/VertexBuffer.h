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
// VertexBuffer.h —— 顶点缓冲（Vertex Buffer）
// 所属子系统：Direct3D 11 图形渲染 / GPU 缓冲资源。
// 职责：把一整块顶点数据（每个顶点含位置 position、纹理坐标 uv、法线 normal 等）
//       上传到 GPU 显存，供"输入装配阶段"（IA, Input Assembler）读取后喂给顶点着色器。
// 架构位置：继承自 GraphicsResource（持有 D3D11 device 等公共依赖），由
//           GraphicsDevice::createVertexBuffer 创建，由 DeviceContext 绑定到管线。
// 关键概念：创建 buffer 时用 D3D11_BIND_VERTEX_BUFFER 标记用途，GPU 据此知道
//           这是一块"顶点数据"；构造时一次性把顶点数据拷进显存，之后通常只读。
// ============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 顶点缓冲：CPU 端顶点数组 → GPU 显存的一块 buffer。每个 mesh 通常对应一个。
	// final 表示不允许再被继承。绘制时由 DeviceContext::setVertexBuffer 绑定、
	// 由 drawIndexedTriangleList 等方法实际使用其中的数据。
	class VertexBuffer final: public GraphicsResource
	{
	public:
		// 构造：依 desc（顶点数组指针 vertexList、顶点数 vertexListSize、单顶点字节数 vertexSize）
		// 创建 GPU buffer 并填入数据；gDesc 提供 device 等公共依赖。
		VertexBuffer(const VertexBufferDesc& desc, const GraphicsResourceDesc& gDesc);
		// 返回顶点数量，供绘制时确定要画多少个顶点。noexcept：承诺不抛异常。
		ui32 getVertexListSize() const noexcept;
	private:
		// 底层 D3D11 buffer 对象；ComPtr 是 COM 智能指针，析构时自动 Release，无需手动释放。
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer{};
		// 单顶点字节数，决定输入装配的 stride（IA 每次读取的跨度），绑定管线时要用。
		ui32 m_vertexSize{};
		// 顶点总数，用于绘制计数，getVertexListSize 返回它。
		ui32 m_vertexListSize{};

		// 友元：DeviceContext 需直接访问私有 m_buffer / m_vertexSize 来把它们绑定到管线，
		//       所以授予它私有访问权限。
		friend class DeviceContext;
	};
}

