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
// 文件职责：集中定义引擎中所有的「描述结构体（Desc）」与部分公共枚举/数据结构。
//
// 核心设计模式 —— Desc + 构造函数：
//   引擎创建对象时，不把一堆零散参数挨个传给构造函数，而是把所有参数
//   打包成一个「Desc 结构体」整体传入。好处：
//     1) 参数有名字，调用处一眼能看懂每个值含义；
//     2) 以后加参数只需给 Desc 加字段、给默认值，不破坏已有调用代码；
//     3) 派生类的 Desc 可内嵌基类的 Desc（如 ResourceDesc 内嵌 BaseDesc），
//        形成层次清晰的参数链。
//
// 架构位置：被 Core/Base.h、各子系统头文件依赖，是最常被 include 的文件之一。
// 协作对象：各 Desc 由对应的类（Window/Display/GraphicsDevice...）的构造函数接收。
// 初学者提示：看到 XxxDesc 就把它理解成「创建 Xxx 所需的一张配置表」。
// =============================================================================
#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Math/Rect.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec2.h>

namespace dx3d
{
	// BaseDesc：Base 类（引擎对象基类）的创建参数。
	// 只含一个 Logger 引用——因为 Base 唯一持有的就是日志器。
	// 几乎所有派生类的 Desc 都会内嵌一个 BaseDesc，从而把 Logger 传下去。
	struct BaseDesc
	{
		Logger& logger;
	};

	// WindowDesc：创建窗口所需配置。内嵌 BaseDesc 以获得日志器。
	struct WindowDesc
	{
		BaseDesc base;
		Rect size{};
	};

	// DisplayDesc：创建显示目标所需配置。Display = 窗口 + 交换链，
	// 需要一个已创建好的 GraphicsDevice 来制造交换链。
	struct DisplayDesc
	{
		WindowDesc window;
		GraphicsDevice& graphicsDevice;
	};

	// GraphicsDeviceDesc：创建 D3D11 图形设备所需配置。
	// 目前无额外参数（设备只依赖日志器），留作以后扩展。
	struct GraphicsDeviceDesc
	{
		BaseDesc base;
	};

	// SwapChainDesc：创建交换链所需配置。
	// winHandle 用 void* 是为了隐藏平台类型（Win32 下实为 HWND），
	// 这样头文件不依赖 <windows.h>，保持跨平台外观。
	struct SwapChainDesc
	{
		void* winHandle{};
		Rect winSize{};
	};

	// ShaderType：着色器类型枚举。
	// D3D11 中顶点着色器（VS）负责变换顶点位置，像素着色器（PS）决定每个像素颜色。
	// enum class 是 C++11 的强类型枚举，不会隐式转成 int，避免误用。
	enum class ShaderType
	{
		VertexShader = 0,
		PixelShader
	};

	// ShaderCompileDesc：编译着色器源码所需配置。
	//   shaderSourceName：源文件名（仅用于日志/调试显示）
	//   shaderSourceCode：指向 HLSL 源代码字节
	//   shaderSourceCodeSize：源代码字节数
	//   shaderEntryPoint：入口函数名（如 "main" 或 "VSMain"）
	//   shaderType：是顶点还是像素着色器
	struct ShaderCompileDesc
	{
		const char* shaderSourceName{};
		const void* shaderSourceCode{};
		size_t shaderSourceCodeSize{};
		const char* shaderEntryPoint{};
		ShaderType shaderType{};
	};

	// GraphicsPipelineLayoutDesc：创建管线布局（着色器绑定）所需配置。
	// 引用两个着色器二进制（已编译好的字节码），用于把它们接入管线。
	// RefPtr 是 shared_ptr 别名——着色器二进制可被多个管线共享，故用共享所有权。
	struct GraphicsPipelineLayoutDesc
	{
		const RefPtr<ShaderBinary>& vsBinary;
		const RefPtr<ShaderBinary>& psBinary;
	};

	// BinaryData：通用二进制数据块，用 {指针, 大小} 描述一段内存。
	// 用 void* 可指向任意类型字节，是 C/C++ 中表示「裸内存」的常见方式。
	struct BinaryData
	{
		const void* data{};
		size_t dataSize{};
	};

	// GraphicsPipelineStateDesc：创建图形管线状态（PSO）所需配置。
	// PSO 封装了 D3D11 渲染状态（着色器、混合、深度等），切换状态时整体替换。
	struct GraphicsPipelineStateDesc
	{
		const GraphicsPipelineLayout& layout;
	};

	// VertexBufferDesc：创建顶点缓冲区所需配置。
	// vertexSize = 单个顶点字节数（决定如何切分顶点列表），如 sizeof(MeshVertex)。
	struct VertexBufferDesc
	{
		const void* vertexList{};
		ui32 vertexListSize{};
		ui32 vertexSize{};
	};

	// ConstantBufferDesc：创建常量缓冲区（CBuffer）所需配置。
	// 常量缓冲用于从 CPU 向 GPU 着色器传递「每帧/每对象」数据（如变换矩阵）。
	struct ConstantBufferDesc
	{
		const void* buffer{};
		ui32 bufferSize{};
	};

	// IndexBufferDesc：创建索引缓冲区所需配置。
	// 索引缓冲存放顶点索引，让 GPU 用少量顶点拼出复杂图元（三角形），避免重复存储顶点。
	struct IndexBufferDesc
	{
		const ui32* indexList{};
		ui32 indexListSize{};
	};



