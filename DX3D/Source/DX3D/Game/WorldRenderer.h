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
// 所属子系统：Game / Rendering —— WorldRenderer 把 ECS 数据翻译成 GPU 绘制命令。
// 职责：每帧遍历世界里的光源/相机/网格/立方体组件，把它们的数据写入常量缓冲
//       （Constant Buffer），配置图形管线状态（着色器、纹理、顶点/索引缓冲），
//       下发 drawIndexedTriangleList 绘制命令，最后执行命令列表并 Present。
// 架构位置：Game::onInternalUpdate 末尾调用 render(world, swapChain, deltaTime)。
// 关键概念（渲染一帧的数据流）：
//   clearAndSetBackBuffer（清后缓冲+设渲染目标）
//   → 遍历 DirectionalLight 组件 → 填 EnvironmentData 常量缓冲
//   → 遍历 Camera 组件   → 填 CameraData 常量缓冲（视图/投影矩阵、位置）
//   → 遍历 Cube/Mesh 组件 → 填 ObjectData（世界矩阵）+ Material 数据 → 绘制
//   → executeCommandList（把录制好的命令交给 GPU 执行）
//   → present（翻页，把后缓冲显示到屏幕）
//   alignas(16)：常量缓冲结构按 16 字节对齐，因为 GPU/HLSL 的 cbuffer 要求
//   16 字节对齐的内存布局，否则着色器读取会错位。
// =============================================================================

#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec4.h>
#include <DX3D/Math/Vec2.h>
#include <DX3D/Math/Mat4x4.h>
#include <vector>

namespace dx3d
{
	// WorldRenderer —— 世界渲染器。final 表示不再被继承。
	// 它不持有任何游戏数据，只是每帧“读”世界并“写”GPU 命令。
	class WorldRenderer final: public Base
	{
	public:
		// 构造：创建设备上下文、各常量缓冲、采样器等渲染所需的固定资源。
		explicit WorldRenderer(const WorldRendererDesc& desc);
		// 每帧渲染入口：把 world 里的可见内容画到 swapChain 的后缓冲并呈现。
		void render(const World& world, SwapChain& swapChain, f32 deltaTime);
	private:
		// 对象级常量数据：每个可绘制物体（Cube/Mesh）的世界矩阵。
		// affineWorld = 旋转×平移×缩放（含缩放，用于带缩放的顶点变换）；
		// rigidWorld  = 旋转×平移（不含缩放，用于法线/光源方向等刚体变换）。
		// alignas(16) 保证结构按 16 字节对齐，匹配 GPU cbuffer 的布局要求。
		struct alignas(16) ObjectData
		{
			Mat4x4 affineWorld{};
			Mat4x4 rigidWorld{};
		};
		// 相机常量数据：视图矩阵、投影矩阵、相机位置。
		// view 把世界坐标变到相机坐标；proj 把相机坐标变到裁剪空间（投影）。
		struct alignas(16) CameraData
		{
			Mat4x4 view{};
			Mat4x4 proj{};
			Vec3 position{};
		};
		// 方向光数据：颜色、方向、强度。pad 是为对齐补的填充字段。
		struct alignas(16) DirectionalLightData
		{
			Vec3 color{}; f32 pad{};
			Vec3 direction{};
			f32 intensity{};
		};
		// 环境常量数据：当前只含一个方向光。未来可扩展环境光、点光等。
		struct alignas(16) EnvironmentData
		{
			DirectionalLightData directionalLightData;
		};

	private:
		// 引用图形设备（用于创建资源、执行命令列表）。
		GraphicsDevice& m_graphicsDevice;
		// 设备上下文：录制与下发绘制命令的接口（D3D11 的 ImmediateContext 封装）。
		RefPtr<DeviceContext> m_deviceContext{};
		// 四个常量缓冲：分别装相机/对象/环境/材质数据，每帧按需更新后绑给着色器。
		RefPtr<ConstantBuffer> m_cameraCb{};
		RefPtr<ConstantBuffer> m_objectCb{};
		RefPtr<ConstantBuffer> m_envCb{};
		RefPtr<ConstantBuffer> m_materialCb{};
		// 纹理采样器：决定如何从贴图取色（过滤、寻址模式等）。
		RefPtr<Sampler> m_sampler{};

		// 当前绘制对象所用的纹理指针临时数组。每物体重置，避免反复分配。
		std::vector<Texture*> m_textures{};
	};
}

