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
// 文件：main.cpp —— 整个游戏的程序入口（Entry Point）
// ----------------------------------------------------------------------------
// 本文件是 DX3D 引擎示例游戏的可执行程序入口。它只做三件事：
//   1. 创建 MainGame 实例（在 try 块里），传入窗口大小与日志级别；
//   2. 调用 game.run() 启动主循环（渲染 + 输入 + 更新），直到窗口关闭；
//   3. 用 try/catch 兜底所有可能的异常，任何异常都让进程以失败码退出。
// 初学者提示：3D 引擎里有很多可能失败的环节（创建窗口、加载显卡设备、
// 读资源文件等），它们会抛 C++ 异常（exception）。main 必须接住它们，
// 否则程序崩溃时用户会看到系统弹窗，体验很差。这就是本文件的核心职责。
// ============================================================================

#include "MainGame.h"


// main 函数：操作系统加载这个 exe 后，从这里开始执行。
// 返回 int：0 表示成功(EXIT_SUCCESS)，非 0 表示失败(EXIT_FAILURE)，
// 操作系统/调用方会读到这个返回值。
int main()
{
	// try 块：把"可能出问题"的代码包起来。如果 try 内部抛出异常
	// （throw），程序不会直接崩溃，而是跳到后面匹配的 catch。
	try
	{
		// 构造 MainGame 对象。这里传入的是 dx3d::GameDesc 结构体：
		//   {{1280, 720}}        —— 窗口客户区宽高（像素），Rect 是 {width,height}
		//   dx3d::Logger::LogLevel::Info —— 日志级别，Info 会打印较详细的信息
		// 构造时引擎会创建窗口、初始化 Direct3D 11 显卡设备、输入系统等。
		// 这些步骤任一失败都会抛异常，被下面的 catch 捕获。
		MainGame game({ {1280,720},dx3d::Logger::LogLevel::Info });
		// run() 进入引擎主循环：每帧处理输入、调用 onUpdate、渲染一帧画面，
		// 直到窗口被关闭或发生错误。正常情况下 run() 返回时游戏结束。
		game.run();
	}
	// 以下逐层捕获异常：从具体到宽泛。
	// std::runtime_error：运行期错误，如显卡设备创建失败、资源加载失败。
	catch (const std::runtime_error&)
	{
		return EXIT_FAILURE;
	}
	// std::invalid_argument：参数非法，如传入了无效的窗口尺寸或路径。
	catch (const std::invalid_argument&)
	{
		return EXIT_FAILURE;
	}
	// std::exception：上面两类异常的基类，兜住所有标准库异常。
	catch (const std::exception&)
	{
		return EXIT_FAILURE;
	}
	// catch(...)：捕获任意类型异常（含非标准异常），是最后的兜底。
	catch (...)
	{
		return EXIT_FAILURE;
	}

	// 若 try 块正常结束（没有抛异常），走到这里返回成功码。
	return EXIT_SUCCESS;
}