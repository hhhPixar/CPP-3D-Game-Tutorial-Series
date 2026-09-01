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
// Component.cpp —— 组件基类的构造与访问器实现。
// 组件构造时一次性绑定好实体/世界/上下文三类引用，之后不再变动。
// =============================================================================

#include <DX3D/Game/Component.h>
#include <DX3D/Game/World.h>

// 构造：把 desc 里的实体/世界/上下文引用存为成员。
// Identifiable(desc.base) 把日志器等基类信息初始化好。
// 三个引用成员以初始化列表绑定，对象生命周期内始终指向同一组对象。
dx3d::Component::Component(const ComponentDesc& desc) : Identifiable(desc.base), m_object(desc.object), m_world(desc.world), m_context(desc.context)
{
}

// 返回所属实体引用。组件常借此访问同实体的其它组件（如 Mesh 取 Transform）。
dx3d::GameObject& dx3d::Component::getGameObject() noexcept
{
	return m_object;
}
