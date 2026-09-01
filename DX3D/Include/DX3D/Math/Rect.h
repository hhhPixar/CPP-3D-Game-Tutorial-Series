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
// 文件：Rect.h
// 职责：定义矩形/尺寸类型 Rect（left、top、width、height）。
// 在引擎中的位置：常用于描述窗口大小、视口 (viewport) 区域、
//   显示器分辨率等"整数像素"几何量。注意它用的是 i32（整数），
//   而非浮点，因为屏幕像素没有小数。
// 给初学者：Rect 把"左上角位置 (left, top)"和"尺寸 (width,
//   height)"打包在一起，方便整体传递。两个构造函数分别支持
//   "只给尺寸（左上角默认 0,0）"和"完整指定四项"。
// ============================================================



namespace dx3d
{
	// ------------------------------------------------------------
	// 矩形类 Rect。
	// 设计意图：纯数据类型，四个 int 成员公开；提供两种构造与
	//   相等/不等比较。默认 left/top/width/height 都为 0。
	// 关键成员：left、top（左上角坐标），width、height（尺寸），
	//   均为 i32（32 位有符号整数）。
	// ------------------------------------------------------------
	class Rect
	{
	public:
		// 默认构造：四项都为 0。
		Rect() = default;
		// 只给尺寸的构造：left/top 默认 0,0，适合"只关心大小"的场景。
		Rect(i32 width, i32 height) : left(0), top(0), width(width), height(height) {}
		// 完整构造：显式指定左上角与尺寸。
		Rect(i32 left, i32 top, i32 width, i32 height) : left(left), top(top), width(width), height(height) {}

		// 相等 ==：四项全部相等才相等。const 表示不修改本对象。
		bool operator==(const Rect& other) const noexcept
		{
			return left == other.left &&
				top == other.top &&
				width == other.width &&
				height == other.height;
		}

		// 不等 !=：直接复用 == 取反，保证两者逻辑始终一致。
		bool operator!=(const Rect& other) const noexcept
		{
			return !(*this == other);
		}

	public:
		// 左上角坐标与宽高；{} 保证默认为 0。
		i32 left{}, top{}, width{}, height{};
	};



}