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
// 所属子系统：Game / ECS —— Component 是所有“组件”的基类。
// 职责：定义组件共有的引用关系（所属实体、世界、游戏上下文）与类型 id 机制。
// 关键概念：
//   - 组件 = 纯数据 + 少量访问器：具体组件（TransformComponent/CubeComponent/
//     CameraComponent/...）继承 Component，承载实体的某一维数据/行为。
//   - 类型 id（typeId）：通过 dx3d_typeid 宏注册，使组件可被按类型分桶存储与查询。
//     ECS 的核心就是“按类型组织数据”，typeId 是这一切的钥匙。
//   - 反向引用：组件持有其所属 GameObject 与 World 的引用，可反过来访问实体、
//     世界与引擎服务，这是组件能彼此协作（如 Mesh 组件取实体 Transform）的基础。
// =============================================================================

#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Identifiable.h>

namespace dx3d
{
	// Component —— 组件基类。继承 Identifiable 以获得 typeId 机制。
	// 具体组件派生自此并通过 dx3d_typeid 注册自己类型，即可被 ECS 按类型管理。
	class Component : public Identifiable
	{
		// 注册类型 id：生成静态 GetTypeId()（基于 typeid 的哈希，每个类型唯一）
		// 与实例 getTypeId()（调用静态版）。ECS 按这个值分桶存储与查询。
		dx3d_typeid(Component)
	public:
		// 构造：从 ComponentDesc 接收实体/世界/上下文引用并保存。
		// 这些引用让组件内部能访问实体、世界与引擎服务。
		explicit Component(const ComponentDesc& desc);
		// 返回所属实体引用：组件常需访问实体上的其它组件（如取 Transform）。
		GameObject& getGameObject() noexcept;
	
	protected:
		// 所属实体引用：组件挂在哪个实体上。引用而非指针，语义上组件不能脱离实体存在。
		GameObject& m_object;
		// 世界引用：可用于登记脏 Transform、查询其它实体等。
		World& m_world;
		// 游戏上下文引用：提供输入/资源/设备三类引擎服务。
		GameContext& m_context;
	};
}

