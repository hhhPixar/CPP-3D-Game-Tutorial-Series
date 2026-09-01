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
// 所属子系统：Game（游戏框架层）—— 整个引擎的顶层入口与组装器。
// 职责：创建并持有全部子系统（Logger/InputSystem/GraphicsDevice/Display
//       /ResourceManager/World/WorldRenderer），驱动每帧的更新与渲染。
// 架构位置：这是用户继承的基类。子类覆盖 onCreate/onUpdate 写游戏逻辑，
//           调用 run() 即进入平台消息循环（Win32Game.cpp 提供 Win32 实现）。
// 关键概念：
//   - 帧循环（Game Loop）：run() 循环处理窗口消息 → onInternalUpdate() 算
//     帧间隔 deltaTime → 输入更新 → 用户游戏逻辑 → 世界更新 → 世界渲染。
//   - deltaTime：本帧与上一帧的时间差（秒），让游戏逻辑与帧率解耦。
//   - final 方法：getWorld/run 等标记 final，子类不可改框架骨架，只能填逻辑。
// =============================================================================

#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>
#include <chrono>

namespace dx3d
{
	// Game 是游戏的根对象。它聚合所有引擎子系统，并提供可被子类覆盖的生命周期钩子。
	// 用户通常写一个 class MyGame : public dx3d::Game，在 onCreate 里创建实体、
	// 在 onUpdate 里移动它们，main() 里只需 game.run() 即可跑起来。
	class Game
	{
		// 禁用拷贝与移动：Game 是独占的顶层对象，语义上不应被复制或转移所有权。
		dx3d_disable_copy_and_move(Game)
	public:
		// 构造：按 GameDesc（窗口大小、日志级别）创建并连接全部子系统。
		// 依赖顺序很关键（见 Game.cpp）：先 Logger，再 Input/Device/Display，
		// 最后 ResourceManager/World/WorldRenderer，后者依赖前面的对象。
		explicit Game(const GameDesc& desc);
		// 析构：声明 virtual 以保证通过 Game* 删除子类对象时调用到子类析构。
		virtual ~Game();

		// 以下四个 getter 都是 final：子类不能改写框架结构，但可用它们访问子系统。
		// 获取当前游戏世界（World），世界里存放所有 GameObject 与 Component。
		virtual World& getWorld() noexcept final;
		// 获取日志器，子类游戏逻辑可借它输出诊断信息。
		virtual Logger& getLogger() const noexcept final;
		// 获取输入系统，用于读取键鼠状态。
		virtual InputSystem& getInputSystem() noexcept final;
		// 获取资源管理器，用于加载贴图/网格/材质等 GPU 资源。
		virtual ResourceManager& getResourceManager() noexcept final;

		// 进入平台消息循环。这是阻塞调用，直到窗口被关闭才返回。
		// 具体实现由平台层提供（Win32 下见 Win32Game.cpp）。
		virtual void run() final;
	protected:
		// 生命周期钩子：游戏初始化完成、进入主循环前调用一次。子类在此创建
		// GameObject、加载资源等。空默认实现允许子类按需覆盖。
		virtual void onCreate() {}
		// 生命周期钩子：每帧调用，deltaTime 为本帧耗时（秒）。子类在此推进
		// 游戏逻辑（移动、碰撞、生成实体等）。空默认实现允许子类按需覆盖。
		virtual void onUpdate(f32 deltaTime) {}
	private:
		// 每帧实际执行的内部流水线。它不是 virtual，固定了帧流程：
		// 算 deltaTime → 输入更新 → 调子类 onUpdate → 世界更新 → 渲染。
		// 把“框架流程”与“用户逻辑”分离：框架控制顺序，用户只填内容。
		void onInternalUpdate();
	private:
		// 以下为 Game 拥有的全部子系统，按所有权语义选择智能指针类型：
		//   UniquePtr = 独占所有权（Game 销毁时它们也销毁）；
		//   RefPtr(shared_ptr) = 共享所有权，这里因为 GraphicsDevice 会被
		//   资源管理器/交换链等多处共享持有，故用 shared_ptr。
		UniquePtr<Logger> m_logger{};
		UniquePtr<InputSystem> m_inputSystem{};
		RefPtr<GraphicsDevice> m_graphicsDevice{};
		UniquePtr<Display> m_display{};
		UniquePtr<ResourceManager> m_resourceManager{};
		UniquePtr<World> m_world{};	
		
		UniquePtr<WorldRenderer> m_worldRenderer{};

		// 主循环运行标志。收到 WM_QUIT 等退出消息时置 false，使 run() 的循环结束。
		bool m_isRunning{ true };

		// 上一帧的时间戳，用于计算 deltaTime。steady_clock 单调递增，
		// 不受系统时间调整影响，适合做帧间隔计时。
		std::chrono::steady_clock::time_point m_previousTime{};
	};
}
