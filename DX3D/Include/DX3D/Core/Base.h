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
// 所属子系统：Core（核心基础设施）—— 对象基类
// 文件职责：声明 Base 类，它是几乎所有引擎对象的共同基类。
//
// 关键概念讲解（初学者重点理解）：
//   Base 只持有一样东西——一个 Logger 引用。这意味着：
//     1) 所有派生类都能通过 getLogger() 打日志，无需各自持有日志器；
//     2) Logger 由最顶层（Game）创建，随着 Desc 一路传递给每个对象，
//        全引擎共用同一个 Logger 实例，日志级别统一控制。
//   通过 dx3d_disable_copy_and_move 宏禁用了拷贝/移动，因为持有引用的对象
//   一旦拷贝会让两个对象绑定到同一引用，违背引用必须绑定的语义，也防止
//   引擎对象（常含 D3D11 资源句柄）被意外复制造成双重释放。
//
// 架构位置：继承树根节点之一（另一个根是 Identifiable，它继承 Base）。
// 协作对象：Logger（被持有）、BaseDesc（构造参数）、所有派生类。
// =============================================================================

#pragma once
#include <DX3D/Core/Common.h>


namespace dx3d
{
	// Base：引擎对象基类。继承它的类可获得统一的日志能力。
	// 设计要点：
	//   - 用宏禁用拷贝/移动（见下），保证对象身份唯一、不可复制。
	//   - 持有 Logger&（引用）而非指针：引用不可为空，强制必须有日志器。
	//   - 析构 virtual：保证 delete 基类指针时能正确调用派生类析构。
	//   - getLogger() 用 final：禁止派生类再覆盖，统一日志访问入口。
	class Base
	{
		dx3d_disable_copy_and_move(Base)
	public:
		// 构造函数：explicit 防止隐式转换，必须显式传入 BaseDesc。
		// 用 desc.logger 初始化引用成员 m_logger（引用必须在构造时绑定）。
		explicit Base(const BaseDesc& desc);
		// 虚析构：派生类通过基类指针销毁时能正确调用自身析构函数。
		virtual ~Base();
		// 返回日志器引用，供派生类调用 DX3DLog* 宏打印日志。
		// noexcept 承诺不抛异常；final 表示派生类不得覆盖此方法。
		virtual Logger& getLogger() const noexcept final;

	protected:
		// 持有的日志器引用。引用绑定后不可再绑定到别的 Logger。
		// 因基类禁止拷贝/移动，引用成员的「不可重新绑定」也不会被破坏。
		Logger& m_logger;
	};
}

