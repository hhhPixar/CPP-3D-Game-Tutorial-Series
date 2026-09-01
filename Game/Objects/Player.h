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
// 文件：Player.h —— 第一人称玩家对象（声明）
// ----------------------------------------------------------------------------
// Player 继承自 dx3d::GameObject，代表"玩家"。它做了两件 GameObject 没做的事：
//   1. onCreate 里给自己挂一个 CameraComponent（相机组件），于是这个物体的
//      位置/旋转就等于"玩家眼睛"的位置/朝向，引擎会用它来生成画面；
//   2. onUpdate 里读鼠标移动改朝向（鼠标转视角）、读 WASD 键改位置（移动）。
// 这就是典型的第一人称（FPS）相机控制。实现见 Player.cpp。
// ============================================================================

#pragma once
#include <DX3D/All.h>


// Player：第一人称玩家。继承 GameObject 以复用"游戏对象 + 组件"机制。
class Player : public dx3d::GameObject
{
	// dx3d_typeid 宏：为这个类注册一个唯一类型 ID，供引擎的类型系统使用
	// （createOrGetComponent<T> 等模板靠它来区分不同组件/对象类型）。
	dx3d_typeid(Player)
public:
	// 构造：传入 GameObjectDesc（含日志、游戏上下文、世界引用等），转交基类。
	explicit Player(const dx3d::GameObjectDesc& desc);
	// 虚析构：override 表示覆盖基类虚函数。本类无额外资源需释放，函数体为空。
	virtual ~Player() override;
protected:
	// onCreate：物体创建时调用一次，本类在此挂载相机组件。
	virtual void onCreate();
	// onUpdate：每帧调用，处理鼠标转向与 WASD 移动。deltaTime 为本帧耗时（秒）。
	virtual void onUpdate(dx3d::f32 deltaTime);
};

