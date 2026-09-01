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
// 文件：MainGame.h —— 游戏示例的头文件（声明）
// ----------------------------------------------------------------------------
// 本文件声明 MainGame 类，它是 dx3d::Game（引擎游戏基类）的具体子类。
// dx3d::Game 已经封装了窗口、Direct3D 11 显卡设备、输入系统、资源管理器、
// 世界（World）和渲染器等子系统。我们要做的只是继承它，重写两个回调：
//   onCreate()  —— 引擎初始化完毕后调用一次，用来"搭场景"（摆物体、贴材质、
//                  放灯光、生成玩家）；
//   onUpdate()  —— 每帧调用，用来"动场景"（如让灯光旋转）。
// 本文件只做声明，实现见 MainGame.cpp。include <DX3D/All.h> 会把引擎全部
// 公共头文件一次性引入，方便示例代码编写。
// ============================================================================

#pragma once
#include <DX3D/All.h>


// MainGame：用 DX3D 引擎搭出的示例游戏。
// 继承 dx3d::Game 后，只需重写 onCreate / onUpdate 即可定义自己的场景与逻辑，
// 不必关心底层窗口、渲染、输入的细节——这正是"引擎"的意义。
class MainGame : public dx3d::Game
{
public:
	// 构造函数：explicit 防止隐式转换。传入 GameDesc（含窗口尺寸、日志级别），
	// 转交给基类 dx3d::Game 完成引擎初始化。
	explicit MainGame(const dx3d::GameDesc& desc);
protected:
	// onCreate：引擎就绪后调用一次，本类在此搭建整个场景（详见 .cpp）。
	virtual void onCreate();
	// onUpdate：每帧调用，deltaTime 是上一帧到本帧的耗时（秒），用于做
	// 与帧率无关（frame-rate independent）的动画。本类用它旋转方向光。
	virtual void onUpdate(dx3d::f32 deltaTime);

private:
	// m_whiteLight：保存方向光物体（GameObject）的原始指针，以便在 onUpdate
	// 每帧修改它的旋转。注意：它指向 World 内部管理的对象，本类不负责释放。
	dx3d::GameObject* m_whiteLight{};
	// m_roty：方向光绕 Y 轴的累计旋转角度（弧度）。每帧累加，持续转动灯光。
	dx3d::f32 m_roty = 0;
};

