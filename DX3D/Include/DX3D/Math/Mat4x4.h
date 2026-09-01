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
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec4.h>
#include <DX3D/Math/MathUtils.h>
#include <cmath>
#include <cassert>
// ============================================================
// 子系统：Math 数学库
// 文件：Mat4x4.h
// 职责：定义 4×4 矩阵 Mat4x4，提供图形流水线所需的全部
//   变换：单位/平移/缩放/绕轴旋转，以及投影（正交、透视），
//   外加行列式、求逆、矩阵乘法、取行/取列。
// 在引擎中的位置：3D 渲染的核心——模型->世界->视图->投影
//   整条变换链都靠 4×4 矩阵完成。本类是 Vec3/Vec4 的主要
//   "消费者"，inverse/determinant 重度依赖 Vec4::cross。
//
// 给初学者的关键概念：
//   1) 存储约定：m_data[row][col]，即"行主序 (row-major)"
//      ——第一个下标是行、第二个是列。例如 m_data[3][0] 表示
//      第 4 行第 1 列。Direct3D 惯用"行向量 (1×4)"左乘矩阵
//      (v * M)，所以平移量放在最后一行 (m_data[3][0..2])。
//   2) LH = 左手坐标系 (left-handed)：DirectX 默认 +Z 朝"屏幕
//      里面"。本文件的投影矩阵都是 LH 版本。
//   3) 齐次坐标：用 4 个分量 (x,y,z,w) 表达 3D。w=1 表示点，
//      w=0 表示方向。4×4 矩阵才能把"平移"也统一成矩阵乘法。
//   4) 投影：把 3D 空间映射到 2D 屏幕。正交=无近大远小（平行
//      投影，常用于 2D/UI）；透视=近大远小（符合真实相机）。
// ============================================================

namespace dx3d
{
	// ------------------------------------------------------------
	// 4×4 矩阵类 Mat4x4。
	// 设计意图：用静态工厂方法 (identity/translate/scale/rotateX
	//   /rotateY/rotateZ/orthoLH/perspectiveFovLH) 生成各类变换
	//   矩阵，再通过 operator* 组合。inverse/determinant 提供求逆。
	// 关键成员：m_data[4][4] 行主序存储，private，所有访问通过
	//   方法或友元完成。默认全 0（{} 初始化）。
	// ------------------------------------------------------------
	class Mat4x4
	{
	public:
		// 默认构造：m_data 全为 0（注意不是单位矩阵，要单位矩阵请调 identity()）。
		Mat4x4() = default;

		// 单位矩阵 (identity)：除对角线为 1、其余为 0。任何向量乘以它都不变，
		//   相当于"乘法里的 1"。常作为 translate/scale 等的起始模板。
		static Mat4x4 identity() noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = 1;
			res.m_data[1][1] = 1;
			res.m_data[2][2] = 1;
			res.m_data[3][3] = 1;
			return res;
		}

		// 平移矩阵 (translate)：把点整体移动 translation。
		//   在行向量约定下，平移量写入第 4 行前三列 m_data[3][0..2]。
		//   [x y z 1] * 此矩阵 = [x+tx, y+ty, z+tz, 1]。先取单位阵再改这一行。
		static Mat4x4 translate(const Vec3& translation) noexcept
		{
			auto res = Mat4x4::identity();
			res.m_data[3][0] = translation.x;
			res.m_data[3][1] = translation.y;
			res.m_data[3][2] = translation.z;
			return res;
		}

