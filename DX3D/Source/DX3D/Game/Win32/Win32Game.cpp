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
// Win32Game.cpp —— Game::run() 的 Windows 平台实现。
// 职责：跑 Win32 消息循环（Message Loop），在不阻塞的前提下持续处理窗口
//       消息并驱动每帧更新。这是 Game 类在 Windows 上的平台特化部分。
// 关键概念：
//   - 无阻塞循环：游戏循环必须“不卡”，所以用 PeekMessage 而非 GetMessage。
//     GetMessage 在无消息时会阻塞（让线程睡眠），不适合实时游戏；
//     PeekMessage 总是立即返回，无论有没有消息，循环可以一帧接一帧跑下去。
//   - 消息与帧分离：内层 while 先把积压的窗口消息（按键、移动、关闭等）排空，
//     排空后再执行一次 onInternalUpdate 推进一帧游戏逻辑与渲染。
// =============================================================================

#include <DX3D/Game/Game.h>
#include <Windows.h>




// Game::run —— 进入主循环。本函数会一直运行，直到收到 WM_QUIT（窗口关闭）才返回。
void dx3d::Game::run()
{
	// 初始化：在进入循环前调用一次子类的 onCreate，让它有机会创建实体、加载资源。
	onCreate();

	// Win32 消息结构，用于 PeekMessage 取出下一条消息。
	MSG msg{};
	// 记录“第一帧之前”的时间点，作为后续计算 deltaTime 的基准（首帧间隔会很小）。
	m_previousTime = std::chrono::steady_clock::now();
	// 主循环：只要 m_isRunning 为 true 就一直跑。退出消息会把它置 false。
	while (m_isRunning)
	{
		// 内层循环：PeekMessage 非阻塞地取出所有积压消息。
		// PM_REMOVE 表示取出的同时从队列里移除。
		// 参数 (NULL,0,0) 表示关心所有窗口的所有消息。
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// 收到 WM_QUIT（PostQuitMessage 发出的退出消息）就结束整个主循环。
			if (msg.message == WM_QUIT)
			{
				m_isRunning = false;
				break;
			}

			// TranslateMessage 做键盘消息的转换（如把 WM_KEYDOWN 翻成 WM_CHAR）。
			// DispatchMessage 把消息派发给对应窗口的窗口过程（WndProc）处理。
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// 消息排空后，推进一帧：算 deltaTime、更新输入、运行游戏逻辑、更新世界、渲染。
		onInternalUpdate();
	}

}


