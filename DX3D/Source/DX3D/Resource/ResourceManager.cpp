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

// 资源管理器实现：核心是"缓存去重 + 按扩展名分发创建"，并包含材质的克隆特例。
// 关键流程：先按路径查缓存 → 命中则复用（材质则克隆）→ 未命中则按扩展名创建 → 存入缓存。
#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/TextureResource.h>
#include <DX3D/Resource/MeshResource.h>

#include <filesystem>

// 构造函数：Base(desc.base) 绑定日志器；m_context 保存系统上下文（含图形设备引用）。
dx3d::ResourceManager::ResourceManager(const ResourceManagerDesc& desc) : Base(desc.base), m_context(desc.context)
{
}

// 核心：按文件路径创建或获取资源（已缓存则复用），返回基类指针 RefPtr<Resource>。
// 由公开模板 createResourceFromFile<T> 调用，再用 dynamic_pointer_cast 转型为具体类型。
dx3d::RefPtr<dx3d::Resource> dx3d::ResourceManager::createResourceFromFileConcrete(const wchar_t* file_path)
{
	// std::filesystem::path：跨平台路径对象，可方便地取扩展名、判断文件是否存在等。
	std::filesystem::path resourcePath{ file_path };
	auto ext = resourcePath.extension();

	// 1) 缓存查找：以路径为键查哈希表，命中说明该文件之前已加载过（去重的关键）。
	auto it = m_resources.find(file_path);
	if (it != m_resources.end())
	{
		// 材质特例：若缓存里是材质（MaterialResource），不直接返回原对象，而是克隆一份。
		// 原因：材质携带参数数据与纹理列表，不同使用者需要各自独立的一份；
		// 但克隆会复用同一份已编译的着色器/管线（见 MaterialResource 拷贝构造），避免重复编译开销。
		auto mat = std::dynamic_pointer_cast<MaterialResource>(it->second);
		if (mat)
			return std::make_shared<MaterialResource>(*mat, MaterialResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
		return it->second;
	}

	// 2) 文件不存在则报错并返回空指针。
	if (!std::filesystem::exists(resourcePath))
	{
		DX3DLogError("File {} doesn't exist.", resourcePath.string().c_str());
		return nullptr;
	}

	// 3) 缓存未命中：按扩展名分发，创建对应类型的资源。
	RefPtr<Resource> resPtr{};
	try
	{
		// .hlsl / .fx 视为材质（着色器文件）：编译着色器并创建管线状态。
		if (!ext.compare(L".hlsl") || !ext.compare(L".fx"))
			resPtr = std::make_shared<MaterialResource>(MaterialResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
		// .jpg / .png 视为纹理图片：用 stb_image 解码后创建 GPU 纹理。
		if (!ext.compare(L".jpg") || !ext.compare(L".png"))
			resPtr = std::make_shared<TextureResource>(TextureResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
		// .obj 视为网格模型：用 tinyobjloader 解析顶点/索引/材质槽位并创建 GPU 缓冲。
		if (!ext.compare(L".obj"))
			resPtr = std::make_shared<MeshResource>(MeshResourceDesc{ getResourceDesc(file_path), m_context.graphicsDevice });
	}
	// 捕获任何创建异常并记日志，避免单个资源加载失败导致整个引擎崩溃。
	catch (...)
	{
		DX3DLogError("Failed to load resource {}", resourcePath.string().c_str());
	}

	// 4) 创建成功则存入缓存并返回，使后续对同一路径的请求直接复用（去重）。
	if (resPtr)
	{
		m_resources.emplace(file_path, resPtr);
		return resPtr;
	}

	return nullptr;
}

// 组装一个 ResourceDesc：把日志器、路径、本管理器打包，传给具体资源的构造函数。
dx3d::ResourceDesc dx3d::ResourceManager::getResourceDesc(const wchar_t* file_path)
{
	return ResourceDesc{ { m_logger }, file_path, *this };
}

