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

// =============================================================================
// GraphicsUtils.h —— D3D 辅助工具函数
// -----------------------------------------------------------------------------
// 职责：提供把引擎层枚举/签名信息转换为 D3D/DXGI 具体格式的辅助函数。
//   着色器编译与输入装配阶段会用到这些映射。
// 关键概念——着色器模型 vs 编译目标：HLSL 着色器按阶段指定编译目标，如 vs_5_0
//   （顶点着色器，Shader Model 5.0，对应 D3D11）。DXGI_FORMAT 描述显存中数据的
//   像素/元素布局，用于顶点装配与纹理论理。
// =============================================================================
#pragma once
#include <DX3D/Core/Common.h>
#include <d3d11.h>
#include <bit>

namespace dx3d
{
	// GraphicsUtils 命名空间：内联工具函数集合，可在头文件中直接定义使用。
	namespace GraphicsUtils
	{
		// GetShaderModelTarget：把 ShaderType 枚举转换为 D3DCompile 所需的编译目标字符串。
		//   顶点着色器 -> "vs_5_0"；像素着色器 -> "ps_5_0"（5_0 对应 Shader Model 5，D3D11 标准）。
		inline const char* GetShaderModelTarget(ShaderType type)
		{
			switch (type)
			{
			case ShaderType::VertexShader: return "vs_5_0";
			case ShaderType::PixelShader: return "ps_5_0";
			default: return "";
			}
		}

		// GetDXGIFormatFromMask：根据顶点签名中某属性的“组件掩码”推断对应的 DXGI_FORMAT。
		//   type：D3D 反射得到的组件类型（目前只处理 FLOAT32）。
		//   mask：位掩码，置位的每一位代表一个有效的 32 位分量（如 R32G32 = 2 个分量）。
		//   std::popcount：C++20 <bit> 函数，返回掩码中 1 的个数，即分量数量。
		//   返回 R32/G32/B32/A32 组合的浮点格式（1~4 分量）。
		inline DXGI_FORMAT GetDXGIFormatFromMask(D3D_REGISTER_COMPONENT_TYPE type, UINT mask)
		{
			// 组件数 = 掩码中 1 的位数。0 个表示未知格式。
			auto componentCount = std::popcount(mask);
			if (componentCount < 1) return DXGI_FORMAT_UNKNOWN;

			// 格式查找表：[typeIndex][componentCount-1]。当前只填了 FLOAT32 一行（4 种）。
			constexpr DXGI_FORMAT formatTable[1][4] =
			{
				{
					DXGI_FORMAT_R32_FLOAT,
					DXGI_FORMAT_R32G32_FLOAT,
					DXGI_FORMAT_R32G32B32_FLOAT,
					DXGI_FORMAT_R32G32B32A32_FLOAT
				}
			};


			// typeIndex：按组件类型选择表行。目前仅 FLOAT32 有意义，其余返回未知格式。
			auto typeIndex = 0u;
			switch (type)
			{
			case D3D_REGISTER_COMPONENT_FLOAT32: typeIndex = 0u; break;
			default: return DXGI_FORMAT_UNKNOWN;
			}

			// 用“分量数-1”作列索引，取出对应的 DXGI 格式。
			return formatTable[typeIndex][componentCount - 1];
		}

	}
}