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

	auto object = world.createGameObject<dx3d::GameObject>();
	object->createOrGetComponent<dx3d::CubeComponent>();
	m_player = object;

	getInputSystem().setCursorLocked(true);
	getInputSystem().setCursorVisible(false);
}

void MainGame::onUpdate(dx3d::f32 deltaTime)
{
	Game::onUpdate(deltaTime);


	auto rot = m_player->getTransform().getRotation();
	rot.x += getInputSystem().getMouseDelta().y * 0.01f;
	rot.y -= getInputSystem().getMouseDelta().x * 0.01f;
	m_player->getTransform().setRotation(rot);


	auto pos = m_player->getTransform().getPosition();	
	auto forward = 0.0f;
	auto rightward = 0.0f;	
	auto speed = 3.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::W)) forward = 1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::S)) forward = -1.0f; 	
	if (getInputSystem().isKeyDown(dx3d::KeyCode::D)) rightward = 1.0f;
	if (getInputSystem().isKeyDown(dx3d::KeyCode::A)) rightward = -1.0f;
	auto direction = dx3d::Vec3::normalize({ rightward,forward,0 });
	pos = pos + direction * speed * deltaTime;
	m_player->getTransform().setPosition(pos);

}
