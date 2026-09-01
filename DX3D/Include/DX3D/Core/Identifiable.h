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
// 所属子系统：Core（核心基础设施）—— 类型身份识别
// 文件职责：提供「运行期类型 ID」机制，让对象能在运行期被按类型区分。
//
// 关键概念讲解（初学者重点理解）：
//   引擎里的 ECS（实体-组件系统）需要在运行期按类型查找组件，
//   例如 GameObject 内部用一个「类型 ID -> 组件」的 map 来管理组件。
//   那么「类型 ID」从哪来？运行期程序已看不到类型名，于是用 typeid(T).hash_code()
//   给每个类生成一个唯一的 size_t 编号作为 ID。
//
//   本文件提供三样东西：
//     1) dx3d_typeid(类名) 宏：在类体内展开，自动生成静态 GetTypeId() 与
//        实例方法 getTypeId()，使该类具备「报上自己类型 ID」的能力。
//     2) HasTypeId concept：编译期检测某类型是否用过 dx3d_typeid 宏。
//     3) IsRegistered concept：检测某派生类是否既继承自基类、又注册了类型 ID。
//   这两个 concept 用于约束模板（requires 子句），保证只有「已注册」的类型
//   才能被创建/查询，否则编译期就报错，把误用挡在运行之前。
//
// 架构位置：Component/GameObject 都继承 Identifiable 并在类内使用 dx3d_typeid 宏。
// 协作对象：Base（基类）、GameObject（按类型 ID 存取组件）、World（按类型创建对象）。
// =============================================================================
#pragma once
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <type_traits> 


// dx3d_typeid(类名) 宏：
// 在某个类的 public 区使用（如 class Component : public Identifiable { dx3d_typeid(Component) ... }）。
// 展开后会给该类注入以下成员：
//   1) using type_id_tag = 类名;  —— 一个「类型标签」类型别名，供 concept 检测该类是否注册过
//   2) static GetTypeId()         —— 静态方法：返回该类的唯一 ID（用 typeid().hash_code()）
//   3) getTypeId() const noexcept override —— 实例方法：调用 GetTypeId()，满足 Identifiable 的纯虚函数
// typeid(Class).hash_code()：typeid 返回 type_info（运行期类型信息），
//   .hash_code() 返回一个 size_t 唯一编号——同一类型编号稳定且唯一，不同类型不同。
//   static const auto id 保证每个类只算一次 ID 并缓存。
// 效果：每个用过此宏的类都有一个唯一的类型编号，可被 map 以编号为键查找。
#define dx3d_typeid(Class) \
public:\
using type_id_tag = Class;\
static size_t GetTypeId()\
{\
	static const auto id = typeid(Class).hash_code();\
	return id;\
}\
size_t getTypeId() const noexcept override\
{\
	return GetTypeId();\
}


namespace dx3d
{
	// Identifiable：可识别对象的基类。
	// 继承 Base（持有日志器），并新增一个纯虚 getTypeId()：
	//   任何派生类必须能回答「你的类型 ID 是多少」。
	// 配合 dx3d_typeid 宏，派生类只需写一行宏就自动实现该方法。
	// 设计意图：让 ECS 能在运行期用一个 size_t 键来区分不同类型的组件/对象。
	class Identifiable : public Base
	{
	public:
		explicit Identifiable(const BaseDesc& desc) :
			Base(desc)
		{
		}
		// 纯虚函数：派生类必须实现（通常通过 dx3d_typeid 宏自动实现）。
		// noexcept 表示承诺不抛异常。返回当前对象所属类的运行期类型 ID。
		virtual size_t getTypeId() const noexcept = 0;
	};
	
	// HasTypeId concept（C++20 概念）：
	// 检测类型 T 是否含有成员类型 type_id_tag 且它等于 T 自身。
	//   - 用了 dx3d_typeid(T) 宏的类会定义 using type_id_tag = T;，于是匹配成功。
	//   - 没用过宏的类没有 type_id_tag 这个嵌套类型，模板替换失败（SFINAE），concept 为 false。
	// std::is_same_v<A,B> 判断 A、B 是否同一类型。typename T::type_id_tag 中的 typename
	//   告诉编译器「T::type_id_tag 是个类型名」（依赖类型，需 typename 前缀）。
	// 用途：在模板 requires 子句里做编译期约束，阻止未注册类型被误用。
	template <typename T>
	concept HasTypeId = std::is_same_v<typename T::type_id_tag, T>;

	// IsRegistered concept：
	// 检测 Derived 是否「已注册」为基类 Base 的派生类，需同时满足：
	//   1) std::is_base_of_v<Base, Derived>：Derived 确实继承自 Base（含间接继承）；
	//   2) HasTypeId<Derived>：Derived 用过 dx3d_typeid 宏，有类型 ID。
	// 两个条件都成立才认为 Derived 是「已注册」类型。
	// 典型用法（见 GameObject）：template<typename T> requires IsRegistered<Component, T>
	//   T* createOrGetComponent()——只有注册过的组件类型 T 才允许编译这函数，
	//   否则编译报错，把「用未注册类型创建组件」这类错误提前到编译期。
	template <typename Base, typename Derived>
	concept IsRegistered = std::is_base_of_v<Base, Derived> && HasTypeId<Derived>;
}
