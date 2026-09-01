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
// 所属子系统：Game / ECS —— GameObject 是 ECS 中的“实体（Entity）”。
// 职责：作为组件的容器与挂载点，每个实体持有一组按类型 id 索引的组件。
// 关键概念：
//   - 实体 = ID + 一组组件：GameObject 继承 Identifiable 拿到 typeId 与对象 id，
//     自身几乎不含游戏数据，数据都在 Component 里。这正是 ECS 的精髓。
//   - 按类型存组件：内部用 unordered_map<size_t, UniquePtr<Component>> 存，
//     key 是组件的 typeId，value 是该组件独占指针。同一实体每种类型最多一个组件。
//   - 模板方法 + requires 约束：createOrGetComponent<T>/getComponent<T> 是模板，
//     requires IsRegistered<Component,T> 在编译期检查 T 是否已注册 typeId，
//     未注册直接编译失败，避免运行时才知道类型不支持。
//   - 内置 Transform：每个实体构造时自动加一个 TransformComponent，
//     因为 3D 实体几乎都需要空间位置/朝向/缩放。
// =============================================================================

#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Identifiable.h>
#include <DX3D/Game/Component.h>

#include <unordered_map>

namespace dx3d
{
	// GameObject —— ECS 中的实体。继承 Identifiable 以获得 typeId 机制。
	// 用 dx3d_typeid 宏注册自身类型（生成静态 GetTypeId() 与实例 getTypeId()）。
	class GameObject : public Identifiable
	{
		dx3d_typeid(GameObject)
	public:
		// 构造：接收 GameObjectDesc（含日志器、GameContext、世界引用）。
		explicit GameObject(const GameObjectDesc& desc);

		// 创建或获取组件：若该实体已有类型 T 的组件就返回它，否则新建一个。
		// 用法：obj.createOrGetComponent<MeshComponent>()。
		// requires IsRegistered<Component,T> 是 C++20 requires 子句，编译期
		// 约束 T 必须是 Component 子类且通过 dx3d_typeid 注册了类型 id。
		// 这样模板内部能调 T::GetTypeId() 做按类型分桶，类型不合法直接编译报错。
		template <typename T>
		T* createOrGetComponent() requires IsRegistered<Component, T>
		{
			// 先尝试取已有的同类组件（一个实体每类最多一个），有就直接返回避免重复。
			auto c = getComponent<T>();
			if (c) return c;
			// 没有则构造一个新组件，传入日志器、自身实体引用、世界引用、游戏上下文。
			// 这些引用让组件内部也能反过来访问实体、世界与引擎服务。
			UniquePtr<Component> cp = std::make_unique<T>(ComponentDesc{
								{m_logger},
								*this,
								m_world,
								m_gameContext
				});
			// 走内部登记逻辑（入实体自己的 map 并向世界登记），再强转为 T*。
			// static_cast 安全：cp 的静态类型虽是 Component*，但实际指向 T。
			return static_cast<T*>(createComponentInternal(cp));
		}

		// 获取类型 T 的组件，没有返回 nullptr。同样带 requires 约束。
		// 内部用 T::GetTypeId() 拿到编译期类型 id，再去 map 里查。
		template <typename T>
		T* getComponent() requires IsRegistered<Component, T>
		{
			return static_cast<T*>(getComponentInternal(T::GetTypeId()));
		}

		// 取实体的 Transform（必存在，构造时自动添加）。空间变换的入口。
		TransformComponent& getTransform() noexcept;
		// 便捷访问器：把 GameContext 里的引擎服务转发出去，供组件/脚本使用。
		World& getWorld() noexcept;
		InputSystem& getInputSystem() noexcept;
		ResourceManager& getResourceManager() noexcept;
	protected:
		// 生命周期钩子：实体被正式加入世界后调用一次。子类实体在此初始化数据、
		// 加载资源、再添加更多组件等。
		virtual void onCreate() {}
		// 生命周期钩子：每帧调用。子类在此推进自身逻辑（移动、状态机等）。
		virtual void onUpdate(f32 deltaTime) {}	
	private:
		// 内部：把组件独占指针纳入实体 map 并向世界登记，返回裸指针。
		Component* createComponentInternal(UniquePtr<Component>& component);
		// 内部：按类型 id 查实体 map 里的组件，没有返回空。
		Component* getComponentInternal(size_t id);
	private:
		// 按类型 id 存组件：key 是组件 typeId，value 是组件独占指针。
		// 这是“实体持有自己组件”的所有权所在；World 里的 m_components 只是视图。
		std::unordered_map<size_t, UniquePtr<Component>> m_components{};
		// 缓存自身 Transform 指针：几乎每帧都要用，避免每次都 map 查找。
		TransformComponent* m_transform{};
		// 引擎服务上下文（输入/资源/设备），转发给组件使用。
		GameContext m_gameContext;
		// 所属世界引用：组件登记、脏 Transform 登记都要走它。
		World& m_world;	
		// 友元：World 需要访问私有 createGameObjectInternal 等以延迟创建实体。
		friend class World;
	};
}

