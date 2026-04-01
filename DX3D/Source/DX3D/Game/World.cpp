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

#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>

dx3d::World::World(const WorldDesc& desc) : Base(desc.base)
{
}

void dx3d::World::update(f32 deltaTime)
{
	if (m_events.size())
	{
		std::swap(m_events, m_eventsSwapBuffer);
		std::swap(m_pendingObjects, m_pendingObjectsSwapBuffer);

		for (auto& e : m_eventsSwapBuffer)
		{
			auto objTypeId = e.object->getTypeId();
			auto pendingObjIndex = e.object->getWorldIndex();

			if (e.eventType == EventType::Create)
			{
				auto& obj = m_pendingObjectsSwapBuffer[pendingObjIndex];
				auto ptr = obj.get();

				auto index = m_objects[objTypeId].size();
				ptr->setWorldIndex(index);

				m_objects[objTypeId].push_back(std::move(obj));

				ptr->onCreate();
			}
		}

		m_pendingObjectsSwapBuffer.clear();
		m_eventsSwapBuffer.clear();
	}

	for (auto&& [typeId, objects] : m_objects)
	{
		for (auto& object : objects)
		{
			object->onUpdate(deltaTime);
		}
	}
}

dx3d::GameObject* dx3d::World::createGameObjectInternal(UniquePtr<GameObject>& object)
{
	if (!object) return {};

	auto ptr = object.get();

	auto index = m_pendingObjects.size();
	ptr->setWorldIndex(index);

	m_pendingObjects.push_back(std::move(object));
	m_events.push_back({ ptr, EventType::Create });

	return ptr;
}
