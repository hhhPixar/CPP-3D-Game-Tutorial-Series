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
// 网格组件（MeshComponent）—— 加载并渲染任意 3D 模型
// ===================================================================
// 【所属子系统】Component 组件子系统，挂到 GameObject 上让它显示成任意网格模型。
// 【职责】持有一个 MeshResource（从 .obj 文件加载的顶点/索引数据），
//         并支持"多材质槽位"——一个模型的不同部分（如车身、车窗）可用不同材质。
// 【与 CubeComponent 的区别】Cube 数据写死在代码里、只一个材质；
//   Mesh 从外部文件加载，且支持多个材质槽位（一个 mesh 可拆成多段，各段独立材质）。
// 【架构位置】WorldRenderer 按 typeid 查询所有 MeshComponent，遍历其材质槽位逐段绘制。
//   材质槽位（MaterialSlot）由 MeshResource 提供，记录每段的起始索引和索引数量。
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>
#include <vector>

namespace dx3d
{
	// 网格组件：final 表示不可再继承。负责"持有什么网格 + 用什么材质(s)"。
	class MeshComponent final : public Component
	{
		// 类型 id 注册，World/WorldRenderer 用它按类型查询所有 Mesh 组件。
		dx3d_typeid(MeshComponent)
	public:
		explicit MeshComponent(const ComponentDesc& data);

		// 设置网格资源（来自 .obj 等）。设入后自动按网格的材质槽数量调整 m_materials 大小：
		// 即"网格有几段，就准备几个材质位"。传入空指针则清空材质列表。
		void setMesh(const RefPtr<MeshResource>& mesh);
		// 返回网格资源指针（可能为空，未 setMesh 时为空）。
		MeshResource* getMesh() const noexcept;

		// 给指定槽位设置材质。index 越界会记日志并返回，不崩溃。
		// 一个网格的多段可以挂不同材质，渲染器按槽位逐段绘制。
		void setMaterial(ui32 index, const RefPtr<MaterialResource>& material);
		// 取指定槽位的材质（可能为空，渲染器遇到空材质会跳过该段不画）。
		MaterialResource* getMaterial(ui32 index) const noexcept;
	private:
		// 网格资源（顶点/索引缓冲 + 材质槽位信息），引用计数管理。
		RefPtr<MeshResource> m_mesh{};
		// 材质数组：长度等于网格的材质槽数量，每项对应一段的材质。可含空项。
		std::vector<RefPtr<MaterialResource>> m_materials{};
	};

}