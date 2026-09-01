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

// 材质资源实现：把 .hlsl 着色器文件当作"材质"——编译着色器、创建管线、
// 存放材质参数（原始字节）与纹理列表。即 材质 = 着色器 + 参数 + 纹理。
#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Graphics/GraphicsPipelineLayout.h>

#include <fstream>
#include <filesystem>

// 主构造函数：读取 .hlsl 源码、编译 VS/PS、创建管线布局与状态、预留纹理槽位。
dx3d::MaterialResource::MaterialResource(const MaterialResourceDesc& desc) : Resource(desc.base), m_graphicsDevice(desc.graphicsDevice)
{
	// 材质的"源文件"就是一个 .hlsl 着色器文件。先取其路径。
	std::filesystem::path shaderFile = desc.base.path;

	auto shaderFileStr = shaderFile.string();
	// 用文件流打开 .hlsl，准备读取其中的着色器源代码文本。
	std::ifstream shaderStream(shaderFile);
	// 打开失败则抛异常。
	if (!shaderStream) DX3DLogThrowError("Failed to open shader file {}", shaderFileStr.c_str());
	// 用 istreambuf_iterator 逐字符读取整个文件内容到字符串 shaderCode。
	// （两个默认构造的"结束迭代器"表示空范围，配合起始迭代器实现"读到文件末尾"）
	std::string shaderCode{
		std::istreambuf_iterator<char>(shaderStream),
		std::istreambuf_iterator<char>()
	};

	// 编译同一个 .hlsl 中的两个入口：顶点着色器入口 _VSMain、像素着色器入口 _PSMain。
	// compileShader 返回编译后的着色器二进制(ShaderBinary)。
	auto vsBinary = m_graphicsDevice.compileShader({ shaderFileStr.c_str(), shaderCode.c_str(),
		shaderCode.size(), "_VSMain", ShaderType::VertexShader });
	auto psBinary = m_graphicsDevice.compileShader({ shaderFileStr.c_str(), shaderCode.c_str(),
		shaderCode.size(), "_PSMain", ShaderType::PixelShader });

	// 创建管线布局：描述这两个着色器需要什么样的顶点输入、常量缓冲、纹理槽位等。
	m_layout = m_graphicsDevice.createGraphicsPipelineLayout({ vsBinary, psBinary });
	// 基于布局创建管线状态对象(PSO)，渲染时绑定它即可绘制。
	m_pipeline = m_graphicsDevice.createGraphicsPipelineState({ *m_layout });
	// 按管线布局声明的纹理槽位数量，预留纹理列表空间（初始为空，后续用 setTexture 填充）。
	m_textures.resize(m_layout->getMaxTextureSlots());
}

// 拷贝构造（由 ResourceManager 在缓存命中材质时调用，"克隆"一份新材质）：
// 共享原材质的 m_layout/m_pipeline（shared_ptr 拷贝，不重新编译着色器），
// 但拥有独立的 m_data 参数与 m_textures 纹理列表。
dx3d::MaterialResource::MaterialResource(const MaterialResource& material, const MaterialResourceDesc& desc) : Resource(desc.base), m_graphicsDevice(desc.graphicsDevice)
{
	// 共享布局与管线状态（shared_ptr 赋值，仅增加引用计数，不重新编译/创建）。
	m_layout = material.m_layout;
	m_pipeline = material.m_pipeline;
	// 按管线布局声明的纹理槽位数量，预留纹理列表空间（初始为空，后续用 setTexture 填充）。
	m_textures.resize(m_layout->getMaxTextureSlots());
}

// 返回渲染用的管线状态对象(PSO)。
const dx3d::GraphicsPipelineState& dx3d::MaterialResource::getGraphicsPipelineState() const noexcept
{
	return *m_pipeline;
}

// 写入材质参数：把调用方提供的字节视图(data)拷贝进内部缓冲 m_data，超过 MaxDataSize 的部分截断。
// std::span<const std::byte> 是不持有数据所有权的只读字节视图（类似指针+长度）。
void dx3d::MaterialResource::setData(const std::span<const std::byte>& data)
{
	if (!data.size())
	{
		DX3DLogError("No material data provided.")
		return;
	}
	if (data.size() > MaxDataSize)
	{
		DX3DLogWarning("Material data size ({} bytes) exceeds the maximum allowed size of {} bytes. Data will be truncated.", data.size(), MaxDataSize)
	}

	// 把大小限制在 MaxDataSize 内，再 memcpy 拷贝到 m_data，并记录实际有效字节数。
	auto size = std::min(data.size(), MaxDataSize);
	memcpy(m_data, data.data(), size);
	m_dataSize = size;
}

// 返回内部材质参数缓冲的只读字节视图(std::span)，调用方可据此读取这些原始字节。
const std::span<const std::byte> dx3d::MaterialResource::getData() const noexcept
{
	return m_data;
}

// 取指定槽位的纹理（返回裸指针，不转移所有权；越界则记错误并返回空指针）。
dx3d::TextureResource* dx3d::MaterialResource::getTexture(size_t index)
{
	if (index >= m_textures.size())
	{
		DX3DLogError("Index {} is out of bounds for list of size {}", index, m_textures.size());
		return {};
	}
	return m_textures[index].get();
}

// 返回纹理槽位数量。
size_t dx3d::MaterialResource::getNumTextures() const noexcept
{
	return m_textures.size();
}

// 把纹理绑定到指定槽位（shared_ptr 赋值，共享所有权）；越界则记错误并忽略。
void dx3d::MaterialResource::setTexture(size_t index, const dx3d::RefPtr<TextureResource>& texture)
{
	if (index >= m_textures.size())
	{
		DX3DLogError("Index {} is out of bounds for list of size {}", index, m_textures.size());
		return;
	}
	m_textures[index] = texture;
}
