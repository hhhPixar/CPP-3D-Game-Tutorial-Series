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
// 文件：Player.cpp —— 第一人称玩家的实现（相机 + 鼠标转向 + WASD 移动）
// ----------------------------------------------------------------------------
// 本文件是学习"3D 中第一人称控制怎么做"的最佳入口，核心是两段向量数学：
//
//  【转向】鼠标移动量（delta）→ 物体旋转角度。
//     鼠标向左右移（delta.x）对应"转头"（绕 Y 轴，yaw）；
//     鼠标向上下移（delta.y）对应"抬头/低头"（绕 X 轴，pitch）。
//     pitch 必须限制在 ±90°（约 1.57 弧度）内，否则视角会翻转，人会"扭脖子"。
//
//  【移动】按键 → 前进/右移标量 → 用物体当前朝向算出方向向量 → 移动位置。
//     forward()/right() 给出物体在世界里的"前"和"右"方向（已含旋转）。
//     把"前向量×前进量 + 右向量×右移量"合成后归一化（normalize），
//     这样斜向走（同时按 W+D）速度不会比直走快一倍。
//     最后 位置 += 方向 × 速度 × 本帧耗时，保证移动速度与帧率无关。
// ============================================================================

#include "Player.h"

// 构造：转交 desc 给基类 GameObject，基类会保存世界引用、游戏上下文等。
Player::Player(const dx3d::GameObjectDesc& desc) : dx3d::GameObject(desc)
{
}

// 析构：本类无需额外清理（组件由 GameObject 统一管理生命周期），故为空。
Player::~Player()
{
}

// onCreate：物体生成时调用一次。给玩家挂上相机组件，从此这个物体就是"眼睛"。
void Player::onCreate()
{
	// createOrGetComponent<CameraComponent>：若还没有相机组件就创建一个。
	// 有了它，引擎渲染时取该相机的视图矩阵（view）与投影矩阵（projection）
	// 来决定"从玩家眼睛看出去"的画面。相机的位置/朝向跟随本物体的 Transform。
	createOrGetComponent<dx3d::CameraComponent>();
}

// onUpdate：每帧调用，实现鼠标转视角 + WASD 移动。deltaTime 为本帧耗时（秒）。
void Player::onUpdate(dx3d::f32 deltaTime)
{
	// 取输入系统引用（本行变量后续未直接使用，下面仍用 getInputSystem() 调用）。
	auto& input = getInputSystem();

	// sensitivity：鼠标灵敏度。鼠标 delta 通常几十~几百像素，乘 0.001 后
	// 换算成很小的弧度，避免稍微一动就转飞。
	auto sensitivity = 0.001f;
	// 取当前旋转（弧度）。Vec3 的 x/y/z 在这里对应绕 X/Y/Z 轴的旋转角：
	//   x = pitch（俯仰，抬头/低头），y = yaw（偏航，左转/右转），z = roll（翻滚，这里不用）。
	auto rot = getTransform().getRotation();
	// 鼠标上下移动（delta.y）改 pitch（绕 X 轴）。注意用 += 且符号为正：
	// 鼠标上移 delta.y 为正 → rot.x 增大 → 抬头向上看。
	rot.x += getInputSystem().getMouseDelta().y * sensitivity;
	// 鼠标左右移动（delta.x）改 yaw（绕 Y 轴）。
	rot.y += getInputSystem().getMouseDelta().x * sensitivity;
	// 限制 pitch 不超过 ±1.57 弧度（约 ±90°）。若不限制，继续抬头会让视角
	// 翻过去变成"倒着看"，像扭断了脖子，这是第一人称相机常见的处理。
	if (rot.x > 1.57f) rot.x = 1.57f;
	else if (rot.x < -1.57f) rot.x = -1.57f;
	// 把新旋转写回 Transform，相机朝向随之改变。
	getTransform().setRotation(rot);

	// ===== 移动：用按键决定"想往哪走"，再用朝向算出世界方向 =====
	// 取当前位置，后面累加位移。
	auto pos = getTransform().getPosition();
	// forward/right 是两个标量（不是向量）：+1 表示要朝该方向走，-1 反向，0 不走。
	auto forward = 0.0f;
	auto right = 0.0f;
	// speed：移动速度（单位/秒）。后面乘 deltaTime 换算成"本帧该走多远"。
	auto speed = 3.0f;
	// 读 WASD：W 前进(+1)，S 后退(-1)；D 右移(+1)，A 左移(-1)。
	// 同时按 W 和 S 会互相抵消（forward 先设 1 再设 -1），符合直觉。
	if (getInputSystem().isKeyDown(dx3d::KeyCode::W)) forward = 1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::S)) forward = -1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::D)) right = 1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::A)) right = -1.0f;
	// getTransform().forward() 返回物体"当前正前方"的单位向量（已含旋转），
	// 乘以 forward 标量后得到"想往前走多远"的向量。right() 同理是右方向量。
	auto forwardDir = getTransform().forward() * forward;
	auto rightDir = getTransform().right() * right;
	// 把前向量与右向量相加得到合成方向，再 normalize（归一化）成长度 1。
	// 为什么要归一化：斜向走时 forwardDir+rightDir 的长度会大于 1（约 1.414），
	// 不归一化的话斜走会比直走快约 41%，归一化后各方向速度一致。
	auto direction = dx3d::Vec3::normalize(forwardDir + rightDir);
	// 位置 += 方向 × 速度 × 本帧耗时。
	// 乘 deltaTime 让"每秒走 speed 单位"与帧率无关——60fps 帧少走、30fps 帧多走，
	// 但每秒总位移相同，这是游戏移动的标准写法。
	pos = pos + direction * speed * deltaTime;
	// 写回新位置，玩家就此移动一帧。
	getTransform().setPosition(pos);
}
