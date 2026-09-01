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
// VertexBuffer.cpp —— VertexBuffer 的实现
// 这里真正调用 D3D11 的 ID3D11Device::CreateBuffer 创建一块 GPU buffer，
// 并用 D3D11_SUBRESOURCE_DATA 把 CPU 端的顶点数组作为初始数据一次性拷进去。
// 创建出来的 buffer 带 D3D11_BIND_VERTEX_BUFFER 标志，专门用作顶点缓冲。
// ============================================================================
#include <DX3D/Graphics/VertexBuffer.h>

// 构造函数实现：把 desc 里的顶点数据上传到 GPU 创建顶点缓冲。
// 参数 desc：含 vertexList（顶点数组指针）、vertexListSize（顶点数）、vertexSize（单顶点字节）。
// 参数 gDesc：提供 device 等公共依赖；同时把 vertexSize/vertexListSize 缓存到成员供查询。
dx3d::VertexBuffer::VertexBuffer(const VertexBufferDesc& desc, const GraphicsResourceDesc& gDesc): 
	GraphicsResource(gDesc), m_vertexSize(desc.vertexSize), m_vertexListSize(desc.vertexListSize)
{
	// 参数校验：顶点列表指针 / 顶点数 / 单顶点大小任一为空或 0 就抛异常（带日志）。
	if (!desc.vertexList) DX3DLogThrowInvalidArg("No vertex list provided.");
	if (!desc.vertexListSize) DX3DLogThrowInvalidArg("Vertex list size must be non-zero.");
	if (!desc.vertexSize) DX3DLogThrowInvalidArg("Vertex size must be non-zero.");

	// 填写 D3D11 缓冲描述 D3D11_BUFFER_DESC：创建 buffer 前必须准备的"规格表"，
	// 告诉 GPU 这块 buffer 多大、用来干什么。
	D3D11_BUFFER_DESC buffDesc{};
	// 总字节数 = 顶点数 × 单顶点字节：整块顶点数据在显存里占多大。
	buffDesc.ByteWidth = desc.vertexListSize * desc.vertexSize;
	// D3D11_BIND_VERTEX_BUFFER 标记此 buffer 专用作顶点缓冲，GPU 据此把它当顶点数据访问。
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// 初始数据 D3D11_SUBRESOURCE_DATA：让 CreateBuffer 在创建 buffer 的同时，
	// 把这块 CPU 内存（顶点数组）一次性拷进 GPU，构造完顶点数据就已就位。
	D3D11_SUBRESOURCE_DATA initData{};
	// pSysMem 指向 CPU 端顶点数组作为拷贝源；拷完后 GPU 不再使用 CPU 端这份内存。
	initData.pSysMem = desc.vertexList;

	// 真正创建 GPU buffer：m_device.CreateBuffer 按 buffDesc 分配显存、用 initData 填充，
	// 输出到 m_buffer（ID3D11Buffer）。DX3DGraphicsLogThrowOnFail 在 HRESULT 失败时记日志并抛异常。
	DX3DGraphicsLogThrowOnFail(
		m_device.CreateBuffer(&buffDesc, &initData, &m_buffer),
		"CreateBuffer failed.");
}

// 返回顶点数量（构造时缓存），供 DeviceContext 绘制时确定顶点计数。noexcept：不抛异常。
dx3d::ui32 dx3d::VertexBuffer::getVertexListSize() const noexcept
{
	return m_vertexListSize;
}
