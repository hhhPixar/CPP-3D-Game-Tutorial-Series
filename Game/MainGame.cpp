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

#include "MainGame.h"
#include "Objects/Player.h"


MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

void MainGame::onCreate()
{
	Game::onCreate();
	auto& world = getWorld();

	//floor
	{
		auto floorTex = getResourceManager().createResourceFromFile<dx3d::TextureResource>(L"Game/Assets/Textures/stone_tiles_02_diff_1k.jpg");
		auto floorMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>(L"Game/Assets/Shaders/MaterialDataShader.hlsl");
		if (floorMat)
		{
			auto matData = dx3d::Vec3(1, 1, 1);
			floorMat->setData(std::as_bytes(std::span{ &matData, 1 }));
			floorMat->setTexture(0, floorTex);
		}

		auto floor = world.createGameObject<dx3d::GameObject>();
		floor->createOrGetComponent<dx3d::CubeComponent>();
		auto comp = floor->createOrGetComponent<dx3d::CubeComponent>();
		comp->setMaterial(floorMat);
		floor->getTransform().setScale({ 6.8f, 0.1f, 6.8f });
		floor->getTransform().setPosition({ 0, 0, 0 });	
	}
	
	//teapot
	{
		auto teapotMesh = getResourceManager().createResourceFromFile<dx3d::MeshResource>(L"Game/Assets/Meshes/teapot.obj");

		auto brickTex = getResourceManager().createResourceFromFile<dx3d::TextureResource>(L"Game/Assets/Textures/red_brick_03_diff_1k.jpg");
		auto brickMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>(L"Game/Assets/Shaders/BasicShader.hlsl");
		if (brickMat) brickMat->setTexture(0, brickTex);

		auto mesh = world.createGameObject<dx3d::GameObject>();
		auto comp = mesh->createOrGetComponent<dx3d::MeshComponent>();
		comp->setMesh(teapotMesh);
		comp->setMaterial(0, brickMat);
		mesh->getTransform().setPosition({ 0, 1, 0 });
		mesh->getTransform().setScale({ 2, 2, 2 });
	}

	//player
	{
		auto player = world.createGameObject<Player>();
		player->getTransform().setPosition({ 0, 1, -2 });

		getInputSystem().setCursorLocked(true);
		getInputSystem().setCursorVisible(false);
	}
}


void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);
}
