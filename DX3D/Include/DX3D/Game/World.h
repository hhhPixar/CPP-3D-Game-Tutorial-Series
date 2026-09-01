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
// 所属子系统：Game / ECS —— World 是“实体-组件系统”的总容器与调度器。
// 职责：管理所有 GameObject（实体）与 Component（组件）的创建、存储与每帧更新。
// 关键概念（ECS 的核心思想）：
//   1) 按类型反射存储（Type-Indexed Storage）：
//      实体和组件不是平铺一个大数组，而是按“类型 id（typeId）”分桶存放在
//      unordered_map<size_t, vector<...>> 里。这样按类型查询极快（O(1) 定位桶），
//      渲染器遍历“所有相机”“所有光源”时直接取对应桶即可，无需遍历全部实体。
//   2) 延迟创建（Deferred Creation）：
//      在一帧的 update 过程中，某个实体的 onUpdate 可能想创建新实体。如果立刻
//      往容器里插，会导致正在遍历的容器发生结构性修改 → 迭代器失效（崩溃）。
//      因此新实体先放进 pending 缓冲，把“创建事件”排队，等本轮遍历结束后再统一提交。
//   3) 脏标记批量更新（Dirty Flag）：
//      Transform 的位置/旋转/缩放改了，世界矩阵就要重算。但一帧里可能改多次。
//      做法：改 Transform 时把它登记进 m_dirtyTransforms（只登记一次），到 update
//      末尾统一重算。避免重复计算，也避免改一次就立即重算的浪费。
// =============================================================================

#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Identifiable.h>
#include <unordered_map>
#include <vector>


namespace dx3d
{
	// World —— 实体与组件的中央容器。final 表示不可再被继承。
	// 它本身继承 Base（持有日志器），并通过 GameContext 持有输入/资源/设备引用，
	// 以便在创建实体时把这些引擎服务转发给实体内部使用。
	class World final : public Base
	{
	public:
		// 构造：接收 WorldDesc（含日志器与 GameContext）。
		explicit World(const WorldDesc& desc);

		// 创建一个类型为 T 的 GameObject（实体）并加入世界。
		// 用法：world.createGameObject<MyEntity>()，返回强类型指针 MyEntity*。
		// 模板约束 requires IsRegistered<GameObject,T>：要求 T 必须是 GameObject 的
		// 子类且通过 dx3d_typeid 宏注册过类型 id，否则编译报错——这是 ECS 按类型
		// 分桶存储的前提（必须有可用的 typeId）。
		template <typename T>
		T* createGameObject() requires IsRegistered<GameObject, T>
		{
			// 先造一个 GameObject 独占指针，塞入日志器、GameContext 与世界引用。
			UniquePtr<GameObject> e = std::make_unique<T>(GameObjectDesc{ 
				{m_logger},
				m_gameContext,
				*this 
				});
			// 走内部延迟创建逻辑：不立刻入桶，而是排队等本轮 update 结束再提交。
			// static_cast 把基类指针转回具体子类 T*，因为 T 确定是 GameObject 子类。
			return static_cast<T*>(createGameObjectInternal(e));
		}

		// 按类型取出该类型所有组件的连续数组，输出数量到 numComponents。
		// 用法：ui32 n; auto comps = world.getComponents<CameraComponent>(n);
		// 返回 T* const* —— 指向“T* 数组”的指针；底层其实是 Component** 做 reinterpret。
		// 这里用 reinterpret_cast 把 Component** 当成 T* const* 读，安全前提是
		// 所有存进桶的组件类型确实都是 T（因为按 typeId 分桶，桶里全是同一类型）。
		// requires IsRegistered<Component,T> 同样要求 T 已注册 typeId。
		template <typename T> requires IsRegistered<Component, T>
		T* const* getComponents(ui32& numComponents) const noexcept
		{
			return reinterpret_cast<T* const*>(getComponentsInternal(T::GetTypeId(), &numComponents));
		}

		// 每帧调用：处理延迟创建事件、调用所有实体的 onUpdate、批量重算脏 Transform。
		void update(f32 deltaTime);
	private:
		// 延迟创建的内部实现：把实体放进 pending 缓冲并登记一个 Create 事件，返回裸指针。
		GameObject* createGameObjectInternal(UniquePtr<GameObject>& object);
		// 把组件按其 typeId 登记进对应桶（供渲染器按类型查询）。
		void addComponentInternal(Component& component);
		// 把发生变化的 Transform 登记进脏列表，留到 update 末尾统一重算世界矩阵。
		void addDirtyTransformInternal(TransformComponent& component);

		// 按 typeId 取出对应组件桶的起始指针与数量（不抛异常）。
		Component* const* getComponentsInternal(size_t typeId, ui32* numComponents) const noexcept;
	private:
		// 事件类型枚举。目前只有 Create；以后可扩展 Destroy/Move 等。
		enum class EventType
		{
			Create = 0
		};
		// 一条延迟事件：记录是哪个对象、它在 pending 缓冲里的下标、事件种类。
		struct GameObjectEvent
		{
			GameObject* object{};
			size_t pendingObjectIndex{};
			EventType eventType{};
		};

	private:	
		// 把引擎服务（输入/资源/设备）保存下来，创建实体时转发给实体内部。
		GameContext m_gameContext;

		// 实体按 typeId 分桶：unordered_map<typeId, vector<实体*>>。
		// 这样按类型查询 O(1) 定位桶，遍历同类实体连续高效。
		std::unordered_map<size_t, std::vector<UniquePtr<GameObject>>> m_objects{};
		// 组件按 typeId 分桶：unordered_map<typeId, vector<组件*>>。
		// 注意这里存的是裸指针 Component*，不持有所有权（所有权在 GameObject 里），
		// World 只是维护一个“按类型索引的视图”供渲染器快速查询。
		std::unordered_map<size_t, std::vector<Component*>> m_components{};

		// 脏 Transform 列表：本帧被改动过的 Transform 会被登记在此，
		// update 末尾统一重算世界矩阵后清空。避免每改一次就重算一次的浪费。
		std::vector<TransformComponent*> m_dirtyTransforms{};

		// 延迟创建缓冲：刚创建但尚未正式入桶的实体先暂存于此。
		// 主缓冲与交换缓冲配合 update 里的 std::swap 使用：把当前缓冲与交换缓冲
		// 互换后，遍历交换缓冲处理事件，期间新产生的延迟创建又写回主缓冲，
		// 实现“遍历过程中安全地再产生新对象”而不互相干扰。
		std::vector<UniquePtr<GameObject>> m_pendingObjects;
		std::vector<UniquePtr<GameObject>> m_pendingObjectsSwapBuffer;

		// 事件队列：与 pendingObjects 配合。同样有主缓冲与交换缓冲的双缓冲设计。
		std::vector<GameObjectEvent> m_events{};
		std::vector<GameObjectEvent> m_eventsSwapBuffer{};


		// 友元：GameObject 与 TransformComponent 需要访问私有方法
		// （addComponentInternal / addDirtyTransformInternal），故声明为友元。
		friend class GameObject;
		friend class TransformComponent;
	};
}

