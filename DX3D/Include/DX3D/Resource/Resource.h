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

// ============================================================================
// 资源系统（Resource System）—— 所有可加载资源类的基类头文件
// 职责：定义"资源（Resource）"这一抽象。一个资源 = 从磁盘文件路径加载、
//       由 ResourceManager 统一管理的数据对象（网格、纹理、材质都派生自它）。
// 架构位置：资源系统最底层基类，继承自 Core 的 Base（提供日志能力）。
// 关键概念：
//   · 资源用"文件路径"作为身份标识；路径用宽字符 wchar_t/wstring 存储，因为
//     Windows 的 Win32 API 用宽字符表示路径，可正确处理中文等非 ASCII 路径。
//   · 每个 Resource 持有创建它的 ResourceManager 引用，便于资源反向访问管理器。
//   · 资源不可拷贝（继承自 Base，Base 用宏禁用了拷贝/移动），只能通过
//     RefPtr（即 std::shared_ptr）共享，配合 ResourceManager 的缓存实现去重。
// ============================================================================
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <string>

namespace dx3d
{
	// 资源基类：所有具体资源（MeshResource/TextureResource/MaterialResource）的共同父类，
	// 统一了"带文件路径、由管理器管理"这一共性。
	class Resource : public Base
	{
	public:
		// 构造函数：用 ResourceDesc（含日志器、文件路径、所属管理器）初始化资源。
		// explicit 防止隐式转换；参数结构见 Core/Common.h 的 ResourceDesc。
		explicit Resource(const ResourceDesc& desc);
	protected:
		// 本资源的来源文件路径（宽字符串），便于按路径查缓存、调试与日志输出。
		std::wstring m_path{};
		// 创建并管理本资源的资源管理器（引用必须初始化，不可重新指向其它管理器）。
		ResourceManager& m_manager;
	};

}