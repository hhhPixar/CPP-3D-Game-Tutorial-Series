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
// 文件：InputSystem.h —— Input（输入）子系统
// 职责：统一收集键盘与鼠标的当前状态，供 Game/World 查询。
// 设计要点：采用“轮询（Polling）”模型——每帧由 update() 主动
//   读取按键与鼠标状态，而非被动等待 Windows 消息（WM_KEYDOWN 等）。
//   这样查询逻辑简单、与帧率对齐，适合游戏。
// 关键概念：
//   · 按键状态：用 current/previous 两个数组做“边缘检测”，
//     区分“按住 isKeyDown / 刚按下 isKeyPressed / 刚松开 isKeyReleased”。
//   · 鼠标锁定：FPS 视角下把鼠标锁在窗口中心，每帧用“偏离中心的位移”
//     驱动摄像机旋转，并把鼠标拉回中心，使其可无限移动而不撞屏幕边缘。
// ============================================================
#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Math/Vec2.h>
#include <DX3D/Math/Rect.h>
#include <array>

namespace dx3d
{
	// 输入系统类。final 表示不可被继承。
	// 通过 Game::getInputSystem() 获取实例；Game 初始化时会调用
	// setCursorLockArea(getClientAreaInScreenSpace()) 设定鼠标锁定区域。
	class InputSystem final: public Base
	{
	public:
		// 构造函数：仅初始化基类（日志器）。
		// 按键状态默认全 false，鼠标位置/delta 默认 0。
		explicit InputSystem(const InputSystemDesc& desc);

		// 某键当前是否处于“按下”状态。按住期间每帧都返回 true（持续型）。
		bool isKeyDown(KeyCode key) const;
		// 某键是否在“本帧”刚被按下：当前按下 且 上一帧未按下。
		// 适合“单次触发”事件（如开火、跳跃）——按住不会连续触发。
		bool isKeyPressed(KeyCode key) const;
		// 某键是否在“本帧”刚被松开：当前未按 且 上一帧在按。
		bool isKeyReleased(KeyCode key) const;

		// 鼠标当前位置（屏幕坐标，像素）。
		Vec2 getMousePosition() const noexcept;
		// 鼠标自上一帧以来的位移增量（屏幕坐标，x/y 像素）。
		// 锁定模式下它代表“从中心偏移了多少”，游戏用它旋转摄像机。
		Vec2 getMouseDelta() const noexcept;

		// 控制系统鼠标光标是否可见。
		void setCursorVisible(bool visible);
        // 开关“鼠标锁定”：开启后每帧把鼠标强制移回锁定区域中心。
        // 这样鼠标可无限移动而不会撞到屏幕边缘——第一人称视角常用。
        void setCursorLocked(bool locked);
        // 设定鼠标锁定区域（通常为窗口客户区矩形）。锁定时鼠标被限制在此区域内。
        void setCursorLockArea(const Rect& rect);

		// 每帧调用一次（由 Game::onInternalUpdate 触发）。
		// 做四件事：保存上一帧按键、轮询当前按键、读鼠标位置并算增量、锁定时归位鼠标。
		void update();
	private:
		// 把引擎 KeyCode 枚举映射为 Windows 虚拟键码（VK_*），供 GetAsyncKeyState 轮询。
		short getInternalKeyCode(const KeyCode& key);
		// 把鼠标移到锁定区域中心（内部用 SetCursorPos）。
		void centerCursor();
	private:
		// 本帧各键状态（按下=true）。以 KeyCode 为下标，大小 = KeyCode::Count。
		std::array<bool, static_cast<std::size_t>(KeyCode::Count)> m_currentKeys{};
		// 上一帧各键状态，用于“按下/松开”的边缘检测。
		std::array<bool, static_cast<std::size_t>(KeyCode::Count)> m_previousKeys{};

		// 鼠标当前位置（屏幕坐标）。
		Vec2 m_mousePosition{};
		// 上一帧鼠标位置，用于计算增量。
		Vec2 m_previousMousePosition{};
		// 本帧鼠标位移增量 = 当前位置 - 上一帧位置。
		Vec2 m_mouseDelta{};

		// 鼠标锁定区域（屏幕坐标）。
		Rect m_lockArea{};

		// 光标是否可见（默认可见）。
		bool m_cursorVisible{ true };
		// 是否处于锁定模式（默认否）。
		bool m_cursorLocked{ false };
	};

}
