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
// 文件职责：整个引擎的「中央定义」文件，包含三大内容：
//   1) 前向声明（forward declaration）：列出所有主要类名，供别的头文件用
//      指针/引用时无需 include 完整定义，从而减少编译耦合与循环依赖。
//   2) 类型别名：i32/ui32/f32/d64，保证不同平台下基本类型的宽度一致。
//   3) 智能指针别名：RefPtr=shared_ptr（共享所有权）、UniquePtr=unique_ptr（独占所有权）。
//   4) 宏 dx3d_disable_copy_and_move：一键禁用类的拷贝与移动构造。
// 架构位置：最底层，几乎所有头文件都会直接或间接 include 本文件。
// 初学者提示：引擎里用「前向声明 + 指针/引用」来打破头文件之间的循环依赖，
// 真正使用对象时再 include 对应的完整定义头。这是大型 C++ 项目的常见做法。
// =============================================================================
//
#pragma once
#include <stdexcept>
#include <memory>

// dx3d_disable_copy_and_move 宏：
// 在类体内使用，一次性把「拷贝构造、拷贝赋值、移动构造、移动赋值」四个特殊成员函数全部 = delete。
// 展开示例（假设传入类名 Base）：
//   protected:
//       Base(const Base&) = delete;
//       Base& operator=(const Base&) = delete;
//       Base(Base&&) = delete;
//       Base& operator=(Base&&) = delete;
// 为什么这么做？引擎对象常持有 D3D11 资源句柄、Win32 句柄等不可随意复制的东西，
// 若被意外拷贝会导致两个对象指向同一资源、析构时双重释放（double free）。
// 用 protected 访问权限使派生类也遵循同一约束，从源头杜绝误用。
#define dx3d_disable_copy_and_move(Class)     \
protected:\
    Class(const Class&) = delete;        \
    Class& operator=(const Class&) = delete; \
    Class(Class&&) = delete;             \
    Class& operator=(Class&&) = delete;

namespace dx3d
{
	// ---- 前向声明区（forward declarations）----------------------------------
	// 下面只给出类名，不给出定义。这样别的头文件可以用「类名& / 类名*」
	// 引用这些对象而无需 include 它们的完整头文件，从而：
	//   - 加快编译速度（改动一个类不会触发所有文件重编译）
	//   - 避免头文件之间相互包含造成的循环依赖（A include B，B include A）
	// 以下为图形/渲染相关类：设备、交换链、缓冲区、着色器、纹理等。
	class Base;
	class Window;
	class Game;
	class InputSystem;
	class GraphicsEngine;
	class GraphicsDevice;
	class Logger;
	class SwapChain;
	class Display;
	class DeviceContext;
	class ShaderBinary;
	class GraphicsPipelineState;
	class VertexBuffer;
	class VertexShaderSignature;
	class ConstantBuffer;
	class IndexBuffer;
	class Texture;
	class Sampler;
	class GraphicsPipelineLayout;

	// 以下为游戏逻辑层类：世界、游戏对象、组件。
	class World;
	class GameObject;
	class Component;
	class TransformComponent;

	// 世界渲染器：负责把 World 的内容渲染到屏幕。
	class WorldRenderer;

	// 以下为资源系统类：资源管理器与各类资源。
	class ResourceManager;
	class Resource;
	class MaterialResource;
	class TextureResource;
	class MeshResource;




	// ---- 类型别名区 ----------------------------------------------------------
	// 用固定含义的别名替代原生类型，让代码意图更清晰，也便于跨平台：
	//   i32  = 32 位有符号整数（范围约 ±21 亿）
	//   ui32 = 32 位无符号整数（范围 0 ~ 约 42 亿），常用于大小、索引
	//   f32  = 32 位单精度浮点，GPU 着色器默认的浮点精度
	//   d64  = 64 位双精度浮点，用于需要高精度的计算
	using i32 = int;
	using ui32 = unsigned int;
	using f32 = float;
	using d64 = double;

	// 智能指针别名（现代 C++ 内存管理核心）：
	//   RefPtr<T> = std::shared_ptr<T>：共享所有权，多个指针可指向同一对象，
	//      内部引用计数：每多一个引用计数 +1，销毁时 -1，归零时才真正释放对象。
	//      适合「多处共同使用、生命周期不唯一」的资源，如纹理、网格。
	//   UniquePtr<T> = std::unique_ptr<T>：独占所有权，同一时刻只能有一个指针持有对象，
	//      不能复制、只能移动；对象销毁时自动释放。适合「单一主人」的资源，如组件。
	// 两者都 RAII（资源获取即初始化）：析构时自动 delete，避免手动管理内存的泄漏。
	template <typename T> using RefPtr = std::shared_ptr<T>;
	template <typename T> using UniquePtr = std::unique_ptr<T>;
}