		// 缩放矩阵 (scale)：沿各轴分别拉伸 scale.x/y/z 倍。
		//   各分量放在对角线 m_data[0][0]、[1][1]、[2][2]；[3][3]=1 保住 w。
		//   大于 1 放大，0~1 缩小，负值镜像翻转。
		static Mat4x4 scale(const Vec3& scale) noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = scale.x;
			res.m_data[1][1] = scale.y;
			res.m_data[2][2] = scale.z;
			res.m_data[3][3] = 1;
			return res;
		}

		// 绕 X 轴旋转 (rotateX)：X 轴不动，Y-Z 平面内旋转角度 x（弧度）。
		//   矩阵行：r0=[1,0,0,0] r1=[0,cos,sin,0] r2=[0,-sin,cos,0] r3=[0,0,0,1]。
		//   结果：y'=y·cos+z·sin，z'=-y·sin+z·cos（行向量 v*M）。
		static Mat4x4 rotateX(f32 x) noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = 1;
			res.m_data[1][1] = std::cos(x);
			res.m_data[1][2] = std::sin(x);
			res.m_data[2][1] = -std::sin(x);
			res.m_data[2][2] = std::cos(x);
			res.m_data[3][3] = 1;
			return res;
		}

		// 绕 Y 轴旋转 (rotateY)：Y 轴不动，X-Z 平面内旋转角度 y（弧度）。
		//   行：r0=[cos,0,-sin,0] r1=[0,1,0,0] r2=[sin,0,cos,0] r3=[0,0,0,1]。
		//   结果：x'=x·cos-z·sin，z'=x·sin+z·cos。
		static Mat4x4 rotateY(f32 y) noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = std::cos(y);
			res.m_data[1][1] = 1;
			res.m_data[0][2] = -std::sin(y);
			res.m_data[2][0] = std::sin(y);
			res.m_data[2][2] = std::cos(y);
			res.m_data[3][3] = 1;
			return res;
		}

		// 绕 Z 轴旋转 (rotateZ)：Z 轴不动，X-Y 平面内旋转角度 z（弧度）。
		//   行：r0=[cos,sin,0,0] r1=[-sin,cos,0,0] r2=[0,0,1,0] r3=[0,0,0,1]。
		//   结果：x'=x·cos+y·sin，y'=-x·sin+y·cos。这正是 2D 旋转公式。
		static Mat4x4 rotateZ(f32 z) noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = std::cos(z);
			res.m_data[0][1] = std::sin(z);
			res.m_data[1][0] = -std::sin(z);
			res.m_data[1][1] = std::cos(z);
			res.m_data[2][2] = 1;
			res.m_data[3][3] = 1;
			return res;
		}

		// 正交投影矩阵 (orthoLH)：左手坐标系的平行投影，无"近大远小"。
		//   常用于 2D、UI、正交视角的 3D。把视口 [−w/2,w/2]×[−h/2,h/2]
		//   映射到 [−1,1]，深度 [zNear,zFar] 映射到 [0,1]（DirectX 约定）。
		//   参数：width/height 视口宽高；zNear/zFar 近远裁剪面。
		//   - m[0][0]=2/w：x 从 [−w/2,w/2] 缩放到 [−1,1]。
		//   - m[1][1]=2/h：y 同理。
		//   - m[2][2]=1/(zFar−zNear)：z 的缩放系数。
		//   - m[3][2]=−zNear/(zFar−zNear)：把 z 平移到 [0,1] 起点。
		static Mat4x4 orthoLH(f32 width, f32 height, f32 zNear, f32 zFar) noexcept
		{
			assert(width != 0.0f && "OrthoLH: width must not be zero");
			assert(height != 0.0f && "OrthoLH: height must not be zero");
			assert(zFar != zNear && "OrthoLH: zNear and zFar cannot be equal");

			Mat4x4 res{};
			res.m_data[0][0] = 2.0f / width;
			res.m_data[1][1] = 2.0f / height;
			res.m_data[2][2] = 1.0f / (zFar - zNear);
			res.m_data[3][2] = -(zNear / (zFar - zNear));
			res.m_data[3][3] = 1;
			return res;
		}

		// 透视投影矩阵 (perspectiveFovLH)：左手坐标系的透视投影，有"近大远小"。
		//   模拟真实相机：视锥内一点投影到 [−1,1]×[−1,1]×[0,1] 的裁剪空间。
		//   参数：fov 垂直视场角(弧度)；aspect=宽/高；zNear/zFar 近远裁剪面。
		//   - yscale=1/tan(fov/2)：垂直缩放。tan(fov/2) 是距离 1 处视锥半高，
		//     取倒数让视锥恰好"塞进"[−1,1]。fov 越大视野越广、物体越小。
		//   - xscale=yscale/aspect：水平缩放。aspect>1(宽屏) 时水平缩放更小，
		//     使宽高比正确、画面不被横向拉伸。
		//   - m[2][2]=zFar/(zFar−zNear)、m[3][2]=−zNear·zFar/(zFar−zNear)：
		//     非线性地把 [zNear,zFar] 映射到 [0,1]（近处精度高，远处精度低）。
		//   - m[2][3]=1：把原始 z 拷贝到 w，随后硬件做"透视除法"(除以 w)
		//     产生近大远小。所以 m[3][3]=0（w 不再保留原值）。
		static Mat4x4 perspectiveFovLH(f32 fov, f32 aspect, f32 zNear, f32 zFar) noexcept
		{
			assert(fov > 0.001f && "perspectiveFovLH: fov must be greater than 0 radians");
			assert(fov < MathUtils::PI - 0.001f && "perspectiveFovLH: fov must be less than PI radians");
			assert(aspect > 0.0f && "perspectiveFovLH: aspect ratio must be greater than 0");
			assert(zFar != zNear && "perspectiveFovLH: zNear and zFar cannot be equal");

			Mat4x4 res{};
			f32 yscale = 1.0f / tan(fov / 2.0f);
			f32 xscale = yscale / aspect;
			res.m_data[0][0] = xscale;
			res.m_data[1][1] = yscale;
			res.m_data[2][2] = zFar / (zFar - zNear);
			res.m_data[2][3] = 1.0f;
			res.m_data[3][2] = (-zNear * zFar) / (zFar - zNear);
			res.m_data[3][3] = 0.0f;
			return res;
		}

		// 矩阵求逆 (inverse)：返回 rhs 的逆矩阵，使 rhs * inverse(rhs) = 单位阵。
		//   用途：把"世界->局部"矩阵反过来得到"局部->世界"等。
		//   算法：伴随矩阵法 (adjugate/det)。先算行列式 det；若 det=
		//   (矩阵不可逆) 直接返回零矩阵。否则对每一列 i：收集除 i 列外的
		//   其它 3 列，用 Vec4::cross 得到该列的余子式向量 v，再按
		//   (−1)^i · v / det 写入输出矩阵的第 i 列 (转置写入 = 伴随矩阵)。
		static Mat4x4 inverse(const Mat4x4& rhs) noexcept
		{
			Mat4x4 out{};
			Vec4 vec[3]{};

			auto det = Mat4x4::determinant(rhs);
			// 行列式为 0：矩阵奇异(不可逆)，返回全 0 矩阵作为兜底。
			if (!det) return{};
			// 遍历每一列 i，准备用"其余 3 列"算该列的伴随元素。
			for (auto i = 0; i < 4; i++)
			{
				// 遍历每一行 j，把"不是 i 列"的那 3 列收集进 vec[0..2]。
				for (auto j = 0; j < 4; j++)
				{
					if (j != i)
					{
						// a 是 vec 的紧凑下标：跳过 i 列后重新编号 0/1/2。
						auto a = j;
						if (j > i) a = a - 1;
						vec[a].x = (rhs.m_data[j][0]);
						vec[a].y = (rhs.m_data[j][1]);
						vec[a].z = (rhs.m_data[j][2]);
						vec[a].w = (rhs.m_data[j][3]);
					}
				}
				// 三列的叉积 = 第 i 列的余子式向量 (cofactor vector)。
				auto v = Vec4::cross(vec[0], vec[1], vec[2]);

				// 伴随矩阵 = 余子式矩阵的转置，所以结果写到第 i 列 (行变列)。
				// (−1)^i 给出棋盘符号 (+,-,+,-)。最后除以 det 完成求逆。
				out.m_data[0][i] = (f32)std::pow(-1.0f, i) * v.x / det;
				out.m_data[1][i] = (f32)std::pow(-1.0f, i) * v.y / det;
				out.m_data[2][i] = (f32)std::pow(-1.0f, i) * v.z / det;
				out.m_data[3][i] = (f32)std::pow(-1.0f, i) * v.w / det;
			}

			return out;
		}
		 
		static f32 determinant(const Mat4x4& rhs) noexcept
		{
			// 取前 3 列各 4 行元素组成 v1/v2/v3（每列当作一个四维向量）。
			auto v1 = Vec4(rhs.m_data[0][0], rhs.m_data[1][0], rhs.m_data[2][0], rhs.m_data[3][0]);
			auto v2 = Vec4(rhs.m_data[0][1], rhs.m_data[1][1], rhs.m_data[2][1], rhs.m_data[3][1]);
			auto v3 = Vec4(rhs.m_data[0][2], rhs.m_data[1][2], rhs.m_data[2][2], rhs.m_data[3][2]);

			// minor = 前 3 列的叉积 = 各位置对应的 3×3 余子式 (代数余子式向量)。
			auto minor = Vec4::cross(v1, v2, v3);
			// 行列式 = 第 4 列与 minor 的点积（前面带负号，对应按第 4 列展开的符号）。
			auto det =
				-(rhs.m_data[0][3] * minor.x +
					rhs.m_data[1][3] * minor.y +
					rhs.m_data[2][3] * minor.z +
					rhs.m_data[3][3] * minor.w);
			return det;
		}


		// 取第 index 行（0~3）作为一个 Vec4。行向量约定下，一行就是一个变换基底。
		Vec4 row(ui32 index) const
		{
			assert(index < 4 && "Matrix row index out of range");
			return { m_data[index][0], m_data[index][1], m_data[index][2], m_data[index][3] };
		}

		// 取第 index 列（0~3）作为一个 Vec4。常用于按列解读矩阵（如读取基向量）。
		Vec4 column(ui32 index) const
		{
			assert(index < 4 && "Matrix column index out of range");
			return { m_data[0][index], m_data[1][index], m_data[2][index], m_data[3][index] };
		}

		// 矩阵乘法 operator*：res = this * rhs。结果矩阵把 rhs 的变换"先于" this 发生。
		//   即对向量 v：v * (A * B) = (v * A) * B，先做 B 再做 A——所以组合矩阵时
		//   顺序与"应用顺序"相反。这里用 i-k-j 循环（缓存 temp=m[row][k] 提速）。
		Mat4x4 operator *(const Mat4x4& rhs) const noexcept
		{
			Mat4x4 res{};
			for (auto row = 0u; row < 4u; ++row)
			{
				for (auto k = 0u; k < 4u; ++k)
				{
					auto temp = m_data[row][k];
					for (auto col = 0u; col < 4u; ++col)
					{
						// res[row][col] += this_row[k] * rhs[k][col]：标准矩阵乘法定义。
						res.m_data[row][col] += temp * rhs.m_data[k][col];
					}
				}
			}
			return res;
		}

	private:
		// 实际存储：4×4 浮点数组，行主序 m_data[行][列]，{} 默认全 0。
		f32 m_data[4][4]{};
	};
}