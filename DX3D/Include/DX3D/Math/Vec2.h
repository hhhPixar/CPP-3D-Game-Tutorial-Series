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

#pragma once
#include <DX3D/Core/Core.h>
// ============================================================
// 子系统：Math 数学库
// 文件：Vec2.h
// 职责：定义二维向量 Vec2（仅 x、y 两个分量）。
// 在引擎中的位置：最基础的几何类型之一，常用于表示 2D
//   屏幕坐标、纹理坐标 (UV)、窗口/视口尺寸等不涉及深度的
//   二维量；与 Vec3/Vec4 一起构成向量类型族。
// 给初学者：向量 = 一组一起使用的数。二维向量就是把两个
//   数 (x, y) 打包，整体表示一个点或一个位移，例如
//   "把某物右移 3、下移 5" 就用 Vec2(3, 5) 来表达。
// ============================================================


namespace dx3d
{
	// ------------------------------------------------------------
	// 二维向量类 Vec2。
	// 设计意图：轻量"纯数据"类型——成员 (x, y) 公开，不做复杂
	//   封装，方便图形代码直接读取分量；这里只提供构造。
	// 关键成员：x、y 均为 f32（32 位浮点），{} 使默认值为 0。
	// ------------------------------------------------------------
	class Vec2
	{
	public:
		// 默认构造：x、y 因成员默认初始化而都为 0。= default 让编译器生成。
		Vec2() = default;
		// 带参构造：用传入的 x、y 直接初始化分量。: x(x), y(y) 是成员初始化列表。
		Vec2(f32 x, f32 y) : x(x), y(y){}

	public:
		// 向量的两个分量；{} 表示若无显式初始化则默认为 0.0f。
		f32 x{}, y{};
	};
}