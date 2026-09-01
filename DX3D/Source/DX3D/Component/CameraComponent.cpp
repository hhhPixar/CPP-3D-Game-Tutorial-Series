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
// CameraComponent 实现：视图矩阵与透视投影矩阵的计算
// ===================================================================
// 【核心概念讲解】
//  - 视图矩阵（view matrix）= 相机刚体世界矩阵的逆。
//    原理：相机的世界矩阵把"相机原点"搬到场景中某个位置/朝向；
//    对整个世界取逆，就等价于把世界搬回"相机在原点、朝前看"的标准姿态。
//    所以本组件不自己存位置/朝向，而是直接取所属对象的 Transform。
//  - 透视投影（perspective projection）：用 perspectiveFovLH（左手系）实现。
//    LH=Left-Handed 左手坐标系，是 Direct3D 默认坐标系约定（Z 向屏幕里增大）。
//    投影矩阵让视锥内的点近大远小地映射到 [-1,1]^3 的标准化设备坐标（NDC）。
//  - 各 setter 都做了合法性校验，无效值直接忽略并 return，避免产生畸形投影矩阵。
#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/World.h>
#include <DX3D/Game/Game.h>
#include <DX3D/Game/GameObject.h>
#include <algorithm>


// 构造函数：先初始化基类，再立刻算一次投影矩阵，保证 getProjectionMatrix 可用。
dx3d::CameraComponent::CameraComponent(const ComponentDesc& data) : Component(data)
{
	computeProjectionMatrix();
}

// 返回视图矩阵：把世界坐标变换到相机坐标。
// 实现为 相机刚体世界矩阵的逆（inverse）。用刚体矩阵而非仿射，是因为相机不应被缩放
// 影响（缩放相机会扭曲投影）。刚体矩阵只含旋转+平移，其逆就是标准的 view 矩阵。
// m_object 来自基类 Component，指本组件所属的 GameObject。
dx3d::Mat4x4 dx3d::CameraComponent::getViewMatrix() noexcept
{
	return Mat4x4::inverse(m_object.getTransform().getRigidWorldMatrix());
}

// 返回缓存的投影矩阵。投影矩阵在 setter/构造时已重算好，这里只读不重算。
dx3d::Mat4x4 dx3d::CameraComponent::getProjectionMatrix() const noexcept
{
	return m_projection;
}

// 设置远裁剪面：far 必须大于当前 near，否则忽略（防止视锥倒置）。
// 改后立即重算投影矩阵，保证后续取到的是最新结果。
void dx3d::CameraComponent::setFarPlane(f32 farPlane) noexcept
{
	if (farPlane <= m_nearPlane) return;
	m_farPlane = farPlane;
	computeProjectionMatrix();
}

dx3d::f32 dx3d::CameraComponent::getFarPlane() const noexcept
{
	return m_farPlane;
}

// 设置近裁剪面：必须大于一个极小值（0.001），否则忽略。
// near 不能为 0，否则透视除法会除零、近面物体失真。
void dx3d::CameraComponent::setNearPlane(f32 nearPlane) noexcept
{
	if (nearPlane <= 0.001f) return;
	m_nearPlane = nearPlane;
	computeProjectionMatrix();
}

dx3d::f32 dx3d::CameraComponent::getNearPlane() const noexcept
{
	return m_nearPlane;
}

// 设置 FOV（视野张角，弧度）。必须在 (0, PI) 之间，否则忽略。
// FOV=0 看不见任何东西，FOV>=PI 则透视无意义（视锥退化）。
void dx3d::CameraComponent::setFieldOfView(f32 fieldOfView) noexcept
{
	if (fieldOfView <= 0.001f || fieldOfView >= MathUtils::PI) return;
	m_fieldOfView = fieldOfView;
	computeProjectionMatrix();
}

dx3d::f32 dx3d::CameraComponent::getFieldOfView() const noexcept
{
	return m_fieldOfView;
}

// 设置视口尺寸：影响宽高比 aspect，进而影响投影矩阵的横向缩放。
// 两个保护：尺寸未变则跳过；传入的宽或高为 0 也跳过（避免除零产生畸形矩阵）。
// WorldRenderer 每帧用 swapChain 的实际尺寸调用此函数，使投影随窗口缩放而更新。
void dx3d::CameraComponent::setViewportSize(const Rect& area) noexcept
{
	if (m_viewportSize == area) return;
	if (m_viewportSize.width == 0 || m_viewportSize.height == 0) return;

	m_viewportSize = area;
	computeProjectionMatrix();
}

dx3d::Rect dx3d::CameraComponent::getViewportSize() const noexcept
{
	return m_viewportSize;
}

// 真正重算透视投影矩阵的地方。调用 Mat4x4::perspectiveFovLH（左手系透视）。
// 参数：FOV、宽高比 aspect = width/height、近面、远面。
// 宽高比决定横向 vs 纵向的缩放比例，使画面不被拉伸；近远面定义可见纵深范围。
void dx3d::CameraComponent::computeProjectionMatrix() noexcept
{
	m_projection = Mat4x4::perspectiveFovLH(m_fieldOfView, (f32)m_viewportSize.width / (f32)m_viewportSize.height,
		m_nearPlane, m_farPlane);
}

