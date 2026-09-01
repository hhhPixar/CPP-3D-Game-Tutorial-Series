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
// IndexBuffer.cpp —— IndexBuffer 的实现
// 调用 ID3D11Device::CreateBuffer 创建索引缓冲，用 D3D11_SUBRESOURCE_DATA 把
// CPU 端索引数组一次性拷进 GPU。buffer 带 D3D11_BIND_INDEX_BUFFER 标志。
// 每个索引是 32 位无符号整数（ui32），绑定到管线时格式为 DXGI_FORMAT_R32_UINT。
// ============================================================================
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/GraphicsUtils.h>

// 构造函数实现：把 desc 里的索引数据上传到 GPU 创建索引缓冲。
// 参数 desc：含 indexList（索引数组指针）、indexListSize（索引数）。
// 参数 gDesc：提供 device 等公共依赖；同时把 indexListSize 缓存到 m_listSize 供查询。
dx3d::IndexBuffer::IndexBuffer(const IndexBufferDesc& desc, const GraphicsResourceDesc& gDesc) : GraphicsResource(gDesc), m_listSize(desc.indexListSize)
{
	// 参数校验：索引数组指针 / 索引数为空或 0 就抛异常（带日志）。
	if (!desc.indexList) DX3DLogThrowInvalidArg("No index list provided.");
	if (!desc.indexListSize) DX3DLogThrowInvalidArg("Index list size must be non-zero.");

	// 填写 D3D11 缓冲描述 D3D11_BUFFER_DESC：创建索引 buffer 前的"规格表"。
	D3D11_BUFFER_DESC buffDesc{};
	// 总字节数 = 索引数 × sizeof(ui32)：每个索引是一个 32 位无符号整数（4 字节）。
	buffDesc.ByteWidth = desc.indexListSize * sizeof(ui32);
	// D3D11_BIND_INDEX_BUFFER 标记此 buffer 专用作索引缓冲。
	buffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// 初始数据 D3D11_SUBRESOURCE_DATA：让 CreateBuffer 创建时就把 CPU 端索引数组拷进 GPU。
	D3D11_SUBRESOURCE_DATA initData{};
	// pSysMem 指向 CPU 端索引数组作为拷贝源。
	initData.pSysMem = desc.indexList;

	// 真正创建 GPU buffer：m_device.CreateBuffer 分配显存并用 initData 填充，输出 m_buffer。
	// 失败时 DX3DGraphicsLogThrowOnFail 记日志并抛异常。
	DX3DGraphicsLogThrowOnFail(m_device.CreateBuffer(&buffDesc, &initData, &m_buffer),
		"CreateIndexBuffer failed.");
}

// 返回索引数量（构造时缓存），供 drawIndexedTriangleList 确定索引计数。noexcept：不抛异常。
dx3d::ui32 dx3d::IndexBuffer::getIndexListSize() const noexcept
{
	return m_listSize;
}


