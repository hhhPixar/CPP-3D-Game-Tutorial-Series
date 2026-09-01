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
// Display.cpp —— 在窗口之上创建交换链的实现。
// 顺序：先由 Window 基类构造出窗口（拿到窗口句柄 m_handle 与尺寸 m_size），
//       再用这两个值通过图形设备创建 SwapChain 并绑定到该窗口。
// =============================================================================

#include <DX3D/Game/Display.h>
#include <DX3D/Graphics/GraphicsDevice.h>

// 构造：Window(desc.window) 先建好窗口；之后 m_handle/m_size 已就绪，
// 用它们调用图形设备的 createSwapChain 建立与本窗口绑定的交换链。
dx3d::Display::Display(const DisplayDesc& desc): Window(desc.window)
{
	// m_handle 是基类 Window 保存的窗口句柄，m_size 是客户区尺寸。
	// SwapChain 需要这两项才能把渲染结果呈递到正确的窗口区域。
	m_swapChain = desc.graphicsDevice.createSwapChain({ m_handle, m_size });
}

// 返回交换链引用。渲染器据此清屏、设视口、最终 Present。
dx3d::SwapChain& dx3d::Display::getSwapChain() noexcept
{
	return *m_swapChain;
}
