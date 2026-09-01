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
// 变换组件（TransformComponent）—— 引擎中最核心、最常用的组件
// ===================================================================
// 【所属子系统】Component 组件子系统，可挂到任意 GameObject（游戏对象）上。
// 【职责】存储对象的位置 position、旋转 rotation、缩放 scale，并据此计算
//         两个"世界矩阵"（world matrix）：把对象从"模型局部坐标"变换到"世界坐标"。
// 【为何维护两个矩阵】
//   - m_affineWorldMatrix（仿射世界矩阵）：含 缩放+旋转+平移，用于变换"顶点位置"，
//     使物体在场景中被正确地放大/缩小/旋转/移动。
//   - m_rigidWorldMatrix（刚体世界矩阵）：只含 旋转+平移，不含缩放。
//     专用于"法线变换"和"光照方向"等场合，避免缩放把法线或方向向量拉歪。
//   例：若一个方块沿 X 轴放大 10 倍，顶点位置必须用仿射矩阵（这样它才真的变长）；
//   但其朝向（forward）和表面法线不能用带缩放的矩阵，否则方向会被扭曲。
// 【脏标记 m_dirty】延迟更新策略：改了 position/rotation/scale 不立刻重算矩阵，
//   只置脏标记，等真正调用 getMatrix 取矩阵时才重算，避免无谓重复计算。
//   通过 World::addDirtyTransformInternal 批量登记，由 World::update 统一刷新。
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Mat4x4.h>


namespace dx3d
{
	// 变换组件：每个 GameObject 都隐含持有一个，描述该对象在 3D 世界中的位姿。
	// 继承自 Component，可被 World 按类型统一管理（World 会按 typeid 归档所有组件）。
	class TransformComponent final : public Component
	{
		// dx3d_typeid 宏：为该类生成一个唯一的类型标识符（type id）。
		// 基于 typeid(Class).hash_code()，让 World/GameObject 能按类型查找组件
		// （例如 world.getComponents<TransformComponent>()）。这是 ECS 风格的类型注册。
		dx3d_typeid(TransformComponent)
	public:
		// 构造函数：data 里携带所属对象、World、GameContext 等引用。
		// 构造后立即 markAsDirty，保证首次取矩阵时一定会重算（初始矩阵不是过时数据）。
		explicit TransformComponent(const ComponentDesc& data);

		// 以下三组 setter/getter：设置 position/rotation/scale。
		// 三个 setter 都会调用 markAsDirty()，因为一旦位姿改变，缓存的世界矩阵就过时了。
		// rotation 用欧拉角（Euler angles，弧度）表示绕 X/Y/Z 三轴的旋转量。
		void setPosition(const Vec3& position);
		Vec3 getPosition() const noexcept;

		void setRotation(const Vec3& rotation);
		Vec3 getRotation() const noexcept;

		void setScale(const Vec3& scale);
		Vec3 getScale() const noexcept;

		// 朝向轴：返回该对象朝前的单位向量。
		// 注意它取的是"刚体矩阵"的第 3 行（row 2），再用 Vec3::normalize 归一化。
		// 因为刚体矩阵不含缩放，方向不会被扭曲；归一化是为了保证长度恰好为 1。
		Vec3 forward() ;
		// 朝向轴：返回该对象朝右的单位向量，取刚体矩阵第 1 行（row 0）。
		Vec3 right() ;
		// 朝向轴：返回该对象朝上的单位向量，取刚体矩阵第 2 行（row 1）。
		Vec3 up() ;

		// 取仿射世界矩阵（含缩放）：变换顶点位置时用，如 VertexShader 里 pos*world。
		Mat4x4 getAffineWorldMatrix() noexcept;
		// 取刚体世界矩阵（不含缩放）：变换法线、光照方向、相机视线方向时用。
		Mat4x4 getRigidWorldMatrix() noexcept;

		// 真正重算两个世界矩阵的函数。若 m_dirty 为 false 则直接返回（不重复计算）。
		// 这就是"延迟更新"的核心：脏了才重算，没脏就用缓存值。
		void updateWorldMatrix() noexcept;
	private:
		// 标记本组件为"脏"（需要重算矩阵）。
		// 同时通过 m_world.addDirtyTransformInternal 把自己登记到 World 的待刷新列表，
		// 让 World::update 在每帧逻辑之后统一处理这些脏组件，保证更新顺序正确。
		void markAsDirty();
	private:
		// 三大变换参数：位置、旋转（欧拉角，弧度）、缩放。
		Vec3 m_position{ 0,0,0 };
		Vec3 m_rotation{ 0,0,0 };
		Vec3 m_scale{ 1,1,1 };
		
		// 刚体世界矩阵：仅 rotation + translation（旋转后平移）。
		// 用于法线/方向，避免缩放影响。详见类头注释。
		Mat4x4 m_rigidWorldMatrix{};   // rotation + translation only
		// 仿射世界矩阵：scale 先作用，再叠加刚体矩阵。
		// 即 scale * (rotation * translation)，用于顶点位置变换。
		Mat4x4 m_affineWorldMatrix{};  // rotation + translation + scale

		// 脏标记：true 表示缓存的世界矩阵已过时、需要重算。
		bool m_dirty{};
	};
}
