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
// 所属子系统：Core（核心基础设施）
// 文件职责：Base 类的实现（构造/析构/getLogger）。
// 实现极简：Base 只负责把 Logger 引用存起来并提供访问。
// 注意 m_logger 是引用成员，必须在成员初始化列表里绑定，不能在函数体内赋值。
// =============================================================================

#include <DX3D/Core/Base.h>


// 构造函数：用初始化列表 m_logger(desc.logger) 绑定日志器引用。
// 引用成员必须在初始化列表中绑定（C++ 规则），函数体内无法再赋值给引用。
dx3d::Base::Base(const BaseDesc& desc): m_logger(desc.logger)
{
}

// 虚析构函数体为空，无显式资源释放——Base 不持有需要手动释放的资源。
// 设为 virtual 的意义在头文件已说明：保证 delete 基类指针时正确调用派生类析构。
dx3d::Base::~Base()
{
}

// 返回日志器引用。noexcept 承诺不抛异常，供 DX3DLog* 宏内部调用 getLogger() 使用。
dx3d::Logger& dx3d::Base::getLogger() const noexcept
{
	return m_logger;
}
