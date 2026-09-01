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
// GameObject.cpp —— 实体的构造与组件管理实现。
// 重点：构造时自动为每个实体附加一个 TransformComponent，这是 3D 实体的标配。
// =============================================================================

#include<DX3D/Game/GameObject.h>
#include<DX3D/Game/Component.h>
#include<DX3D/Component/TransformComponent.h>
#include<DX3D/Game/World.h>

// 构造：把日志器交给 Identifiable/Base，保存世界引用与 GameContext。
dx3d::GameObject::GameObject(const GameObjectDesc& desc) : Identifiable(desc.base), m_world(desc.world), m_gameContext(desc.gameContext)
{
	// 每个实体必带一个 TransformComponent（位置/旋转/缩放）。
	// createOrGetComponent 会新建并登记到本实体的 map 与世界的组件桶里。
	// 缓存其指针到 m_transform，后续 getTransform 直接返回，免 map 查找。
	m_transform = createOrGetComponent<TransformComponent>();
}

// 返回内置 Transform 引用。必有，故返回引用而非指针。
dx3d::TransformComponent& dx3d::GameObject::getTransform() noexcept
{
	return *m_transform;
}

// 返回所属世界引用。
dx3d::World& dx3d::GameObject::getWorld() noexcept
{
	return m_world;
}

// 从 GameContext 取输入系统转发出去。
dx3d::InputSystem& dx3d::GameObject::getInputSystem() noexcept
{
	return m_gameContext.input;
}

// 从 GameContext 取资源管理器转发出去。
dx3d::ResourceManager& dx3d::GameObject::getResourceManager() noexcept
{
	return m_gameContext.resourceManager;
}

// 内部：把组件纳入本实体的 map（按 typeId），并通知世界把它登记进组件桶。
// 一个实体每种类型最多一个组件：若该 typeId 已存在则拒绝（返回空）。
dx3d::Component* dx3d::GameObject::createComponentInternal(UniquePtr<Component>& component)
{
	// 空指针保护。
	if (!component) return {};
	// 取组件类型 id，作为 map 的 key。
	auto typeId = component->getTypeId();
	// 保存裸指针：稍后 std::move 会清空 component 引用，但指针值仍有效。
	auto ptr = component.get();
	// 同类型已存在则不重复添加（一实体一类一组件）。
	if (m_components.find(typeId) != m_components.end()) return {};
	// 把独占指针转入实体 map，所有权归属实体。
	m_components.emplace(typeId, std::move(component));
	// 通知世界把这个组件登记进对应类型桶，供渲染器按类型查询。
	m_world.addComponentInternal(*ptr);
	// 返回裸指针供调用方使用。
	return ptr;
}

// 内部：按类型 id 查实体 map 里的组件，没有返回空。
dx3d::Component* dx3d::GameObject::getComponentInternal(size_t id)
{
	auto it = m_components.find(id);
	if (it == m_components.end()) return {};
	// unique_ptr 的 .get() 取裸指针，不转移所有权。
	return it->second.get();
}
