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
// 文件：Win32Window.cpp —— Window 子系统的 Windows（Win32）实现
// 职责：用 Win32 API 实现 Window.h 声明的平台无关接口，完成三件事：
//   1) 注册窗口类并创建原生窗口（RegisterClassEx / CreateWindowEx）；
//   2) 提供窗口过程 WindowProcedure（WndProc），处理发给本窗口的系统消息；
//   3) 查询客户区在屏幕坐标系下的矩形（供鼠标锁定区域使用）。
// 关键概念：
//   · 消息驱动：Windows 用"消息（Message）"通知窗口发生的事件（按键、
//     鼠标、关闭、重绘…）。消息经"消息循环"分发到"窗口过程"。本文件只
//     实现窗口过程；真正抽取并分发消息的循环（PeekMessage/DispatchMessage）
//     在 Win32Game.cpp 的 Game::run() 里。
//   · 窗口类（WNDCLASSEX）：创建窗口前必须先注册一个"类"，指定类名与
//     窗口过程；CreateWindowEx 再依据该类创建窗口实例。
// ============================================================
#include <DX3D/Window/Window.h>
#include <Windows.h>
#include <stdexcept>

// 窗口过程（Window Procedure，WndProc）。Windows 在窗口收到消息时回调本函数：
// 每发生一个事件（按键、鼠标、关闭、重绘…），就投递一条消息进来由它处理。
// 参数：hwnd=消息所属窗口句柄；msg=消息类型（如 WM_CLOSE）；
//       wparam/lparam=随消息附带的数据，不同消息含义不同。
// 返回 LRESULT（处理结果）；未处理的消息交给 DefWindowProc 兜底。
static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// 依据"消息类型 msg"分别处理。
	switch (msg)
	{
	// WM_CLOSE：用户点了窗口右上角"X"或按 Alt+F4，请求关闭窗口。
	case WM_CLOSE:
	{
		// 投递 WM_QUIT（退出码 0）到本线程消息队列。
		// 消息循环（Game::run）取到 WM_QUIT 后会跳出循环、结束整个程序——
		// 因此在本程序中，"关窗口"等同于"退出游戏"。
		PostQuitMessage(0);
		break;
	}
	default:
		// 默认分支：本函数只处理 WM_CLOSE，其余消息（重绘、鼠标移动等）
		// 一律交给 Windows 自带的 DefWindowProc 处理，否则窗口会失去响应。
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

// 构造函数（Windows 实现）。流程：注册窗口类 → 计算外框尺寸 → 创建窗口 → 显示。
// 参数 desc：base=日志器，size=期望的客户区尺寸。
// 成员初始化列表先初始化基类 Base 与 m_size。
dx3d::Window::Window(const WindowDesc& desc) : Base(desc.base), m_size(desc.size)
{
	// 用一个 lambda 封装"注册窗口类"这一步，便于稍后用 std::invoke 只执行一次。
	auto registerWindowClassFunction = []()
		{
			// WNDCLASSEX：描述"窗口类"的结构。窗口必须先属于一个类，
			// 类里最关键的是 lpfnWndProc——指向本窗口的窗口过程函数。
			WNDCLASSEX wc{};
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.lpszClassName = L"DX3DWindow";
			// 把本类的窗口过程指向上面定义的 WindowProcedure：
			// 这样该类创建出的所有窗口，其消息都会送到 WindowProcedure 处理。
			wc.lpfnWndProc = &WindowProcedure;
			// 向 Windows 注册此类。成功返回一个"原子"（atom，可当作该类的标识）。
			return RegisterClassEx(&wc);
		};


	// 用 std::invoke 执行上面的 lambda，并用 static 保证整个程序只注册一次。
	// windowClassId 即注册返回的原子标识，后续 CreateWindowEx 要用到它。
	static const auto windowClassId = std::invoke(registerWindowClassFunction);


	if (!windowClassId)
		DX3DLogThrowError("RegisterClassEx failed.");
	
	// 先按"期望的客户区尺寸"构造一个 RECT（左上 0,0，右下=宽高）。
	RECT rc{ 0,0,m_size.width, m_size.height };
	// AdjustWindowRect：依据期望的"客户区"矩形，反推出所需的"外框"矩形
	// （加上标题栏/边框）。因为 CreateWindowEx 要的是外框尺寸，而我们指定的是客户区尺寸。
	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

	// CreateWindowEx：依据窗口类（用 MAKEINTATOM 把原子转回类标识）真正创建窗口，
	// 返回窗口句柄 HWND 存入 m_handle。样式 WS_CAPTION|WS_SYSMENU 表示有标题栏与系统菜单（关闭按钮）。
	// 注意尺寸用 AdjustWindowRect 算出的外框尺寸 rc，而非客户区尺寸 m_size。
	m_handle = CreateWindowEx(NULL, MAKEINTATOM(windowClassId), L"PardCode | C++ 3D Game Tutorial Series",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, NULL, NULL);

	if (!m_handle)
		DX3DLogThrowError("CreateWindowEx failed.");

	// 让窗口显示出来（SW_SHOW）。至此窗口创建完成、可见。
	ShowWindow(static_cast<HWND>(m_handle), SW_SHOW);
}


// 获取客户区在"屏幕坐标系"下的矩形。
// 客户区是窗口内不含标题栏/边框的绘图区；返回的 left/top 是它在屏幕中的位置，
// width/height 是尺寸。Game 用这个矩形作为鼠标锁定区域（setCursorLockArea）。
dx3d::Rect dx3d::Window::getClientAreaInScreenSpace()
{
	auto hwnd = static_cast<HWND>(m_handle);
	
	RECT client{};
	// GetClientRect：获取客户区矩形，但坐标是"相对窗口左上角"的（left/top 通常为 0）。
	GetClientRect(hwnd, &client);

	POINT topLeft{ client.left, client.top };
	POINT bottomRight{ client.right, client.bottom };
	// ClientToScreen：把"窗口相对坐标"换算成"屏幕绝对坐标"。
	// 分别转换左上角和右下角两个点，即可拼出客户区在屏幕上的矩形。
	ClientToScreen(hwnd, &topLeft);
	ClientToScreen(hwnd, &bottomRight);

	return { 
		topLeft.x , 
		topLeft.y , 
		bottomRight.x - topLeft.x, 
		bottomRight.y - topLeft.y 
	};
}




// 析构函数（Windows 实现）：销毁原生窗口、释放其 HWND 资源。
dx3d::Window::~Window()
{
	// DestroyWindow：销毁窗口并释放其句柄。调用后 m_handle 不再有效。
	DestroyWindow(static_cast<HWND>(m_handle));
}
