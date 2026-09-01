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
// MeshComponent 实现：网格资源与多材质槽位的管理
// ===================================================================
// 【关键概念】
//  - 材质槽位（material slot）：一个网格可拆成多段（如 .obj 里多个 group），
//    每段有 startIndex（起始索引）和 indexCount（索引数），可挂不同材质。
//    WorldRenderer 渲染时遍历每个槽位，取其材质和索引范围逐段绘制。
//  - m_materials 数组长度由网格的槽数决定：setMesh 时调用 resize 对齐。
//  - 越界保护：setMaterial/getMaterial 都检查 index，越界记日志而非崩溃。
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/World.h>
#include <DX3D/Game/Game.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/MeshResource.h>


// 构造函数：仅初始化基类，网格和材质均为空。实际网格靠 setMesh 注入。
dx3d::MeshComponent::MeshComponent(const ComponentDesc& data) : Component(data)
{
}

// 设置网格资源，并按网格的材质槽数量调整材质数组。
// 有网格时 resize 到槽数（预备好每个槽位的材质位，初始为空）；传空则清空。
// 这样保证 m_materials.size() 始终与当前网格的段数一致，避免 setMaterial 越界。
void dx3d::MeshComponent::setMesh(const RefPtr<MeshResource>& mesh)
{
	m_mesh = mesh;
	if (m_mesh) m_materials.resize(m_mesh->getNumMaterialSlots());
	else m_materials.resize(0);
}

// 返回网格资源原始指针（可能为空）。调用方据此判断是否有网格可画。
dx3d::MeshResource* dx3d::MeshComponent::getMesh() const noexcept
{
	return m_mesh.get();
}

// 给第 index 个槽位设置材质。越界检查：若 index 超出当前 m_materials 大小，
// 记错误日志并返回（提示调用方先 setMesh 再 setMaterial），不修改任何数据。
void dx3d::MeshComponent::setMaterial(dx3d::ui32 index, const RefPtr<MaterialResource>& material)
{
	if (index >= m_materials.size())
	{
		DX3DLogError("Index {} is out of bounds for the materials list (size: {}). Ensure setMesh() has been called before setting materials.", index, m_materials.size());
		return;
	}

	m_materials[index] = material;
}

// 取第 index 个槽位的材质。越界返回空指针并记日志。
// 返回空表示该段无材质，WorldRenderer 会跳过该段不绘制（continue）。
dx3d::MaterialResource* dx3d::MeshComponent::getMaterial(ui32 index) const noexcept
{
	if (index >= m_materials.size())
	{
		DX3DLogError("Index {} is out of bounds for the materials list (size: {}).", index, m_materials.size());
		return {};
	}
	return m_materials[index].get();
}
