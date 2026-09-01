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

// ============================================================
// 文件：Window.h —— Window（窗口）子系统
// 职责：定义平台无关的窗口抽象基类 Window。它只声明窗口的通用接口
//       （构造、析构、获取客户区在屏幕坐标系下的矩形），把"创建窗口"
//       的具体细节留给各操作系统去实现。
// 架构位置：Window 继承自 Base（持有 Logger 日志器）；Game 中的 Display
//           类继承 Window，并附加 SwapChain（交换链），把窗口与渲染绑定。
// 关键概念：
//   · 客户区（Client Area）= 窗口内不含标题栏/边框的纯绘图区域，渲染目标
//     与鼠标锁定区域都以客户区为准，而非整个窗口外框。
//   · 平台无关：头文件用 void* 存放原生窗口句柄（Windows 下即 HWND），
//     不在头文件暴露 <Windows.h>，实现细节集中在对应 .cpp（见 Win32Window.cpp）。
// ============================================================
#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Common.h>


namespace dx3d
{
	// 窗口抽象基类：只声明接口，不含实现。
	// 具体如何创建原生窗口、如何处理 OS 消息，由各平台实现提供
	// （Windows 实现见 Source/DX3D/Window/Win32/Win32Window.cpp）。
	// 子类 Display 会在此基础上扩展渲染相关功能（持有 SwapChain）。
	class Window: public Base
	{
	public:
		// 构造函数：依据 WindowDesc（base=日志器，size=客户区尺寸）创建
		// 底层原生窗口。explicit 禁止隐式转换，必须显式构造。
		explicit Window(const WindowDesc& desc);
		// 虚析构（override）：声明为 virtual，保证用 Window 基类指针
		// 删除子类对象时也能调用子类析构，正确关闭/释放原生窗口。
		virtual ~Window() override;
		// 获取本窗口客户区在"屏幕坐标系"下的矩形：left/top 为客户区
		// 左上角在屏幕中的坐标，width/height 为客户区尺寸。
		// 注意是客户区（绘图区）而非整个窗口；Game 用它设置鼠标锁定区域。
		dx3d::Rect getClientAreaInScreenSpace();
	protected:
		// 原生窗口句柄（Windows 下即 HWND）。用 void* 而非 HWND，是为了
		// 不在头文件依赖 <Windows.h>，保持平台无关；{} 表示零初始化（nullptr）。
		void* m_handle{};
		// 客户区尺寸（width/height，像素）；{} 零初始化。
		Rect m_size{};
	};
}

