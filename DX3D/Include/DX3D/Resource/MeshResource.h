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
// 网格资源（MeshResource）—— 把一个 .obj 模型文件加载成可绘制的网格
// 职责：用第三方库 tinyobjloader 解析 .obj 文件，得到
//   · 顶点数组（MeshVertex：位置 position / 纹理坐标 texcoord / 法线 normal）
//   · 索引数组（IndexBuffer，指示如何把顶点连成三角形）
//   · 材质槽位列表（MaterialSlot：每段连续索引对应一个材质，便于按材质分多次绘制）
// 并在 GPU 上创建对应的 VertexBuffer / IndexBuffer。
// 架构位置：资源系统，派生自 Resource；由 ResourceManager 在 .obj 扩展名时创建并缓存。
// 关键概念：一个网格可能含多种材质，每种材质覆盖若干三角形，故拆成多个 MaterialSlot。
// ============================================================================
#pragma once
#include <DX3D/Resource/Resource.h>
#include <vector>

namespace dx3d
{
	// 网格资源类：final 表示不可再被继承。持有一个顶点缓冲、一个索引缓冲、若干材质槽位。
	class MeshResource final : public Resource
	{
	public:
		// 构造函数：解析 .obj 文件并创建 GPU 顶点/索引缓冲与材质槽位。详见 .cpp。
		explicit MeshResource(const MeshResourceDesc& desc);
		// 取所有材质槽位。输出参数 numSlots 返回槽位数量；返回指向内部数组的指针（不持有所有权）。
		const MaterialSlot * getMaterialSlots(ui32& numSlots) const noexcept;

		// 返回材质槽位数量（即这个网格需要分多少段、按多少种材质来绘制）。
		ui32 getNumMaterialSlots() const noexcept;
		// 取 GPU 顶点缓冲（包含全部顶点的位置/纹理坐标/法线）。
		const VertexBuffer& getVertexBuffer() const noexcept;
		// 取 GPU 索引缓冲（指示顶点如何组成三角形，按材质槽位分段）。
		const IndexBuffer& getIndexBuffer() const noexcept;
	private:
		// 顶点缓冲（GPU 资源，用 shared_ptr 共享生命周期）。
		RefPtr<VertexBuffer> m_vertexBuffer{};
		// 索引缓冲（GPU 资源）。
		RefPtr<IndexBuffer> m_indexBuffer{};
		// 材质槽位列表：每个槽位描述一段连续索引范围及其对应的材质编号。
		std::vector<MaterialSlot> m_matSlots{};
	};
}