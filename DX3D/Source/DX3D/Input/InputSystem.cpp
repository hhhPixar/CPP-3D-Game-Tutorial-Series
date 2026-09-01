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
// 文件：InputSystem.cpp —— Input 子系统的实现
// 依赖 Windows API：GetAsyncKeyState 轮询按键、GetCursorPos/SetCursorPos
//   读写鼠标位置、ShowCursor 控制可见性。
// 数据流：每帧 update() → 轮询所有按键 → 读鼠标位置 → 算增量 → 锁定归位。
// 说明：本实现不监听 WM_KEYDOWN 等消息，而是主动轮询，查询简单且与帧同步。
// ============================================================
#include <DX3D/Input/InputSystem.h>
#include <ranges>
#include <Windows.h>

// 构造函数：仅初始化基类 Base（持有日志器）。
// 按键数组与鼠标位置均在头文件中默认初始化（全 false / 0）。
dx3d::InputSystem::InputSystem(const InputSystemDesc& desc) : Base(desc.base)
{
}

// 某键当前是否按下：直接查本帧状态数组 m_currentKeys。
// key 先转成下标（KeyCode 枚举值）。
bool dx3d::InputSystem::isKeyDown(KeyCode key) const
{
	return m_currentKeys[static_cast<std::size_t>(key)];
}

// 某键是否在本帧“刚按下”：本帧按下 且 上一帧未按下（边缘检测）。
bool dx3d::InputSystem::isKeyPressed(KeyCode key) const
{
	return m_currentKeys[static_cast<std::size_t>(key)] &&
		!m_previousKeys[static_cast<std::size_t>(key)];
}

// 某键是否在本帧“刚松开”：本帧未按 且 上一帧在按（边缘检测）。
bool dx3d::InputSystem::isKeyReleased(KeyCode key) const
{
	return !m_currentKeys[static_cast<std::size_t>(key)] &&
		m_previousKeys[static_cast<std::size_t>(key)];
}


// 返回鼠标当前位置（屏幕坐标）。
dx3d::Vec2 dx3d::InputSystem::getMousePosition() const noexcept
{
	return m_mousePosition;
}

// 返回本帧鼠标位移增量。游戏用它驱动摄像机旋转（尤其锁定模式下）。
dx3d::Vec2 dx3d::InputSystem::getMouseDelta() const noexcept
{
	return m_mouseDelta;
}


// 设置鼠标光标可见性。注意 ShowCursor 是引用计数式 API，
// 见下方两个 while 循环：把计数“逼”到目标状态，确保真正显示/隐藏。
void dx3d::InputSystem::setCursorVisible(bool visible)
{
	m_cursorVisible = visible;

	// ShowCursor 内部维护一个计数器：返回值 <0 表示隐藏、≥0 表示显示。
	// 这两个循环分别把计数“逼”到显示/隐藏的稳定状态。
	while (ShowCursor(visible) < 0 && visible) {}
	while (ShowCursor(visible) >= 0 && !visible) {}
}

// 开关鼠标锁定。开启时立即把鼠标归位到锁定区域中心。
void dx3d::InputSystem::setCursorLocked(bool locked)
{
	m_cursorLocked = locked;
	// 开启锁定时，立即把鼠标归位到中心。
	if (locked) centerCursor();
}

// 记录鼠标锁定区域（屏幕坐标矩形）。
void  dx3d::InputSystem::setCursorLockArea(const Rect& rect)
{
	m_lockArea = rect;
}

// 把鼠标移到锁定区域 m_lockArea 的几何中心。
// 用于锁定模式：每帧测完位移后把鼠标拉回中心，下一帧继续从中心算偏移。
void dx3d::InputSystem::centerCursor()
{
	const auto centerX = m_lockArea.left + (m_lockArea.width / 2);
	const auto centerY = m_lockArea.top + (m_lockArea.height / 2);

	// SetCursorPos：把鼠标移到屏幕绝对坐标 (centerX, centerY)。
	SetCursorPos(centerX, centerY);

	m_mousePosition.x = static_cast<f32>(centerX);
	m_mousePosition.y = static_cast<f32>(centerY);
}

// 每帧调用一次。核心流程：
//   1) 把本帧按键存为上一帧；2) 轮询所有按键；3) 读鼠标位置并算增量；4) 锁定时归位。
void dx3d::InputSystem::update()
{
	// 第 1 步：把本帧按键状态存为“上一帧”，供下一帧边缘检测。
	m_previousKeys = m_currentKeys;

	// 第 2 步：遍历每个 KeyCode，轮询其当前状态写入 m_currentKeys。
	for (auto i: std::views::iota(0u,static_cast<std::size_t>(KeyCode::Count)))
	{
		// 先把 KeyCode 转成 Windows 虚拟键码 vk。
		const auto vk = getInternalKeyCode(static_cast<KeyCode>(i));
		// GetAsyncKeyState 异步查询某键；最高位(0x8000)为 1 表示当前按下。
		m_currentKeys[i] = (GetAsyncKeyState(vk) & 0x8000) != 0;
	}

	// 第 3 步：记下上一帧鼠标位置，用于计算增量。
	m_previousMousePosition = m_mousePosition;

	POINT point{};
	// GetCursorPos：读取鼠标在屏幕中的当前绝对坐标。
	GetCursorPos(&point);

	m_mousePosition.x = static_cast<f32>(point.x);
	m_mousePosition.y = static_cast<f32>(point.y);

	// 第 3 步(续)：位移增量 = 当前位置 - 上一帧位置。
	m_mouseDelta.x = m_mousePosition.x - m_previousMousePosition.x;
	m_mouseDelta.y = m_mousePosition.y - m_previousMousePosition.y;

	// 第 4 步：若处于锁定模式，把鼠标拉回中心，为下一帧继续测量偏移做准备。
	if (m_cursorLocked) centerCursor();
}

// 把引擎 KeyCode 枚举映射成 Windows 虚拟键码（VK_*）。
// 字母/数字键的虚拟键码恰好等于其 ASCII 码；特殊键用 switch 逐个映射。
short dx3d::InputSystem::getInternalKeyCode(const KeyCode& key)
{
	// 把枚举值转成 int，便于下面做算术偏移。
	const auto value = static_cast<int>(key);
	// A-Z
	// 字母键 A~Z：虚拟键码与大写 ASCII 码相同，故用字符 A 加偏移量。
	if (key >= KeyCode::A && key <= KeyCode::Z) return 'A' + (value - static_cast<int>(KeyCode::A));
	// 0-9
	// 数字键 Num0~Num9：同理，虚拟键码与 ASCII 数字相同。
	if (key >= KeyCode::Num0 && key <= KeyCode::Num9) return '0' + (value - static_cast<int>(KeyCode::Num0));

	// 特殊键（Shift/Esc/方向键/鼠标键等）无连续规律，逐个映射到对应 VK_*。
	switch (key)
	{
	case KeyCode::Shift: return VK_SHIFT;
	case KeyCode::Escape: return VK_ESCAPE;
	case KeyCode::Space: return VK_SPACE;
	case KeyCode::Enter: return VK_RETURN;
	case KeyCode::MouseLeft: return VK_LBUTTON;
	case KeyCode::MouseMiddle: return VK_MBUTTON;
	case KeyCode::MouseRight: return VK_RBUTTON;
	case KeyCode::Up: return VK_UP;
	case KeyCode::Down: return VK_DOWN;
	case KeyCode::Left: return VK_LEFT;
	case KeyCode::Right: return VK_RIGHT;
	default: return 0;
	}
}
