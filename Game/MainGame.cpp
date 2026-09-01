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
// 文件：MainGame.cpp —— 游戏示例的实现（搭场景 + 动灯光）
// ----------------------------------------------------------------------------
// 本文件演示"如何用 DX3D 引擎搭一个 3D 场景"，是初学者最先该读的部分。
// 引擎采用"游戏对象 + 组件"（GameObject + Component，类似 ECS）的模式：
//   - World（世界）是容器，调用 world.createGameObject<T>() 生成一个物体；
//   - 给物体用 createOrGetComponent<XxxComponent>() 挂载组件（Component），
//     组件决定物体的"能力"：能被渲染的形状（CubeComponent / MeshComponent）、
//     发光（DirectionaLightComponent）、当相机（CameraComponent）等；
//   - TransformComponent（变换组件）每个物体自带，控制位置/旋转/缩放。
// 资源（纹理 Texture、网格 Mesh、材质 Material/Shader）由 ResourceManager 从
// 磁盘文件加载，再绑定到组件上。本文件 onCreate() 搭四样东西：
//   1) 红砖立方体台座；2) 大理石半身像；3) 白色方向光；4) 第一人称玩家。
// ============================================================================

#include "MainGame.h"
#include "Objects/Player.h"


// 构造函数：把 desc 转交给基类 dx3d::Game 完成引擎初始化。
// : dx3d::Game(desc) 是成员初始化列表，先构造基类，基类构造期间会创建
// 窗口、显卡设备等。本类自身没有需要额外初始化的成员，故函数体为空。
MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

