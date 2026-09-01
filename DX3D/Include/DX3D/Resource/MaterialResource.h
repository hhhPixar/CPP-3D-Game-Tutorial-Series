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
// 材质资源（MaterialResource）—— 把一个 .hlsl 着色器文件当作"材质"
// 设计理念：材质 = 着色器程序 + 参数数据 + 纹理列表。
//   · 着色器：读取 .hlsl 源码，编译顶点着色器(_VSMain)与像素着色器(_PSMain)，
//     创建 GraphicsPipelineLayout（描述着色器输入签名/纹理槽位）与 GraphicsPipelineState（PSO）。
//   · 参数数据：材质参数以"原始字节"(std::byte) 存于 m_data（上限 MaxDataSize=256 字节），
//     用 setData/getData 以 std::span 视图读写——std::span 是不持有数据所有权的轻量视图
//     （类似指针+长度，调用方负责保证所指数据有效）。
//   · 纹理列表：按管线布局声明的纹理槽位数量存放纹理，可逐槽位 setTexture/getTexture。
// 克隆特性：ResourceManager 每次取用材质都会拷贝一份（见拷贝构造），让每个使用者拥有
//   独立的参数与纹理；而 m_layout/m_pipeline 用 shared_ptr 共享，避免重复编译着色器。
// ============================================================================
#pragma once
#include <DX3D/Resource/Resource.h>
#include <vector>
#include <span>

namespace dx3d
{
	
	// 材质资源类：final 表示不可再被继承。一个材质 = 着色器管线 + 参数字节缓冲 + 纹理列表。
	class MaterialResource final: public Resource
	{
	public:
		// 构造函数：读取并编译 .hlsl 着色器，创建管线布局与管线状态，预留纹理槽位。详见 .cpp。
		explicit MaterialResource(const MaterialResourceDesc& desc);
		// 拷贝构造：由 ResourceManager 在缓存命中材质时调用，"克隆"出一份新材质。
		// 共享原材质的 m_layout/m_pipeline（shared_ptr 拷贝，不重新编译着色器），
		// 但拥有独立的 m_data 参数与 m_textures 纹理列表。
		MaterialResource(const MaterialResource& material, const MaterialResourceDesc& desc);

		// 取渲染用的管线状态对象（PSO），渲染器绑定后即可绘制使用本材质的几何。
		const GraphicsPipelineState& getGraphicsPipelineState() const noexcept;
		// 写入材质参数：把一段原始字节（std::span<const std::byte> 视图）拷贝进 m_data，
		// 超过 MaxDataSize 的部分会被截断。这些字节通常对应着色器里的常量缓冲数据。
		void setData(const std::span<const std::byte>& data);
		// 读取材质参数：返回指向 m_data 字节缓冲的 std::span 视图（不拷贝，仅暴露内部数据）。
		const std::span<const std::byte> getData() const noexcept;
		
		// 取指定槽位的纹理（返回裸指针，不转移所有权；越界返回空指针）。
		TextureResource* getTexture(size_t index);
		// 返回纹理槽位数量（由管线布局决定）。
		size_t getNumTextures()  const noexcept;
		// 把一个纹理绑定到指定槽位（用 shared_ptr 共享所有权）；越界则忽略。
		void setTexture(size_t index, const dx3d::RefPtr<TextureResource>& texture);
	public:
		// 材质参数的最大字节数（256 字节）。材质数据存于一个固定大小的字节缓冲中。
		static constexpr std::size_t MaxDataSize{ 256 };
	private:
		// 引用图形设备（用于编译着色器、创建管线等）。
		GraphicsDevice& m_graphicsDevice;

		// 管线布局：描述着色器的输入签名（顶点结构、常量缓冲、纹理槽位等）。可被多份材质共享。
		RefPtr<GraphicsPipelineLayout> m_layout{};	
		// 管线状态（PSO）：完整渲染状态，渲染时绑定它即可绘制。可被多份材质共享。
		RefPtr<GraphicsPipelineState> m_pipeline{};

		// 纹理列表：按槽位存放，每个槽位可绑定一个 TextureResource。
		std::vector<RefPtr<TextureResource>> m_textures{};

		// 材质参数的字节缓冲（原始内存，按字节访问）。实际有效数据量由 m_dataSize 记录。
		std::byte m_data[MaxDataSize]{};
		// 当前材质参数的有效字节数（setData 时记录）。
		size_t m_dataSize{};
	};

}