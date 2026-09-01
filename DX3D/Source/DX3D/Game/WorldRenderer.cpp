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
// WorldRenderer.cpp —— 渲染一帧的完整实现。
// 数据流（每帧固定顺序）：
//   1) clearAndSetBackBuffer：用背景色清后缓冲并把后缓冲设为渲染目标。
//   2) 环境（方向光）：遍历 DirectionalLight 组件，把强度/方向/颜色写入 envCb。
//   3) 相机：遍历 Camera 组件，把视图/投影矩阵与位置写入 cameraCb。
//   4) 立方体：遍历 Cube 组件，按其材质设置管线状态与纹理，下发绘制命令。
//   5) 网格：遍历 Mesh 组件，按材质槽（MaterialSlot）逐段绘制。
//   6) executeCommandList：把整帧录制的命令交给 GPU 执行。
//   7) present：把后缓冲翻到屏幕（垂直同步翻页）。
// 现代 C++ 用法：
//   - std::views::iota(0u, n)：生成 [0,n) 的整数序列，做索引循环，免去裸 for。
//   - std::span<T>：不持有所有权的连续数组视图，给 API 传“首指针+长度”更安全。
//   - std::as_bytes(std::span{&x,1})：把结构体内存视作字节序列，
//     便于常量缓冲按原始字节上传 GPU（GPU 不关心 C++ 类型，只认字节布局）。
// =============================================================================

#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Game/GameObject.h>

#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/DirectionalLightComponent.h>

#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/TextureResource.h>
#include <DX3D/Resource/MeshResource.h>

#include <DX3D/Math/Vec3.h>
#include <fstream>
#include <ranges>


// 构造：创建本渲染器长期复用的 GPU 资源（上下文、常量缓冲、采样器）。
dx3d::WorldRenderer::WorldRenderer(const WorldRendererDesc& desc): Base(desc.base), m_graphicsDevice(desc.engine)
{
	auto& device = m_graphicsDevice;
	// 设备上下文：录制并执行绘制命令的接口（D3D11 立即上下文封装）。
	m_deviceContext = device.createDeviceContext();

	// 预留纹理指针容器容量，避免每帧反复分配/释放小内存。
	m_textures.reserve(32);

	// 四个常量缓冲：按各自结构大小创建。创建后内容为空，每帧 update 时填充。
	m_objectCb = device.createConstantBuffer({ {}, sizeof(ObjectData) });
	m_cameraCb = device.createConstantBuffer({ {}, sizeof(CameraData) });
	m_envCb = device.createConstantBuffer({ {}, sizeof(EnvironmentData) });
	// 材质缓冲用最大尺寸，因不同材质数据大小不一，取最大值容纳所有情况。
	m_materialCb = device.createConstantBuffer({ {}, dx3d::MaterialResource::MaxDataSize });

	// 采样器：定义贴图采样方式（过滤模式、寻址模式），创建一次长期复用。
	m_sampler = device.createSampler({});
}

