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
// World.cpp —— 世界的每帧调度实现，集中体现两个核心机制：
//   1) 延迟创建（Deferred Creation）：用双缓冲（swap buffer）在 update 中安全地
//      把上一轮排队的事件落地，期间新产生的事件写回另一个缓冲，互不干扰。
//      这样即使在遍历实体的过程中某个实体又创建了新实体，也不会破坏当前遍历。
//   2) 脏标记批量更新（Dirty Flag Batch Update）：update 末尾统一重算所有被
//      标脏的 Transform 世界矩阵，避免一帧内重复计算。
// =============================================================================

#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/Component.h>
#include <DX3D/Component/TransformComponent.h>

// 构造：把 desc 里的日志器交给基类 Base，把 GameContext（输入/资源/设备）留作创建实体时转发。
dx3d::World::World(const WorldDesc& desc) : Base(desc.base), m_gameContext(desc.gameContext)
{
}

// 每帧更新：三步走 —— 处理延迟事件 → 遍历实体 onUpdate → 重算脏 Transform。
void dx3d::World::update(f32 deltaTime)
{
	// 第一步：如果上一轮累积了延迟创建事件，现在统一落地。
	// 用双缓冲的关键：先把主缓冲与交换缓冲指针互换，于是这一轮处理的是“旧”缓冲，
	// 而本轮新产生的事件/对象写入“新”主缓冲，不会破坏正在遍历的旧缓冲。
	if (m_events.size())
	{
		std::swap(m_events, m_eventsSwapBuffer);
		std::swap(m_pendingObjects, m_pendingObjectsSwapBuffer);
	
		// 遍历“旧”事件缓冲（交换后已指向 m_eventsSwapBuffer）。
		for (auto& e : m_eventsSwapBuffer)
		{
			// 取该实体所属的类型 id（用于决定放进 m_objects 的哪个桶）。
			auto objTypeId = e.object->getTypeId();
			// 该实体在 pending 缓冲里的下标（用于取出独占指针）。
			auto pendingObjIndex = e.pendingObjectIndex;

			// 目前只有 Create 一种事件：把实体从 pending 缓冲正式搬进对应类型桶。
			if (e.eventType == EventType::Create)
			{
				// 从 pending 交换缓冲取出独占指针（std::move 转移所有权）。
				auto& obj = m_pendingObjectsSwapBuffer[pendingObjIndex];
				auto ptr = obj.get();
				// push_back(std::move(obj)) 把实体所有权交给 m_objects 桶。
				m_objects[objTypeId].push_back(std::move(obj));
				// 实体正式入桶后，才调它的 onCreate 钩子（此时它已在世界中、可被查询）。
				ptr->onCreate();		
			}
		}

		// 处理完后清空交换缓冲（主缓冲可能因本轮新增而又非空，留到下一帧处理）。
		m_pendingObjectsSwapBuffer.clear();
		m_eventsSwapBuffer.clear();
	}

	// 第二步：遍历每个类型桶，桶内遍历每个实体，调用其 onUpdate。
	// 结构化绑定 [typeId, objects] 同时取 map 的 key 和 value。
	for (auto&& [typeId, objects] : m_objects)
	{
		for (auto& object : objects)
		{
			object->onUpdate(deltaTime);
		}
	}	

	// 第三步：批量重算所有“脏”的 Transform 世界矩阵。
	// 改 Transform 的位置/旋转/缩放时只是标脏不立即重算（见 TransformComponent.cpp），
	// 在这里统一重算，保证渲染时矩阵都是最新的，同时避免一帧内改多次重算多次。
	for (auto& comp : m_dirtyTransforms)
	{
		comp->updateWorldMatrix();
	}
	// 重算完毕，清空脏列表，下一帧重新登记。
	m_dirtyTransforms.clear();
}

// 延迟创建的内部实现：实体不立刻入桶，先暂存于 pending 缓冲并排队一个 Create 事件。
// 返回实体的裸指针，调用方（createGameObject 模板）可立即拿到引用，但实体要等
// 下一次 update 才真正进入 m_objects 桶并被遍历到。
dx3d::GameObject* dx3d::World::createGameObjectInternal(UniquePtr<GameObject>& object)
{
	// 空指针保护，直接返回空。
	if (!object) return {};

	// 先保存裸指针：对象所有权稍后要 std::move 进 pending，move 之后原对象为空，
	// 但裸指针值仍然有效，可用来返回给调用方以及记录在事件里。
	auto ptr = object.get();

	// 记录该对象在 pending 缓冲中的下标，以便后续事件处理时按下标取回。
	auto index = m_pendingObjects.size();
	// 把所有权转入 pending 缓冲（object 这个引用参数被清空）。
	m_pendingObjects.push_back(std::move(object));
	// 排队一条 Create 事件，等 update 时落地。
	m_events.push_back({ ptr, index, EventType::Create });

	// 返回裸指针：实体已存在，但暂时只在 pending 里，尚未进入类型桶。
	return ptr;
}

// 把组件按其 typeId 登记进对应桶。组件所有权在 GameObject，这里只登记裸指针做视图。
// 渲染器 getComponents 就是从这些桶里按类型取数组。
void dx3d::World::addComponentInternal(Component& component)
{
	// 取组件的类型 id，决定放进 m_components 的哪个桶。
	auto typeId = component.getTypeId();
	// 把裸指针追加到对应类型桶尾部。
	m_components[typeId].push_back(&component);
}

// 把发生变化的 Transform 登记进脏列表。Transform 改位置/旋转/缩放时会调到这里。
// 注意：Transform 内部用 m_dirty 标志保证同一帧内只登记一次（见 markAsDirty）。
void dx3d::World::addDirtyTransformInternal(TransformComponent& component)
{
	m_dirtyTransforms.push_back(&component);
}

// 按 typeId 取组件桶：返回桶内裸指针数组的首地址与数量。
// 渲染器通过 getComponents<T> 间接调到这里，按类型一次性拿到该类型全部组件。
dx3d::Component* const* dx3d::World::getComponentsInternal(size_t typeId, ui32* numComponents) const noexcept
{
	// 在 map 里找对应 typeId 的桶。
	auto it = m_components.find(typeId);
	if (it != m_components.end())
	{
		// 找到了：通过输出参数返回数量，返回数组首地址（vector 内部连续存储）。
		*numComponents = static_cast<ui32>(it->second.size());
		return it->second.data();
	}

	// 没有该类型组件：返回 0 个与空指针。调用方据此跳过该类型渲染。
	*numComponents = 0u;
	return {};
}