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
// 所属子系统：Game —— Display 是“带交换链的窗口”，连接窗口与图形呈现。
// 职责：继承 Window（提供窗口句柄与客户区），额外创建并持有 SwapChain。
// 架构位置：Game 持有 Display，WorldRenderer 渲染时取 Display 的 SwapChain
//           来清屏与呈现（Present）。SwapChain 把每帧画好的后缓冲翻到屏幕。
// 关键概念：
//   - 交换链（SwapChain）：Direct3D 中负责“后缓冲画、前缓冲显”的机制，
//     实现流畅无闪烁的逐帧刷新。
// =============================================================================

#pragma once
#include <DX3D/Window/Window.h>

namespace dx3d
{
	// Display —— 窗口 + 交换链。final 表示不再被继承。
	// 它在 Window 基础上增加图形呈现能力，是“可被渲染目标”的窗口。
	class Display final: public Window
	{
	public:
		// 构造：按 DisplayDesc 创建窗口，再用图形设备为其建立 SwapChain。
		explicit Display(const DisplayDesc& desc);

		// 返回交换链引用。WorldRenderer 据此清后缓冲、设置视口并最终 Present。
		SwapChain& getSwapChain() noexcept;
	private:
		// 交换链。用 shared_ptr 因为它内部可能被多处引用（如设备上下文）。
		RefPtr<SwapChain> m_swapChain{};
	};
}

