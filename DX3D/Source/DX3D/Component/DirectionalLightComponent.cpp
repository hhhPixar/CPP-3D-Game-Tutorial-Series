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
// DirectionalLightComponent 实现：方向光的颜色与强度存取
// ===================================================================
// 【说明】本组件只存 color 和 intensity 两个标量属性，实现很简短。
//   真正"方向"不在此文件出现——它由 WorldRenderer 在渲染时直接读取
//   光源对象 Transform 的刚体世界矩阵 row(2)（+Z 朝向）得到。
//   所以本文件看不到方向相关逻辑，那部分在 WorldRenderer.cpp 里。
// 【光照概念】最终打在物体表面的光亮度 = 颜色(color) × 强度(intensity)，
//   再与物体材质颜色、表面法线、光线方向的点积等一起在着色器中计算（如漫反射）。
#include <DX3D/Component/DirectionalLightComponent.h>
#include <DX3D/Game/World.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec2.h>
#include <DX3D/Math/Mat4x4.h>
#include <DX3D/Graphics/GraphicsDevice.h>


// 构造函数：仅初始化基类，颜色/强度用成员默认值（白光、强度1.0）。
dx3d::DirectionaLightComponent::DirectionaLightComponent(const ComponentDesc& data) : Component(data)
{
}

// 设置光强度。无校验，调用方自行保证合理性（通常 >= 0）。
void dx3d::DirectionaLightComponent::setIntensity(dx3d::f32 intensity)
{
	m_intensity = intensity;
}

dx3d::f32 dx3d::DirectionaLightComponent::getIntensity() const noexcept
{
	return m_intensity;
}

// 设置光颜色（RGB，各分量 0~1）。
void dx3d::DirectionaLightComponent::setColor(const Vec3& color)
{
	m_color = color;
}

dx3d::Vec3 dx3d::DirectionaLightComponent::getColor() const noexcept
{
	return m_color;
}
