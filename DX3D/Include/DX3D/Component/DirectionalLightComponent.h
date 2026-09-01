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
// 方向光组件（DirectionalLightComponent）—— 模拟太阳光
// ===================================================================
// 【所属子系统】Component 组件子系统，挂到 GameObject 上使其成为一盏方向光。
// 【职责】只存颜色 color 和强度 intensity；"方向"不在这里存，
//         而是由所属对象的 Transform 的"刚体世界矩阵"决定（见下）。
// 【方向光概念】方向光没有位置概念（像太阳，离得极远），只有"方向"。
//   所有被照物体接收到的光线方向一致。本引擎用光源对象的朝向 forward 作为光的方向。
// 【方向取自朝向的原理】WorldRenderer 渲染时取光源 Transform 的刚体世界矩阵第 3 行
//   （row(2)，即变换后的 +Z 轴）作为光的方向向量。所以"旋转光源对象=改变光照方向"。
//   用刚体矩阵而非仿射矩阵，避免缩放把方向扭曲。详见 TransformComponent 注释。
// 【命名注意】代码中类名实际拼写为 DirectionaLightComponent（少一个 l），
//   文件名是 DirectionalLightComponent.h（拼写正确）。这是源码既有拼写，保留不动。
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Component.h>

namespace dx3d
{
	// 方向光组件：final 不可继承。color/intensity 在此存，方向由 Transform 提供。
	// 注意类名拼写（少一个 l）与文件名不同，属既有代码，保持原样。
	class DirectionaLightComponent final : public Component
	{
		// 类型 id 注册。WorldRenderer 用它查询场景中的所有方向光。
		// 注意宏内参数与类名拼写一致（DirectionaLightComponent）。
		dx3d_typeid(DirectionaLightComponent)
	public:
		explicit DirectionaLightComponent(const ComponentDesc& data);

		// 设置/获取光强度（标量）。最终光照 = color * intensity（在着色器里相乘）。
		void setIntensity(dx3d::f32 intensity);
		f32 getIntensity() const noexcept;

		// 设置/获取光的颜色（RGB，各分量 0~1）。白色 {1,1,1} 是最常用的中性光。
		void setColor(const Vec3& color);
		Vec3 getColor() const noexcept;
	private:
		// 光颜色，默认白光 {1,1,1}（不偏色）。
		Vec3 m_color{ 1,1,1 };
		// 光强度，默认 1.0。控制光的明暗。
		f32 m_intensity = 1.0f;
	};
}
