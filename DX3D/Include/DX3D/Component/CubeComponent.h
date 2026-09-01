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
// 立方体组件（CubeComponent）—— 程序化生成的 3D 立方体
// ===================================================================
// 【所属子系统】Component 组件子系统，可挂到 GameObject 上让它显示成一个立方体。
// 【职责】在构造时用代码（而非外部文件）生成一个单位立方体的顶点和索引数据，
//         上传到显卡成为 VertexBuffer/IndexBuffer，并允许挂一个材质（Material）。
// 【架构位置】WorldRenderer 渲染时按 typeid 查询所有 CubeComponent，
//             用其顶点/索引缓冲和材质来绘制。相当于一个"内置基本几何体"。
// 【与 MeshComponent 的区别】Cube 是引擎内置的简单几何体，数据写死在代码里；
//   Mesh 是从 .obj 文件加载的任意网格。两者都继承 Component，都按类型被渲染器收集。
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>

namespace dx3d
{
	// 立方体组件：构造时即生成好顶点/索引缓冲（只生成一次），后续只读不重生成。
	class CubeComponent final : public Component
	{
		// dx3d_typeid 宏：为 CubeComponent 生成唯一类型 id，
		// 让 WorldRenderer 能按类型 world.getComponents<CubeComponent>() 查到它。
		dx3d_typeid(CubeComponent)
	public:
		// 构造函数：在此函数内一次性生成立方体的 24 顶点 + 36 索引并创建 GPU 缓冲。
		explicit CubeComponent(const ComponentDesc& data);

		// 设置/获取材质（Material）。材质含纹理、着色器管线等，决定立方体怎么被画。
		void setMaterial(const RefPtr<MaterialResource>& material);
		MaterialResource* getMaterial();

		// 提供给渲染器使用的顶点缓冲和索引缓冲（GPU 资源，引用返回避免拷贝）。
		// 顶点缓冲存顶点数据（位置+UV+法线），索引缓冲存三角形如何拼装。
		VertexBuffer& getVertexBuffer();
		IndexBuffer& getIndexBuffer();

	private:
		// 材质资源（可空，为空则不渲染该立方体）。
		RefPtr<MaterialResource> m_material{};
		// 顶点缓冲与索引缓冲：均为 GPU 资源，用引用计数智能指针 RefPtr 管理。
		RefPtr<VertexBuffer> m_vb{};
		RefPtr<IndexBuffer> m_ib{};
	};
}
