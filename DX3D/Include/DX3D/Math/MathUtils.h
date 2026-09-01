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
#include <numbers>
// ============================================================
// 子系统：Math 数学库
// 文件：MathUtils.h
// 职责：提供数学常量。目前只有圆周率 PI。
// 在引擎中的位置：被 Mat4x4 的投影/旋转函数引用——例如
//   perspectiveFovLH 用 PI 校验视场角上限。把常量集中存放，
//   避免各处自定义 PI 数值不一致。
// 给初学者：PI（π≈3.14159）是圆周率，弧度制下"半圈=π，
//   一圈=2π"。三角函数 std::sin/std::cos 的参数都用弧度，
//   所以把角度转弧度时常乘 PI/180。
// ============================================================

namespace dx3d
{
	// MathUtils 命名空间：集中放置数学常量，供全引擎复用。
	namespace MathUtils
	{
		// 圆周率 π（float 精度）。std::numbers::pi_v<float> 是 C++20
		//   标准库提供的编译期精确常量；inline 让头文件里定义也不会
		//   重复链接。auto 推导为 float。
		inline auto PI = std::numbers::pi_v<float>;
	}
}