// 每帧渲染主函数。world 提供数据，swapChain 提供呈现目标，deltaTime 供动画使用。
void dx3d::WorldRenderer::render(const World& world, SwapChain& swapChain, f32 deltaTime)
{
	// 取后缓冲尺寸，用于设置视口（决定画面画到屏幕哪个区域）。
	auto size = swapChain.getSize();

	// 用上下文别名简化书写。
	auto& context = *m_deviceContext;
	// 1) 清后缓冲并用指定背景色（蓝灰）填充，同时把它设为当前渲染目标。
	// 第二个参数是 RGBA 清屏色。
	context.clearAndSetBackBuffer(swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });
	//context.clearAndSetBackBuffer(swapChain, { 0,0,0,1 });
	// 设视口大小为后缓冲尺寸，使渲染铺满整个画面。
	context.setViewportSize(size);

	// 绑定纹理采样器到着色器。std::span<Sampler*> 把数组首指针+长度打包传给 API，
	// 比传裸指针+int 更安全（封装了边界信息）。
	Sampler* samplers[] = { m_sampler.get() };
	context.setSamplers(std::span<Sampler*>{samplers});

	// 组件数量输出变量。getComponents 会把数量写入它，并返回组件数组首指针。
	auto numComponents = 0u;

	// 给四个常量缓冲起别名，写法简洁。
	auto& cameraCb = *m_cameraCb;
	auto& objectCb = *m_objectCb;
	auto& envCb = *m_envCb;
	auto& materialCb = *m_materialCb;


	// 环境数据（方向光等）默认清零。
	EnvironmentData envData{};
	//directional lights
	// 2) 方向光：从世界取该类型所有组件，把第一个的强度/方向/颜色写入环境缓冲。
	// 目前只取一个方向光（break 跳出循环），因此简单写入后即结束。
	{
		// getComponents 返回 T* const*（指向组件指针数组的指针）。
		// ECS 的“按类型分桶”优势：这里 O(1) 拿到该类型全部组件，无需遍历所有实体。
		auto components = world.getComponents<dx3d::DirectionaLightComponent>(numComponents);
		// std::views::iota(0u, numComponents) 生成 [0, numComponents) 的无符号整数序列，
		// 等价于 for(ui32 i=0;i<numComponents;i++)，但更现代、可组合。
		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			// 方向光本身也挂在某个实体上，取其 Transform 决定光照方向。
			auto& transform = component->getGameObject().getTransform();
			// 取刚体世界矩阵的第 2 行（行索引 2）作为光照方向。
			// 引擎约定：方向光实体的局部 Z 轴指向“光照射的方向”。
			auto dir = transform.getRigidWorldMatrix().row(2);

			// 把光参数写入环境数据结构。
			envData.directionalLightData.intensity = component->getIntensity();
			envData.directionalLightData.direction = { dir.x,dir.y,dir.z };
			envData.directionalLightData.color = component->getColor();
			// 只用第一个方向光，写入后立刻退出循环。
			break;
		}
		// std::as_bytes(std::span{&envData,1})：把 envData 这一个结构体的内存
		// 视作原始字节序列上传到常量缓冲。GPU 着色器的 cbuffer 按字节布局读取，
		// 这种“字节级”上传方式与 C++ 类型解耦，只要内存布局匹配即可。
		context.updateConstantBuffer(envCb, std::as_bytes(std::span{ &envData, 1 }));
	}

	//cameras
	// 3) 相机：取所有 CameraComponent，用第一个填 CameraData。
	{	
		CameraData cameraData{};
		auto components = world.getComponents<CameraComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{		
			auto component = components[i];
			// 视图矩阵：把世界坐标变换到相机坐标（相当于从相机视角看世界）。
			cameraData.view = component->getViewMatrix();
			// 把后缓冲尺寸告知相机，使其投影矩阵的宽高比正确（避免画面拉伸）。
			component->setViewportSize(size);
			// 投影矩阵：把相机坐标变换到裁剪空间（透视/正交投影）。
			cameraData.proj = component->getProjectionMatrix();
			// 相机世界位置，供光照计算（如视线方向、镜面反射）使用。
			cameraData.position = component->getGameObject().getTransform().getPosition();
			// 把相机数据按字节上传到 cameraCb，供顶点着色器使用。
			context.updateConstantBuffer(cameraCb, std::as_bytes(std::span{ &cameraData, 1 }));
			// 同样只用第一个相机。
			break;
		}
	}


	//cubes
	// 4) 立方体：逐个 Cube 组件绘制。每个立方体带自己的顶点/索引缓冲与材质。
	{	
		ObjectData objectData{};
		auto components = world.getComponents<CubeComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			// 立方体实体的 Transform 提供世界矩阵（把模型从局部坐标变到世界坐标）。
			auto& transform = component->getGameObject().getTransform();
			
			// 取该立方体的材质。材质包含着色器（管线状态）与贴图、参数等。
			auto material = component->getMaterial();
			
			if (material)
			{	
				// 把对象的仿射（含缩放）与刚体（不含缩放）世界矩阵写入对象常量缓冲。
				// 仿射用于顶点位置变换，刚体用于法线/方向变换。
				objectData.affineWorld = transform.getAffineWorldMatrix();
				objectData.rigidWorld = transform.getRigidWorldMatrix();

				// 设置图形管线状态（顶点着色器+像素着色器+混合/深度等），决定如何着色。
				context.setGraphicsPipelineState(material->getGraphicsPipelineState());
				// 上传对象世界矩阵到对象常量缓冲。
				context.updateConstantBuffer(objectCb, std::as_bytes(std::span{&objectData, 1 }));
				// 上传材质参数到材质常量缓冲（颜色、反光系数等）。
				context.updateConstantBuffer(materialCb, material->getData());
				// 把四个常量缓冲一次性绑定到着色器（对应 HLSL 的 b0/b1/b2/b3 寄存器）。
				ConstantBuffer* cbs[] = { &objectCb, &cameraCb, &envCb, &materialCb};
				context.setConstantBuffers(std::span<ConstantBuffer*>{cbs});

				// 收集本材质用到的所有纹理。先清空再按材质纹理数 resize。
				m_textures.clear();
				m_textures.resize(material->getNumTextures());
				// 逐个槽位取纹理，缺省的留空（nullptr）。
				for (auto t: std::views::iota(0u, m_textures.size()))
				{
					auto tex = material->getTexture(t);
					if (tex) m_textures[t] = &tex->getTexture();
				}
				// 把纹理数组绑定到着色器（像素着色器采样这些贴图）。
				context.setTextures(std::span<Texture*>{m_textures});

				// 绑定本立方体的顶点缓冲（顶点位置/法线/纹理坐标数据）。
				context.setVertexBuffer(component->getVertexBuffer());
				// 绑定索引缓冲（三角形由哪些顶点索引组成，定义拓扑）。
				context.setIndexBuffer(component->getIndexBuffer());
				// 下发绘制命令：按索引列表绘制三角形。参数：索引数量、起始顶点偏移、起始索引偏移。
				context.drawIndexedTriangleList(component->getIndexBuffer().getIndexListSize(), 0u, 0u);
			}
		}
	}


	//meshes
	// 5) 网格：与立方体类似，但一个网格可能含多段（多材质槽），需逐段绘制。
	{
		ObjectData objectData{};
		auto components = world.getComponents<MeshComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{
			auto comp = components[i];
			// 取网格资源（顶点/索引缓冲与材质槽信息）。没有则跳过该网格。
			auto meshRes = comp->getMesh();
			if (!meshRes) continue;
			auto& mesh = *meshRes;

			// 该网格实体世界矩阵（顶点/法线从局部到世界的变换）。
			objectData.affineWorld = comp->getGameObject().getTransform().getAffineWorldMatrix();
			objectData.rigidWorld = comp->getGameObject().getTransform().getRigidWorldMatrix();


			// 绑定该网格共享的顶点/索引缓冲（一个网格共用一套顶点数据）。
			context.setVertexBuffer(mesh.getVertexBuffer());
			context.setIndexBuffer(mesh.getIndexBuffer());

			// 取材质槽数组：每个槽对应网格中的一段，用各自的材质与纹理子集。
			auto numSlots = 0u;
			auto slots = mesh.getMaterialSlots(numSlots);
			
			// 逐材质槽绘制：不同段可用不同着色器/贴图，但共用同一套顶点缓冲。
			for (auto u : std::views::iota(0u, numSlots))
			{
				auto slot = slots[u];
				// 取本段对应的材质（由网格组件按材质索引提供）。
				auto material = comp->getMaterial(u);
				// 没有材质的段跳过（不绘制该子集）。
				if (!material) continue;
				auto numTexs = material->getNumTextures();

				// 设置本段的管线状态（着色器组合）。
				context.setGraphicsPipelineState(material->getGraphicsPipelineState());
				// 对象矩阵每个网格只算一次，但每个槽都要重新上传（GPU 在槽间会复用绑定）。
				context.updateConstantBuffer(objectCb, std::as_bytes(std::span{&objectData, 1 }));
				context.updateConstantBuffer(materialCb, material->getData());
				// 绑定四个常量缓冲。
				ConstantBuffer* cbs[] = { &objectCb, &cameraCb, &envCb, &materialCb};
				context.setConstantBuffers(std::span<ConstantBuffer*>{cbs});

				// 收集本段纹理并绑定。
				m_textures.clear();
				m_textures.resize(material->getNumTextures());
				for (auto t: std::views::iota(0u, m_textures.size()))
				{
					auto tex = material->getTexture(t);
					if (tex) m_textures[t] = &tex->getTexture();
				}
				context.setTextures(std::span<Texture*>{m_textures});

				// 绘制本段三角形：用 slot.indexCount 个索引、从 slot.startIndex 开始。
				// 起始顶点偏移为 0。
				context.drawIndexedTriangleList(slot.indexCount, 0, slot.startIndex);
			}
		}
	}

	// 6) 执行命令列表：把上述所有录制命令真正交给 GPU 执行（绘制真正发生在此）。
	m_graphicsDevice.executeCommandList(context);
	// 7) 呈现：把画好的后缓冲翻到屏幕（按垂直同步节奏翻页，避免撕裂）。
	swapChain.present();
}
