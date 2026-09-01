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
// TransformComponent 实现：把 position/rotation/scale 编译成世界矩阵
// ===================================================================
// 【设计要点】
//  - 延迟更新（lazy evaluation）：setXxx 只置脏标记，不立即重算矩阵；
//    取矩阵时（getMatrix）才调用 updateWorldMatrix 真正重算。
//  - 批量刷新：markAsDirty 会把自己登记到 World::m_dirtyTransforms，
//    World::update 在所有 GameObject::onUpdate 跑完后统一重算脏组件，
//    保证用到的总是本帧最新结果（而不是旧数据被覆盖前的中间值）。
//  - 刚体 vs 仿射分离：rigidWorld 不含 scale，给法线/光照/相机方向用；
//    affineWorld 含 scale，给顶点位置用。详见头文件注释。
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Game/World.h>


// 构造函数：先调用基类 Component 构造（拿到 m_object/m_world/m_context），
// 再 markAsDirty 把自己置脏，确保第一次取矩阵时一定会重算（而非空白默认值）。
dx3d::TransformComponent::TransformComponent(const ComponentDesc& data) : Component(data)
{
	markAsDirty();
}

// 设置位置：保存新值后置脏。世界矩阵依赖 position，所以必须重算。
void dx3d::TransformComponent::setPosition(const Vec3& position)
{
	m_position = position;
	markAsDirty();
}

// 读取位置（直接返回成员，const noexcept 表示不改状态、不抛异常）。
dx3d::Vec3 dx3d::TransformComponent::getPosition() const noexcept
{
	return m_position;
}

// 设置旋转（欧拉角，弧度）。旋转变了，刚体矩阵和仿射矩阵都要重算。
void dx3d::TransformComponent::setRotation(const Vec3& rotation)
{
	m_rotation = rotation;
	markAsDirty();
}

dx3d::Vec3 dx3d::TransformComponent::getRotation() const noexcept
{
	return m_rotation;
}

// 设置缩放：只影响仿射矩阵（顶点位置），不影响刚体矩阵（法线/方向）。
// 但因为 markAsDirty 会让两者一起重算（它们在 updateWorldMatrix 里同时更新），
// 重算时 rigidWorld 本来也不含 scale，所以方向不受缩放影响。
void dx3d::TransformComponent::setScale(const Vec3& scale)
{
	m_scale = scale;
	markAsDirty();
}

dx3d::Vec3 dx3d::TransformComponent::getScale() const noexcept
{
	return m_scale;
}


// 返回"前方"朝向单位向量。
// 取刚体矩阵的第 3 行（row(2)），即变换后的 +Z 轴基向量，再归一化。
// 为什么用刚体矩阵？因为若有非均匀缩放，仿射矩阵会把轴方向拉歪，
// 而刚体矩阵不含缩放，方向保持正交，归一化后即为纯朝向。
// 矩阵的"行"对应坐标轴基向量：row(0)=右、row(1)=上、row(2)=前（左手系）。
dx3d::Vec3 dx3d::TransformComponent::forward()
{
	auto forward = getRigidWorldMatrix().row(2);
	return dx3d::Vec3::normalize({ forward.x,forward.y,forward.z});
}

// 返回"右方"朝向单位向量，取刚体矩阵第 1 行（变换后的 +X 轴），归一化。
dx3d::Vec3 dx3d::TransformComponent::right()
{
	auto right = getRigidWorldMatrix().row(0);
	return dx3d::Vec3::normalize({ right.x,right.y,right.z });
}

// 返回"上方"朝向单位向量，取刚体矩阵第 2 行（变换后的 +Y 轴），归一化。
dx3d::Vec3 dx3d::TransformComponent::up()
{
	auto up = getRigidWorldMatrix().row(1);
	return dx3d::Vec3::normalize({ up.x,up.y,up.z });
}

// 取仿射世界矩阵（含缩放），变换顶点位置用。内部会确保矩阵已刷新。
dx3d::Mat4x4 dx3d::TransformComponent::getAffineWorldMatrix() noexcept
{
	updateWorldMatrix();
	return m_affineWorldMatrix;
}

// 取刚体世界矩阵（不含缩放），变换法线/光照方向/相机视线用。
dx3d::Mat4x4 dx3d::TransformComponent::getRigidWorldMatrix() noexcept
{
	updateWorldMatrix();
	return m_rigidWorldMatrix;;
}



// 真正重算两个世界矩阵的地方。延迟更新的核心实现。
// 若 m_dirty 为 false 直接返回，避免每帧重算（典型性能优化）。
// 矩阵乘法顺序：rotateX * rotateY * rotateZ * translate。
// 注意矩阵乘法不满足交换律，这里的顺序对应"先绕X旋转、再绕Y、再绕Z、最后平移"。
// 刚体矩阵 = 旋转 * 平移；仿射矩阵 = 缩放 * 刚体矩阵（scale 作用在最外层）。
void dx3d::TransformComponent::updateWorldMatrix() noexcept
{
	if (!m_dirty) return;

	m_dirty = false;

	// 刚体世界矩阵：仅旋转+平移，不含缩放。给法线/方向用，保证方向不被拉伸。
	m_rigidWorldMatrix =
		Mat4x4::rotateX(m_rotation.x) *
		Mat4x4::rotateY(m_rotation.y) *
		Mat4x4::rotateZ(m_rotation.z) *
		Mat4x4::translate(m_position);

	// 仿射世界矩阵：在最外层乘上缩放，即 scale * (rotation * translation)。
	// 这样缩放作用在模型局部坐标上（先放大再旋转/平移），顶点位置变换用此矩阵。
	m_affineWorldMatrix =
		Mat4x4::scale(m_scale) *
		m_rigidWorldMatrix;
}


// 置脏标记：标记本组件的世界矩阵已过时、需要重算。
// 若已经脏了就直接返回（不重复登记，避免 World 列表里出现重复项）。
// 否则置脏并把自己加入 World 的待刷新列表，由 World::update 统一批量处理。
void dx3d::TransformComponent::markAsDirty()
{
	if (m_dirty) return;
	m_dirty = true;
	m_world.addDirtyTransformInternal(*this);
}
