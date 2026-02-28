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
#include <cmath>
#include <cassert>

namespace dx3d
{
	class Mat4x4
	{
	public:
		Mat4x4() = default;

		static Mat4x4 identity() noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = 1;
			res.m_data[1][1] = 1;
			res.m_data[2][2] = 1;
			res.m_data[3][3] = 1;
			return res;
		}

		static Mat4x4 translate(const Vec3& translation) noexcept
		{
			auto res = Mat4x4::identity();
			res.m_data[3][0] = translation.x;
			res.m_data[3][1] = translation.y;
			res.m_data[3][2] = translation.z;
			return res;
		}

		static Mat4x4 scale(const Vec3& scale) noexcept
		{
			Mat4x4 res{};
			res.m_data[0][0] = scale.x;
			res.m_data[1][1] = scale.y;
			res.m_data[2][2] = scale.z;
			res.m_data[3][3] = 1;
			return res;
		}

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
						res.m_data[row][col] += temp * rhs.m_data[k][col];
					}
				}
			}
			return res;
		}

	private:
		f32 m_data[4][4]{};
	};
}