	// GameContext：运行期「游戏上下文」，把游戏运行所需的几大子系统引用打包在一起，
	// 便于在 World/GameObject/Component 之间统一传递，避免挨个传三个引用。
	struct GameContext
	{
		InputSystem& input;
		ResourceManager& resourceManager;
		GraphicsDevice& device;
	};

	// GameDesc：创建 Game（游戏主入口）所需配置。
	// 这是整个引擎最顶层的创建参数，用户通常只配窗口大小与日志级别。
	struct GameDesc
	{
		Rect windowSize{ 1280,720 };
		Logger::LogLevel logLevel = Logger::LogLevel::Error;
	};

	// WorldDesc：创建 World 所需配置。内嵌 BaseDesc 与 GameContext。
	struct WorldDesc 
	{
		BaseDesc base;
		GameContext gameContext;
	};

	// GameObjectDesc：创建 GameObject 所需配置。需指向所属 World 与运行上下文。
	struct GameObjectDesc
	{
		BaseDesc base;
		GameContext gameContext;
		World& world;
	};

	// ComponentDesc：创建 Component 所需配置。
	// 组件必须知道自己挂在哪个 GameObject 上、属于哪个 World、以及运行上下文。
	// 注意 context 用引用传递，保证引用的是外部真实对象而非拷贝。
	struct ComponentDesc
	{
		BaseDesc base;
		GameObject& object;
		World& world;
		GameContext& context;
	};

	// WorldRendererDesc：创建世界渲染器所需配置。需指向 GraphicsDevice 以执行绘制。
	struct WorldRendererDesc
	{
		BaseDesc base;
		GraphicsDevice& engine;
	};

	// KeyCode：统一键码枚举。把 Win32 虚拟键码（VK_*）抽象成引擎自己的枚举，
	// 让上层逻辑不直接依赖 Windows 头文件。
	// Count 放在最后，其数值等于有效按键数量，常用于遍历或数组定长。
	enum class KeyCode
	{
		Unknown = 0,
		// Letters
		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T, U,
		V, W, X, Y, Z,

		// Numbers
		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,


		Escape,
		Shift,
		Space,
		Enter,

		// Mouse buttons (optional inclusion)
		MouseLeft,
		MouseRight,
		MouseMiddle,

		// Arrows
		Up,
		Down,
		Left,
		Right,

		Count
	};

	// InputSystemDesc：创建输入系统所需配置。目前仅需日志器。
	struct InputSystemDesc
	{
		BaseDesc base;
	};

	// ResourceDesc：资源基类的创建配置。所有资源（材质/纹理/网格）的 Desc 都内嵌它。
	// path 是宽字符字符串（wchar_t*），因为 Windows 文件 API 用宽字符支持中文等路径。
	struct ResourceDesc
	{
		BaseDesc base;
		const wchar_t* path{};
		ResourceManager& manager;
	};

	// MeshResourceDesc：网格资源创建配置。内嵌 ResourceDesc 再追加 GraphicsDevice 引用，
	// 因为资源加载后需在 GPU 上创建对应的 D3D11 资源（缓冲区/纹理）。
	struct MeshResourceDesc
	{
		ResourceDesc base;
		GraphicsDevice& graphicsDevice;
	};
	// MaterialResourceDesc：材质资源创建配置。结构与 MeshResourceDesc 相同。
	struct MaterialResourceDesc
	{
		ResourceDesc base;
		GraphicsDevice& graphicsDevice;
	};
	// TextureResourceDesc：纹理资源创建配置。结构与上两者相同。
	struct TextureResourceDesc
	{
		ResourceDesc base;
		GraphicsDevice& graphicsDevice;
	};

	// SystemContext：系统级上下文，目前仅含图形设备引用。
	// 与 GameContext 区别：SystemContext 侧重「系统级单例依赖」，GameContext 侧重「游戏运行依赖」。
	struct SystemContext
	{
		GraphicsDevice& graphicsDevice;
	};
		
	// ResourceManagerDesc：创建资源管理器所需配置。内嵌 SystemContext。
	struct ResourceManagerDesc
	{
		BaseDesc base;
		SystemContext context;
	};

	// TextureDesc：创建 D3D11 纹理所需配置。size 为纹理宽高，pixels 为初始像素数据。
	struct TextureDesc
	{
		Rect size{};
		const void* pixels{};
	};
	// SamplerDesc：创建采样器（Sampler）所需配置。
	// 采样器决定纹理如何被采样（过滤方式、寻址模式等）。目前无参数，使用默认采样状态。
	struct SamplerDesc
	{
	};


	// MeshVertex：网格顶点的内存布局。GPU 顶点缓冲区里每个顶点就长这样。
	// 顺序对应顶点着色器输入签名（input layout）：
	//   position：3D 位置（模型空间坐标）
	//   texcoord：纹理坐标（UV，决定贴图如何贴到表面）
	//   normal：法线（垂直于表面的单位向量，用于光照计算）
	struct MeshVertex
	{
		Vec3 position{};
		Vec2 texcoord{};
		Vec3 normal{};
	};

	// MaterialSlot：材质槽位。一个网格可能由多个材质组成，
	// 每个 MaterialSlot 描述「连续的一段索引区间使用哪个材质」。
	//   startIndex：这段起始索引（在 IndexBuffer 中的位置）
	//   indexCount：这段有多少个索引
	//   materialIndex：对应材质在材质列表中的下标（-1 表示无材质）
	struct MaterialSlot
	{
		ui32 startIndex{};
		ui32 indexCount{};
		i32 materialIndex{};
	};






}