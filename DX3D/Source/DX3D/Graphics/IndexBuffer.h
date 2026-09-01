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
// IndexBuffer.h —— 索引缓冲（Index Buffer）
// 所属子系统：Direct3D 11 图形渲染 / GPU 缓冲资源。
// 职责：存"顶点索引"数组（每个索引是一个 32 位无符号数 ui32），告诉 GPU
//       "用顶点缓冲里第几个顶点"来拼三角形，从而避免共享顶点被重复存储。
//       配合 DeviceContext::drawIndexedTriangleList 使用。
// 架构位置：继承自 GraphicsResource；由 GraphicsDevice::createIndexBuffer 创建，
//           由 DeviceContext::setIndexBuffer 绑定（格式 DXGI_FORMAT_R32_UINT）。
// 关键概念：D3D11_BIND_INDEX_BUFFER 标记为索引缓冲；索引比直接列顶点更省显存，
//           例如立方体只需 8 个顶点 + 36 个索引，而非 36 个重复顶点。
// ============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 索引缓冲：存"顶点索引"数组，告诉 GPU 用哪些顶点拼三角形，避免顶点重复存储。
	// 与顶点缓冲配合：顶点缓冲给"顶点数据"，索引缓冲给"怎么拼成三角形"。
	// final 表示不允许再被继承。
	class IndexBuffer final : public GraphicsResource
	{
	public:
		// 构造：依 desc（indexList 索引数组指针、indexListSize 索引数）创建 GPU buffer 并填入数据；
		// gDesc 提供 device 等公共依赖。
		IndexBuffer(const IndexBufferDesc& desc, const GraphicsResourceDesc& gDesc);
		// 返回索引数量，供 drawIndexedTriangleList 确定要画多少个索引。noexcept：不抛异常。
		ui32 getIndexListSize() const noexcept;
	private:
		// 底层 D3D11 buffer（ComPtr 智能指针自动释放 COM 对象）。
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer{};
		// 索引数量（每索引一个 ui32），供绘制时确定索引计数。
		ui32 m_listSize{};

		// 友元：DeviceContext 需直接访问私有 m_buffer 把它绑定到管线，故授予私有访问权限。
		friend class DeviceContext;
	};
}
