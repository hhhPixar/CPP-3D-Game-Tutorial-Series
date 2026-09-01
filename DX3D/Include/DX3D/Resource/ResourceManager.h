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

// ============================================================================
// 资源管理器（ResourceManager）—— 资源系统的"工厂 + 缓存"
// 职责：
//   1. 工厂：按文件扩展名创建对应类型的资源
//      （.obj→MeshResource / .jpg|.png→TextureResource / .hlsl|.fx→MaterialResource）。
//   2. 缓存去重：以文件路径为键缓存已创建资源。同一文件不会被重复解析，
//      多次请求同一资源直接返回同一份缓存对象（网格、纹理是只读共享数据）。
//   3. 材质特例：材质每次取用都会"克隆"一份，让每个使用者拥有独立的参数与纹理，
//      而编译好的着色器/管线状态在多份克隆间共享（详见 .cpp）。
// 关键概念：
//   · createResourceFromFile<T> 是模板方法：调用方指定期望的资源类型 T，
//      内部用 std::dynamic_pointer_cast 把基类 RefPtr<Resource> 安全向下转型为 RefPtr<T>。
//   · m_resources 是哈希表 unordered_map，键为路径 wstring，值为 RefPtr<Resource>，
//      按路径哈希查找，平均 O(1)，实现"同路径只加载一次"的去重。
// ============================================================================
#pragma once
#include <unordered_map>
#include <string>
#include <DX3D/Core/Common.h>
#include <DX3D/Resource/Resource.h>


namespace dx3d
{
	// 资源管理器：final 表示不可被继承（避免子类破坏缓存/创建逻辑）。
	class ResourceManager final: public Base
	{
	public:
		// 构造函数：用 ResourceManagerDesc 初始化（含日志器与系统上下文）。
		// explicit 防止隐式转换。
		explicit ResourceManager(const ResourceManagerDesc& desc);
		// 模板方法：按文件路径创建/获取资源并转型为指定类型 T（如 MeshResource）。
		// 真正的创建+缓存逻辑放在私有的 createResourceFromFileConcrete 中，
		// 这里只负责把返回的 RefPtr<Resource> 用 dynamic_pointer_cast 安全转为 RefPtr<T>。
		template<typename T>
		RefPtr<T> createResourceFromFile(const wchar_t* file_path)
		{
			// dynamic_pointer_cast：运行时检查所指对象是否真的是 T 类型；
			// 是则返回指向同一对象的 shared_ptr<T>（不拷贝对象，只增加引用计数），
			// 否则返回空的 shared_ptr<T>。相当于 shared_ptr 版的 dynamic_cast。
			return std::dynamic_pointer_cast<T>(createResourceFromFileConcrete(file_path));
		}

	private:
		// 实际的创建/缓存逻辑（非模板，返回基类指针）。具体见 .cpp 实现。
		RefPtr<Resource> createResourceFromFileConcrete(const wchar_t* file_path);
		// 根据文件路径构造一个 ResourceDesc（把日志器、路径、本管理器打包给具体资源用）。
		ResourceDesc getResourceDesc(const wchar_t* file_path);
	private:
		// 资源缓存表：路径 → 资源对象。unordered_map 按路径哈希查找，平均 O(1)。
		std::unordered_map<std::wstring, RefPtr<Resource>> m_resources{};
		// 系统上下文，保存对 GraphicsDevice 的引用；创建资源时需要图形设备来建 GPU 缓冲/纹理/管线。
		SystemContext m_context;
	};
}
