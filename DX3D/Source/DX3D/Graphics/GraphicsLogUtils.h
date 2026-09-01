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
// GraphicsLogUtils.h —— 图形层 HRESULT 检查工具
// -----------------------------------------------------------------------------
// 职责：封装对 Direct3D/DXGI API 返回值（HRESULT）的统一检查。
//   D3D/DXGI 几乎所有 API 都返回 HRESULT，FAILED(hr) 为真表示失败。
// 关键概念——HRESULT：Win32/COM 风格的返回码，负数表示错误，常用 SUCCEEDED/FAILED 判断。
//   本文件提供：① 着色器编译结果检查（含错误信息 blob）；② 失败即抛异常的便捷宏。
// =============================================================================
#pragma once
#include<DX3D/Core/Logger.h>
#include <d3d11.h>

namespace dx3d
{
	// GraphicsLogUtils 命名空间：内联函数，头文件中可安全包含使用。
	namespace GraphicsLogUtils
	{
		// CheckShaderCompile：检查着色器编译结果。
		//   hr：D3DCompile 的返回值；errorBlob：编译器输出的错误/警告文本。
		//   失败则记录详细错误并抛 runtime_error；有警告但成功则记录警告。
		inline void CheckShaderCompile(Logger& logger, HRESULT hr, ID3DBlob* errorBlob)
		{
			// 若有错误信息 blob，取出其缓冲指针（以 C 字符串看待）。
			auto errorMsg = errorBlob ? static_cast<const char*>(errorBlob->GetBufferPointer()) : nullptr;

			// 编译失败：记录错误详情并抛异常，终止流程。
			if (FAILED(hr))
				DX3DLogThrow(logger, std::runtime_error, Logger::LogLevel::Error, "Shader compilation failed.\nDetails:\n{}",
					errorMsg ? errorMsg : "");
			// 编译成功但有警告信息：仅记录警告，继续运行。
			if (errorMsg)
				DX3DLog(logger, Logger::LogLevel::Warning,"Shader compiled with warnings.\nDetails:\n{}", errorMsg);
		}

	}
}

// DX3DGraphicsLogThrowOnFail：检查 HRESULT，失败则抛异常的通用宏。
//   用法：DX3DGraphicsLogThrowOnFail(apiCall(), "失败描述", 可选格式化参数...);
//   先求值 hr 为局部变量 res，再判断 FAILED，避免重复求值。
#define DX3DGraphicsLogThrowOnFail(hr,message,...)\
	{\
	auto res = (hr);\
	if (FAILED(res))\
		DX3DLogThrowError(message, __VA_ARGS__);\
	}


// DX3DGraphicsCheckShaderCompile：便捷宏，调用 GraphicsLogUtils::CheckShaderCompile。
//   通过 getLogger() 取得日志器（要求调用处所在类继承 Base，提供 getLogger）。
#define DX3DGraphicsCheckShaderCompile(hr, errorBlob)\
{\
auto res = (hr);\
dx3d::GraphicsLogUtils::CheckShaderCompile(getLogger(), res, errorBlob);\
}