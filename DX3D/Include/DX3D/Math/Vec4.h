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
// 文件：Vec4.h
// 职责：定义四维向量 Vec4（x、y、z、w 四个分量），并提供
//   静态 cross —— 三个四维向量的"叉积"。
// 在引擎中的位置：Vec4 主要用于 4x4 矩阵的行/列读写以及
//   齐次坐标 (homogeneous coordinate)。它被 Mat4x4 的
//   determinant / inverse 大量使用。
// 给初学者：
//   1) 第四个分量 w 在齐次坐标里很关键：点用 w=1，方向用
//      w=0；它让你用同一个矩阵统一处理平移与旋转。
//   2) 这里的 cross 不是中学"两个三维向量叉积"，而是四维
//      推广：给定三个四维向量 v1/v2/v3，求一个同时垂直于
//      它们的四维向量。它用来算 4x4 矩阵的行列式与逆
//      （见 Mat4x4::determinant / inverse）。你只需知道：
//      下面公式来自行列式的拉普拉斯展开，结果是一个"代数
//      余子式向量"，与第 4 列点乘即得行列式。
// ============================================================

namespace dx3d
{
	// ------------------------------------------------------------
	// 四维向量类 Vec4。
	// 设计意图：纯数据类型，分量公开；除构造外只提供一个静态
	//   cross（四维三向量叉积），专供矩阵求逆/行列式使用。
	// 关键成员：x、y、z、w 均为 f32，默认 0。
	// ------------------------------------------------------------
	class Vec4
	{
	public:
		// 默认构造：四个分量都默认为 0。
		Vec4() = default;
		// 带参构造：四个分量显式初始化。w 在齐次坐标中区分"点(1)/方向(0)"。
		Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

		// 四维三向量叉积（4D cross product of 3 vectors）。
		//   输入：v1、v2、v3 三个四维向量。
		//   输出：一个同时垂直于三者的四维向量；其分量大小等于
		//         对应 3×3 子矩阵的代数余子式 (cofactor)。
		//   用途：Mat4x4::determinant 用它对前 3 列叉积、再与第 4
		//         列点乘得行列式；Mat4x4::inverse 用它对每列以外的
		//         3 列叉积得到伴随矩阵的列。
		//   注意：参数是非常引用 (Vec4&)——调用方需传左值；本函数不改入参。
		//   初学者无需手算：把下面四行视作"按行列式展开公式直接代写"。
		static Vec4 cross(Vec4& v1, Vec4& v2, Vec4& v3) noexcept
		{
			return {
				// x 分量：划掉 v1.x 后，{y,z,w} 对应 3×3 子式（带符号）
				 v1.y * (v2.z * v3.w - v3.z * v2.w) - v1.z * (v2.y * v3.w - v3.y * v2.w) + v1.w * (v2.y * v3.z - v2.z * v3.y),
				// y 分量：划掉 v1.y 后，{x,z,w} 对应子式，整体取负号
				 -(v1.x * (v2.z * v3.w - v3.z * v2.w) - v1.z * (v2.x * v3.w - v3.x * v2.w) + v1.w * (v2.x * v3.z - v3.x * v2.z)),
				// z 分量：划掉 v1.z 后，{x,y,w} 对应子式（带符号）
				 v1.x * (v2.y * v3.w - v3.y * v2.w) - v1.y * (v2.x * v3.w - v3.x * v2.w) + v1.w * (v2.x * v3.y - v3.x * v2.y),
				// w 分量：划掉 v1.w 后，{x,y,z} 对应子式，整体取负号
				 -(v1.x * (v2.y * v3.z - v3.y * v2.z) - v1.y * (v2.x * v3.z - v3.x * v2.z) + v1.z * (v2.x * v3.y - v3.x * v2.y))
			};
		}
	public:
		// 四个分量，{} 保证默认 0。w 常用于齐次坐标或矩阵元素。
		f32 x{}, y{}, z{}, w{};
	};
}