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
// 相机组件（CameraComponent）—— 把 3D 世界"看"到 2D 屏幕上
// ===================================================================
// 【所属子系统】Component 组件子系统，挂到 GameObject 上使其成为一台摄像机。
// 【职责】提供两类矩阵：
//   - 视图矩阵 view matrix：把"世界坐标"变换到"相机/观察坐标"（相机视角下的坐标）。
//     本引擎用刚体世界矩阵的逆矩阵实现：相机世界矩阵把相机摆到场景中，
//     它的逆矩阵反过来就是把整个世界变换到相机原点——这正是 view 矩阵。
//   - 投影矩阵 projection matrix：把 3D 观察坐标"压扁"到 2D 屏幕平面，
//     近大远小（透视投影 perspective）。由 FOV、宽高比、近/远平面决定。
// 【透视投影概念】想象一个从相机出发向外扩张的视锥体（frustum）：
//   近裁剪面（near plane）和远裁剪面（far plane）截取它，只有视锥内的物体才被画出来。
//   FOV 越大视野越广（类似广角镜头），近/远平面定义可见范围。所有参数改了都需重算投影矩阵。
// 【与 Transform 的关系】相机位置/朝向不在这里存，而是读取所属 GameObject 的 Transform。
//   所以"移动相机"=改其 Transform 的 position/rotation；本组件只管投影参数和取 view 矩阵。
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Math/Rect.h>
#include <DX3D/Math/Mat4x4.h>

namespace dx3d
{
	// 相机组件：final 表示不可再被继承。挂到 GameObject 上即成为一台摄像机。
	class CameraComponent final : public Component
	{
		// 类型 id 注册，World/WorldRenderer 用它按类型查询所有相机。
		dx3d_typeid(CameraComponent)
	public:
		// 构造时立即计算一次投影矩阵，保证可用。
		explicit CameraComponent(const ComponentDesc& data);

		// 取视图矩阵（世界->相机坐标）。
		// 实现为 m_object 的 Transform 刚体世界矩阵的逆：相机世界矩阵摆相机，
		// 其逆矩阵把世界搬进相机视角。所以相机的 position/forward 即由 Transform 决定。
		Mat4x4 getViewMatrix() noexcept;
		// 取投影矩阵（相机坐标->屏幕坐标）。投影矩阵在参数变更时重算并缓存于此。
		Mat4x4 getProjectionMatrix() const noexcept;

		// 远裁剪面（far plane）：比这更远的物体不渲染。必须大于 near 才生效。
		void setFarPlane(f32 farPlane) noexcept;
		f32 getFarPlane() const noexcept;

		// 近裁剪面（near plane）：比这更近的物体不渲染。必须大于 0（不能为0或负）。
		void setNearPlane(f32 nearPlane) noexcept;
		f32 getNearPlane()const noexcept;

		// 视野张角（field of view，弧度）。越大视野越广但物体显得更小更远。
		// 限制在 (0, PI) 之间，否则透视无意义。
		void setFieldOfView(f32 fieldOfView) noexcept;
		f32 getFieldOfView() const noexcept;

		// 视口大小（宽高，像素）。决定宽高比 aspect = width/height，影响投影矩阵的横向缩放。
		// WorldRenderer 每帧用窗口尺寸调用 setViewportSize，使投影随窗口变化而更新。
		void setViewportSize(const Rect& size) noexcept;
		Rect getViewportSize() const noexcept;

	private:
		// 根据 FOV、宽高比、近/远面重算透视投影矩阵并缓存。
		// 内部调用 Mat4x4::perspectiveFovLH（LH=左手坐标系，DX 默认）。
		void computeProjectionMatrix() noexcept;

	private:		
		// 缓存的投影矩阵。任意参数变更后由 computeProjectionMatrix 重算。
		Mat4x4 m_projection{};

		// 透视参数：近面、远面、FOV（弧度，默认约 1.3 弧度≈74°）。
		f32 m_nearPlane = 0.01f;
		f32 m_farPlane = 100.0f;
		f32 m_fieldOfView = 1.3f;
		// 视口宽高（像素），用于算宽高比。默认 1x1，渲染器每帧用实际窗口尺寸覆盖。
		Rect m_viewportSize { 1,1 };

		// 投影脏标记：初始为 true，构造时强制算一次。后续各 setter 变更后置 true 并立即重算。
		bool m_dirty{ true };
	};

}
