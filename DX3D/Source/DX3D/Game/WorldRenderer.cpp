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

#include <DX3D/Resource/MaterialResource.h>

#include <DX3D/Math/Vec3.h>
#include <fstream>
#include <ranges>


dx3d::WorldRenderer::WorldRenderer(const WorldRendererDesc& desc): Base(desc.base), m_graphicsDevice(desc.engine)
{
	auto& device = m_graphicsDevice;
	m_deviceContext = device.createDeviceContext();

	const Vertex vertexList[] =
	{
		{{-0.5f,-0.5f,-0.5f}},
		{{-0.5f,0.5f,-0.5f} },
		{{0.5f,0.5f,-0.5f}},
		{{0.5f,-0.5f,-0.5f}},

		{{0.5f,-0.5f,0.5f}},
		{{0.5f,0.5f,0.5f}},
		{{-0.5f,0.5f,0.5f}},
		{{-0.5f,-0.5f,0.5f}}
	};

	const ui32 indexList[] =
	{
		0,1,2,  
		2,3,0,  

		4,5,6,
		6,7,4,

		1,6,5,
		5,2,1,

		7,0,3,
		3,4,7,

		3,2,5,
		5,4,3,

		7,6,1,
		1,0,7
	};

	m_objectCb = device.createConstantBuffer({ {}, sizeof(ObjectData) });
	m_cameraCb = device.createConstantBuffer({ {}, sizeof(CameraData) });
	m_materialCb = device.createConstantBuffer({ {}, dx3d::MaterialResource::MaxDataSize });

	m_vb = device.createVertexBuffer({vertexList, std::size(vertexList), sizeof(Vertex)});
	m_ib = device.createIndexBuffer({indexList, std::size(indexList)});
}

void dx3d::WorldRenderer::render(const World& world, SwapChain& swapChain, f32 deltaTime)
{
	auto size = swapChain.getSize();

	auto& context = *m_deviceContext;
	context.clearAndSetBackBuffer(swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });
	context.setViewportSize(size);

	auto numComponents = 0u;

	auto& cameraCb = *m_cameraCb;
	auto& objectCb = *m_objectCb;
	auto& materialCb = *m_materialCb;

	{	
		CameraData cameraData{};
		auto components = world.getComponents<CameraComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{	
		
			auto component = components[i];
			cameraData.view = component->getViewMatrix();
			component->setViewportSize(size);
			cameraData.proj = component->getProjectionMatrix();
			context.updateConstantBuffer(cameraCb, std::as_bytes(std::span{ &cameraData, 1 }));
			break;
		}
	}

	{	
		ObjectData objectData{};
		auto components = world.getComponents<CubeComponent>(numComponents);
		for (auto i : std::views::iota(0u, numComponents))
		{
			auto component = components[i];
			auto& transform = component->getGameObject().getTransform();
			auto material = component->getMaterial();

			if (material)
			{	
				objectData.world = transform.getAffineWorldMatrix();

				context.setGraphicsPipelineState(material->getGraphicsPipelineState());
				context.updateConstantBuffer(objectCb, std::as_bytes(std::span{&objectData, 1 }));
				context.updateConstantBuffer(materialCb, material->getData());
				ConstantBuffer* cbs[] = { &objectCb, &cameraCb, &materialCb};
				context.setConstantBuffers(std::span<ConstantBuffer*>{cbs});

				auto& vb = *m_vb;
				auto& ib = *m_ib;
				context.setVertexBuffer(vb);
				context.setIndexBuffer(ib);
				context.drawIndexedTriangleList(ib.getIndexListSize(), 0u, 0u);
			}
		}
	}

	m_graphicsDevice.executeCommandList(context);
	swapChain.present();
}
