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
// 文件：Vec3.h
// 职责：定义三维向量 Vec3（x、y、z 三个分量），并附带常用
//   运算：自增 +=、数乘 *=、归一化 normalize，以及类外的
//   二元 + 与数乘 * 运算符。
// 在引擎中的位置：3D 图形最核心的类型——空间位置、方向、
//   位移、颜色 (rgb) 等都常用 Vec3；光照、相机朝向、物体
//   坐标离不开它。与 Vec2/Vec4 构成向量族。
// 给初学者：三维向量就是三个数 (x, y, z) 打包，整体表示
//   空间里一个点或一个方向。归一化 (normalize) 是把向量
//   长度缩成 1、方向不变，常用于"只要方向不要大小"的场合
//   （如光照方向、表面法线）。
// ============================================================

namespace dx3d
{
	// ------------------------------------------------------------
	// 三维向量类 Vec3。
	// 设计意图：成员公开，便于图形代码直接访问 x/y/z。
	// 关键成员：x、y、z 为 f32，默认 0。除构造外还提供
	//   operator+= (向量加)、operator*= (数乘) 与静态
	//   normalize (归一化)；类外的自由函数提供 + 与 *。
	// ------------------------------------------------------------
	class Vec3
	{

	public:
		// 默认构造：x/y/z 因成员默认初始化为 0。= default 交给编译器生成。
		Vec3() = default;
		// 带参构造：用三个浮点初始化分量。: x(x), y(y), z(z) 为成员初始化列表。
		Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

		// 向量自增 +=：把 rhs 各分量加到自己身上，返回 *this 以支持链式 (a += b += c)。
		Vec3& operator+=(const Vec3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		// 向量数乘 *=：每个分量同乘标量 scalar，整体等价于把向量沿原方向
		//   拉伸/缩短 scalar 倍（负号会反向）。返回 *this 以便链式。
		Vec3& operator*=(float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		// 归一化 normalize：把 v 缩放成长度 (length) 为 1 的向量，方向不变。
		//   为什么要归一化：光照、法线等只关心"朝哪"不关心"多远"，统一为
		//   单位长后，点积 (dot) 才能直接反映夹角余弦。
		//   做法：lenSq = 长度平方 = x²+y²+z²（不开方更快）；invLen = 1/长度；
		//   各分量乘 invLen 等价于除以长度。
		//   零向量兜底：长度为 0 无法归一化（会除以 0），这里直接返回零向量，
		//   避免产生 NaN 污染渲染。
		static Vec3 normalize(const Vec3& v)
		{
			// 长度的平方：不开方可省一次 sqrt，是图形代码常见优化。
			float lenSq = v.x * v.x + v.y * v.y + v.z * v.z;

			// 长度为 0 即零向量：无方向可言，返回零向量兜底，避免除零产生 NaN。
			if (lenSq == 0.0f) return Vec3{ 0.0f, 0.0f, 0.0f };

			// 先算 1/长度，下面用乘法代替除法（乘法比除法快）。
			float invLen = 1.0f / std::sqrt(lenSq);

			// 每个分量乘 1/长度，整体长度变为 1，方向保持不变。
			return Vec3{
				v.x * invLen,
				v.y * invLen,
				v.z * invLen
			};
		}
	public:
		// 三个分量，{} 保证未显式初始化时为 0。
		f32 x{}, y{}, z{};
	};

	// 自由函数 operator+：向量加 a + b。lhs 按值传入（得到副本），复用
	//   operator+= 完成相加再返回结果；lhs 与 rhs 都不被修改。
	inline Vec3 operator+(Vec3 lhs, const Vec3& rhs)
	{
		lhs += rhs;
		return lhs;
	}

	// 向量 * 标量：v * scalar。按值传 v 得副本，复用 *= 再返回。
	inline Vec3 operator*(Vec3 v, float scalar)
	{
		v *= scalar;
		return v;
	}

	// 标量 * 向量：scalar * v。交换律：让 2.0f * v 这种写法也合法，
	//   实现与上面一致，仅参数顺序不同。
	inline Vec3 operator*(float scalar, Vec3 v)
	{
		v *= scalar;
		return v;
	}
}

