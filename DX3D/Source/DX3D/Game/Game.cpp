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
// Game.cpp —— Game 类的实现，集中体现“子系统组装顺序”。
// 注意各子系统的创建存在依赖关系：后续子系统需要前面已创建的对象引用，
// 因此这里的语句顺序不可随意调换。各 Desc 结构（见 Core/Common.h）本质上
// 是把已建好对象的引用打包传入，避免在构造函数里做复杂查找。
// =============================================================================

#include <DX3D/Game/Game.h>
#include <DX3D/Window/Window.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Input/InputSystem.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Resource/ResourceManager.h>


// 构造函数：按依赖顺序逐个创建子系统。desc 携带窗口大小与日志级别两项配置。
dx3d::Game::Game(const GameDesc& desc)
{
	// 第一步：日志器最先创建，因为后续所有子系统构造时都可能要写日志。
	// desc.logLevel 控制输出级别（Error/Info/Debug 等）。
	m_logger = std::make_unique<Logger>(desc.logLevel);	

	// 打印启动横幅，方便确认引擎已开始运行。
	DX3DLogInfo("PardCode | C++ 3D Game Tutorial Series");
	DX3DLogInfo("--------------------------------------");

	// 第二步：输入系统，依赖日志器。负责采集键鼠状态。
	m_inputSystem = std::make_unique<InputSystem>(InputSystemDesc{ *m_logger });
	// 第三步：图形设备（D3D11 设备），用 shared_ptr 因为它会被多处共享持有。
	m_graphicsDevice = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ *m_logger });
	// 第四步：显示窗口，继承自 Window 并额外创建交换链（SwapChain）。
	// 传入日志器、窗口大小与图形设备，供其内部建立 D3D 的呈现链路。
	m_display = std::make_unique<Display>(DisplayDesc{ {*m_logger,desc.windowSize},*m_graphicsDevice });
	
	// 第五步：资源管理器，负责加载贴图/网格/材质。它依赖图形设备来创建 GPU 资源。
	auto context = SystemContext{ *m_graphicsDevice };
	m_resourceManager = std::make_unique<ResourceManager>(ResourceManagerDesc{ {*m_logger},context });

	// 第六步：游戏世界（World）—— ECS 中的“容器”，存放所有实体与组件。
	// GameContext 把输入/资源管理器/图形设备打包，供实体内部按需访问引擎服务。
	m_world = std::make_unique<World>(WorldDesc{ BaseDesc{*m_logger}, GameContext{*m_inputSystem, *m_resourceManager,*m_graphicsDevice} });
	// 第七步：世界渲染器，负责把 World 里的 ECS 数据转成 GPU 绘制命令。依赖图形设备。
	m_worldRenderer = std::make_unique<WorldRenderer>(WorldRendererDesc{ {*m_logger},*m_graphicsDevice });

	// 把鼠标光标锁定在窗口客户区内，避免在锁定鼠标操作时指针移出窗口。
	m_inputSystem->setCursorLockArea(m_display->getClientAreaInScreenSpace());

	DX3DLogInfo("Game initialized.");
}

// 析构：记录关闭日志。成员按声明逆序自动析构（unique_ptr 会释放各子系统）。
dx3d::Game::~Game()
{
	DX3DLogInfo("Game is shutting down...");
}

// 返回世界引用。子类游戏逻辑通过它创建实体、查询组件。
dx3d::World& dx3d::Game::getWorld() noexcept
{
	return *m_world;
}

// 返回日志器。const 因为日志器本身不需要被修改。
dx3d::Logger& dx3d::Game::getLogger() const noexcept
{
	return *m_logger;
}

// 返回输入系统引用，供子类读取按键/鼠标。
dx3d::InputSystem& dx3d::Game::getInputSystem() noexcept
{
	return *m_inputSystem;
}

// 返回资源管理器引用，供子类加载贴图/网格等。
dx3d::ResourceManager& dx3d::Game::getResourceManager() noexcept
{
	return *m_resourceManager;
}

// 每帧内部流水线：固定了“输入 → 用户逻辑 → 世界更新 → 渲染”的顺序。
// 这种分层让框架统一控制流程，用户只负责填 onUpdate 的内容。
void dx3d::Game::onInternalUpdate()
{
	// 取当前时间，与上一帧相减得到本帧耗时，再更新“上一帧”为当前时间。
	// steady_clock 单调递增，保证 deltaTime 始终为正且不受系统时钟跳变影响。
	auto currentTime = std::chrono::steady_clock::now();
	// duration<f32> 把时间差转成以秒为单位的浮点数，便于游戏逻辑直接使用。
	std::chrono::duration<f32> delta = currentTime - m_previousTime;
	m_previousTime = currentTime;
	// count() 取出秒数浮点值。这就是传给所有 update 方法的 deltaTime。
	auto deltaTime = delta.count();

	// 1) 更新输入：采集本帧键盘/鼠标状态，必须在游戏逻辑之前，这样逻辑读到的是最新输入。
	m_inputSystem->update();

	// 2) 调用子类游戏逻辑。这里把 deltaTime 传下去，使移动速度与帧率无关。
	onUpdate(deltaTime);

	// 3) 更新世界：处理延迟创建的实体、调用各实体 onUpdate、批量重算脏的 Transform 矩阵。
	m_world->update(deltaTime);

	// 4) 渲染：把 World 里的相机/光源/网格组件数据转成 GPU 绘制命令并呈现到屏幕。
	m_worldRenderer->render(*m_world, m_display->getSwapChain(), deltaTime);
}
