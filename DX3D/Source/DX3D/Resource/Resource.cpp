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

// 资源基类的实现文件。构造时把文件路径和对 ResourceManager 的引用保存下来，
// 同时初始化基类 Base 部分（绑定日志器），供所有派生资源类（网格/纹理/材质）共用。
#include <DX3D/Resource/Resource.h>

// Resource 构造函数：
//  - Base(desc.base)：先初始化基类 Base（绑定日志器 m_logger）。
//  - m_path(desc.path)：把传入的 wchar_t* 路径保存为 wstring。
//  - m_manager(desc.manager)：绑定资源管理器引用（引用必须在此初始化）。
dx3d::Resource::Resource(const ResourceDesc& desc) : Base(desc.base), m_path(desc.path), m_manager(desc.manager)
{
}