// onCreate：引擎就绪后由基类调用一次。本方法搭建整个场景。
// 读法提示：下面用 { } 把每个物体的创建代码分块，便于理解。
void MainGame::onCreate()
{
	// 先调用基类 onCreate，确保引擎自身初始化完成，再开始搭我们的场景。
	Game::onCreate();
	// 取得世界（World）的引用。World 是所有游戏对象的容器，新物体都从它创建。
	auto& world = getWorld();

	//pedestal
	// —— 台座：一个带红砖纹理的立方体，无高光（哑光/磨砂效果）。
	{
		// 从磁盘加载纹理图片（红砖贴图），返回 TextureResource。
		auto brickTex = getResourceManager().createResourceFromFile<dx3d::TextureResource>(L"Game/Assets/Textures/red_brick_03_diff_1k.jpg");
		// 从磁盘加载材质（MaterialResource）。材质本质是一份 .hlsl 着色器（Shader）
		// 程序，告诉 GPU 如何绘制表面（如何采样纹理、如何响应光照）。
		auto pedestalMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>(L"Game/Assets/Shaders/MaterialShader.hlsl");
		// 检查材质是否加载成功（失败时可能返回空，避免空指针崩溃）。
		if (pedestalMat)
		{
			// 给材质传入一个"高光强度"参数 spec=0.0：0 表示完全哑光，没有镜面高光。
			// setData 用字节视图（as_bytes + span）把一个 float 原样拷给着色器常量。
			float spec = 0.0f;
			pedestalMat->setData(std::as_bytes(std::span(&spec, 1)));
			// 把红砖纹理绑定到材质的第 0 号纹理槽（采样器），着色器采样时会取它。
			pedestalMat->setTexture(0, brickTex);
		}

		// 在世界里创建一个普通游戏对象（GameObject）作为台座。
		auto pedestal = world.createGameObject<dx3d::GameObject>();
		// 给台座挂一个 CubeComponent（立方体组件）：引擎内置的简单几何体，
		// 不需要外部网格文件，自带 6 面立方体顶点。第一次调用会真正创建组件。
		pedestal->createOrGetComponent<dx3d::CubeComponent>();
		// 再取一次该组件的指针（此时已存在，直接返回），用来设材质。
		auto comp = pedestal->createOrGetComponent<dx3d::CubeComponent>();
		comp->setMaterial(pedestalMat);
		// 设置缩放为 2 倍（xyz 都放大到 2），让台座大一点。
		pedestal->getTransform().setScale({ 2, 2, 2 });
		// 放到 y=-1（向下），使台座顶面大约在地面附近，半身像摆在它上面。
		pedestal->getTransform().setPosition({ 0, -1, 0 });
	}
	
	//marble bust
	// —— 大理石半身像：用外部 .obj 网格 + 大理石纹理，有高光（光滑反光）。
	{
		// 加载大理石纹理。
		auto marbleBustTex = getResourceManager().createResourceFromFile<dx3d::TextureResource>(L"Game/Assets/Textures/marble_bust_01_diff_1k.jpg");
		// 加载 .obj 网格文件（MeshResource）：存放了半身像的顶点/法线/UV 数据。
		// 与 CubeComponent 不同，MeshComponent 用任意外部模型，能表现复杂形状。
		auto marbleBustMesh = getResourceManager().createResourceFromFile<dx3d::MeshResource>(L"Game/Assets/Meshes/marble_bust_01.obj");
		// 同样用 MaterialShader.hlsl 材质，但参数不同：大理石有光泽。
		auto marbleBustMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>(L"Game/Assets/Shaders/MaterialShader.hlsl");
		if (marbleBustMat) 
		{
			// spec=1.0：高光强度拉满，大理石表面会反射出明亮的高光斑点。
			float spec = 1.0f;
			marbleBustMat->setData(std::as_bytes(std::span(&spec, 1)));
			marbleBustMat->setTexture(0, marbleBustTex);
		}
		// 创建物体并挂 MeshComponent（网格组件），用来渲染任意外部模型。
		auto mesh = world.createGameObject<dx3d::GameObject>();
		auto comp = mesh->createOrGetComponent<dx3d::MeshComponent>();
		// 把 .obj 网格绑给组件；setMaterial(0, ...) 把材质绑到第 0 号子网格。
		// （一个模型可含多个子网格，每个可单独贴材质，这里只有 0 号。）
		comp->setMesh(marbleBustMesh);
		comp->setMaterial(0, marbleBustMat);
		// 缩放 4 倍并放在原点 (0,0,0)，正好立在台座上方。
		mesh->getTransform().setScale({ 4, 4, 4 });
		mesh->getTransform().setPosition({ 0, 0, 0 });
	}

	//white light
	// —— 白色方向光：模拟太阳，所有被照物体会被它照亮。
	{
		// 创建灯光物体。
		auto light = world.createGameObject<dx3d::GameObject>();
		// 保存到成员 m_whiteLight，以便 onUpdate 每帧改它的旋转。
		m_whiteLight = light;
		// 挂载方向光组件。注意类名拼写是 DirectionaLightComponent
		// （少了结尾的 l），这是引擎里的既定写法，照用即可。
		light->createOrGetComponent<dx3d::DirectionaLightComponent>();
		auto comp = light->createOrGetComponent<dx3d::DirectionaLightComponent>();
		// 设灯光颜色为白色 {R=1,G=1,B=1}（各分量 0~1）。
		comp->setColor({ 1,1,1 });
		// 方向光本身没有"位置"，只有"方向"——方向由物体的旋转决定。
		// 这里设旋转 {0.707, 0, 0}：绕 X 轴转 0.707 弧度（约 40.5°），
		// 使光线斜向下照射，立体感更强。onUpdate 会再叠加绕 Y 轴的旋转。
		light->getTransform().setRotation({0.707f,0.0f,0 });
	}

	//player
	// —— 第一人称玩家：自带相机，可用鼠标转向、WASD 移动。
	{
		// 创建我们自定义的 Player 对象（见 Objects/Player.cpp）。
		// createGameObject<Player> 用模板生成 Player 子类实例。
		auto player = world.createGameObject<Player>();
		// 放在 (0, 1, -2)：站在台座前方、离地 1 单位、后退 2 单位观察半身像。
		player->getTransform().setPosition({ 0, 1, -2 });

		// 锁定鼠标在窗口内并隐藏光标，实现第一人称视角操作：
		// 移动鼠标不再移动光标，而是改变视角（Player::onUpdate 里读取鼠标 delta）。
		getInputSystem().setCursorLocked(true);
		getInputSystem().setCursorVisible(false);
	}
}


// onUpdate：每帧调用。deltaTime 是上一帧到本帧的耗时（秒）。
// 本方法让方向光持续绕 Y 轴旋转，产生"太阳移动"的光照变化效果。
void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	// 先调用基类 onUpdate（引擎内部更新，如输入轮询）。
	Game::onUpdate(deltaTime);
	// 累加旋转角度：0.57 弧度/秒（约 32.7°/秒）乘以本帧耗时。
	// 用 deltaTime 相乘保证"每秒转固定角度"，与帧率高低无关——
	// 这是做动画的标准写法（frame-rate independent）。
	m_roty += 0.57f * deltaTime;
	// 把新的旋转赋给灯光：X 轴仍保持 0.707（斜向下不变），
	// Y 轴用累计的 m_roty，于是灯光绕 Y 轴匀速转圈，照向不断变化。
	m_whiteLight->getTransform().setRotation(dx3d::Vec3(0.707f, m_roty, 0));
}
