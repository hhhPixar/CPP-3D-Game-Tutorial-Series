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
#include "Objects/MyObject.h"


MainGame::MainGame(const dx3d::GameDesc& desc) : dx3d::Game(desc)
{
}

void MainGame::onCreate()
{
	Game::onCreate();
	auto& world = getWorld();

	for (auto x = 0; x < 3;x++)
	{
		for (auto y = 0; y < 3; y++)
		{
			auto object = world.createGameObject<dx3d::GameObject>();
			object->createOrGetComponent<dx3d::CubeComponent>();
			object->getTransform().setPosition({ (dx3d::f32) -1 + x,(dx3d::f32) -1 + y, 0 });
			m_objects[y*3 + x] = object;
		}
	}
}

void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);

	m_rot += deltaTime * 0.707f;
	m_scale = std::abs(std::sin(m_rot));

	for (auto i = 0; i < 9; i++)
	{
		m_objects[i]->getTransform().setRotation({ m_rot * i, m_rot, m_rot * i });
		m_objects[i]->getTransform().setScale({ m_scale,m_scale,m_scale });
	}
}
