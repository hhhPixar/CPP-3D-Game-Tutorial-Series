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

	{
		auto basicMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>(L"Game/Assets/Shaders/Basic.hlsl");
		if (basicMat)
		{
			auto matData = dx3d::Vec3(0.1f, 0.1f, 0.1f);
			basicMat->setData(std::as_bytes(std::span{ &matData, 1 }));
		}

		auto floor = world.createGameObject<dx3d::GameObject>();
		floor->createOrGetComponent<dx3d::CubeComponent>();
		auto comp = floor->createOrGetComponent<dx3d::CubeComponent>();
		comp->setMaterial(basicMat);
		floor->getTransform().setScale({ 6.8f, 0.1f, 6.8f });
		floor->getTransform().setPosition({ 0, -1, 0 });
		
	}

	srand((unsigned int)time(NULL));

	for (auto y = -2; y < 3; y++)
	{
		for (auto x = -2; x < 3; x++)
		{
			auto basicMat = getResourceManager().createResourceFromFile<dx3d::MaterialResource>(L"Game/Assets/Shaders/Basic.hlsl");
			if (basicMat)
			{
				auto r = (rand() % 255) / 255.0f;
				auto g = (rand() % 255) / 255.0f;
				auto b = (rand() % 255) / 255.0f;
				auto matData = dx3d::Vec3(r, g, b);
				basicMat->setData(std::as_bytes(std::span{ &matData, 1 }));
			}

			auto cube = world.createGameObject<dx3d::GameObject>();
			auto comp = cube->createOrGetComponent<dx3d::CubeComponent>();
			comp->setMaterial(basicMat);

			auto height = (rand() % 120) + (80.0f);
			height /= 100.0f;

			auto width = (rand() % 600) + (200.0f);
			width /= 1000.0f;

			cube->getTransform().setScale({ width, height, width });
			cube->getTransform().setPosition({ x * 1.4f, (height / 2.0f) - 1.0f, y * 1.4f });		
		}
	}

	auto player = world.createGameObject<Player>();
	player->getTransform().setPosition({ 0, 1, -2});

	getInputSystem().setCursorLocked(true);
	getInputSystem().setCursorVisible(false);
}


void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);
}
