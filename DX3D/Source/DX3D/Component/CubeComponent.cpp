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

// ===================================================================
// CubeComponent 实现：程序化生成立方体顶点/索引缓冲
// ===================================================================
// 【关键概念】
//  - 立方体有 6 个面，每面是 2 个三角形。但为什么是 24 顶点而非 8？
//    因为同一角点在不同面有不同的法线（和 UV），所以每面用独立 4 顶点，
//    6 面 × 4 = 24 顶点；每面 2 三角形 × 3 顶点 = 6 索引，6 面 × 6 = 36 索引。
//  - 每个顶点含三组数据：position（位置）、texcoord（UV 纹理坐标）、normal（法线）。
//    UV 用 {0..1} 表示纹理上从一角到对角的覆盖范围；法线决定该面朝哪（用于光照）。
//  - 缓冲用 static 修饰，意味着整个程序生命周期只创建一次 GPU 资源，所有立方体共享，
//    极大节省显存（无数立方体共用同一份顶点/索引数据）。
#include <DX3D/Component/CubeComponent.h>
#include <DX3D/Game/World.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec2.h>
#include <DX3D/Math/Mat4x4.h>
#include <DX3D/Graphics/GraphicsDevice.h>


// 构造函数：在这里写死立方体的几何数据，并上传到 GPU。
dx3d::CubeComponent::CubeComponent(const ComponentDesc& data) : Component(data)
{
	// 24 个顶点：每个 {position, texcoord, normal}。
	// 立方体边长为 1（坐标范围 -0.5 ~ 0.5），原点在中心。
	// 每个面 4 顶点独立，使其拥有各自正确的法线和 UV（不与相邻面共享）。
	static const MeshVertex vertexList[] =
	{
		// Front (+Z)
		{{-0.5f, -0.5f,  0.5f}, {0, 1},{0,0,1}},
		{{-0.5f,  0.5f,  0.5f}, {0, 0},{0,0,1}},
		{{ 0.5f,  0.5f,  0.5f}, {1, 0},{0,0,1}},
		{{ 0.5f, -0.5f,  0.5f}, {1, 1},{0,0,1}},

		// Back (-Z)
		{{ 0.5f, -0.5f, -0.5f}, {0, 1},{0,0,-1}},
		{{ 0.5f,  0.5f, -0.5f}, {0, 0},{0,0,-1}},
		{{-0.5f,  0.5f, -0.5f}, {1, 0},{0,0,-1}},
		{{-0.5f, -0.5f, -0.5f}, {1, 1},{0,0,-1}},

		// Left (-X)
		{{-0.5f, -0.5f, -0.5f}, {0, 1},{-1,0,0}},
		{{-0.5f,  0.5f, -0.5f}, {0, 0},{-1,0,0}},
		{{-0.5f,  0.5f,  0.5f}, {1, 0},{-1,0,0}},
		{{-0.5f, -0.5f,  0.5f}, {1, 1},{-1,0,0}},

		// Right (+X)
		{{ 0.5f, -0.5f,  0.5f}, {0, 1},{1,0,0}},
		{{ 0.5f,  0.5f,  0.5f}, {0, 0},{1,0,0}},
		{{ 0.5f,  0.5f, -0.5f}, {1, 0},{1,0,0}},
		{{ 0.5f, -0.5f, -0.5f}, {1, 1},{1,0,0}},

		// Top (+Y)
		{{-0.5f,  0.5f,  0.5f}, {0, 1},{0,1,0}},
		{{-0.5f,  0.5f, -0.5f}, {0, 0},{0,1,0}},
		{{ 0.5f,  0.5f, -0.5f}, {1, 0},{0,1,0}},
		{{ 0.5f,  0.5f,  0.5f}, {1, 1},{0,1,0}},

		// Bottom (-Y)
		{{-0.5f, -0.5f, -0.5f}, {0, 1},{0,-1,0}},
		{{-0.5f, -0.5f,  0.5f}, {0, 0},{0,-1,0}},
		{{ 0.5f, -0.5f,  0.5f}, {1, 0},{0,-1,0}},
		{{ 0.5f, -0.5f, -0.5f}, {1, 1},{0,-1,0}},
	};

	// 36 个索引：每 3 个索引组成一个三角形，每 6 个组成一个面的两个三角形。
	// 索引引用 vertexList 里的顶点，避免重复存储顶点数据。
	// 顶点顺序（0,2,1 / 0,3,2）保证从面外看是逆时针环绕（背面剔除的标准朝向）。
	static const ui32 indexList[] =
	{
		 0,  2,  1,   0,  3,  2,   // Front
		 4,  6,  5,   4,  7,  6,   // Back
		 8, 10,  9,   8, 11, 10,   // Left
		12, 14, 13,  12, 15, 14,   // Right
		16, 18, 17,  16, 19, 18,   // Top
		20, 22, 21,  20, 23, 22    // Bottom
	};

	// 通过 GraphicsDevice 创建 GPU 顶点缓冲/索引缓冲（static 保证只创建一次）。
	// 第三个参数 sizeof(MeshVertex) 是每个顶点的步长（stride），GPU 据此切分数据。
	static const auto vb = m_context.device.createVertexBuffer({ vertexList, std::size(vertexList), sizeof(MeshVertex)});
	static const auto ib = m_context.device.createIndexBuffer({ indexList, std::size(indexList) });

	// 用引用计数智能指针持有 GPU 缓冲，多个立方体共享同一份资源。
	m_vb = vb;
	m_ib = ib;
}

// 设置材质（纹理+着色器等）。材质为空时渲染器会跳过该立方体不画。
void dx3d::CubeComponent::setMaterial(const RefPtr<MaterialResource>& material)
{
	m_material = material;
}

// 返回原始材质指针（可能为 nullptr，调用方需判空）。
dx3d::MaterialResource* dx3d::CubeComponent::getMaterial()
{
	return m_material.get();
}

// 提供给渲染器的顶点缓冲（解引用 RefPtr 得到引用）。
dx3d::VertexBuffer& dx3d::CubeComponent::getVertexBuffer()
{
	return *m_vb;
}

// 提供给渲染器的索引缓冲。
dx3d::IndexBuffer& dx3d::CubeComponent::getIndexBuffer()
{
	return *m_ib;
}
