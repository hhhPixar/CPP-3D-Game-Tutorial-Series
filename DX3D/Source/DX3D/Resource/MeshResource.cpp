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

// 网格资源实现：用第三方库 tinyobjloader 解析 .obj 文件，组装顶点/索引/材质槽位，
// 并在 GPU 上创建 VertexBuffer / IndexBuffer。过程中包含从 OBJ 坐标系到
// Direct3D 坐标系的转换：Z 轴取反（右手系→左手系）、纹理 V 翻转（原点左下→左上）、
// 三角形顶点顺序调整（逆时针 CCW→顺时针 CW，配合背面剔除约定）。
#include <DX3D/Resource/MeshResource.h>
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec2.h>

#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Resource/ResourceManager.h>

#include <filesystem>
#include <ranges>
#include <map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>


// 构造函数：解析 .obj 文件、组装顶点与索引、切分材质槽位、创建 GPU 缓冲。
// 整个过程在一次构造中完成，之后该对象即持有可绘制所需的全部 GPU 资源。
dx3d::MeshResource::MeshResource(const MeshResourceDesc& desc) : Resource(desc.base)
{
	// 把宽字符路径转成窄字符串（std::string），因为 tinyobjloader 的接口用 std::string。
	auto inputfile{ std::filesystem::path(desc.base.path).string() };
	// tinyobj::ObjReader 是 tinyobjloader 库提供的 .obj 读取器。
	// 调用 ParseFromFile 后，可通过 GetAttrib()/GetShapes() 取到顶点、法线、纹理坐标、形状等。
	tinyobj::ObjReader reader{};

	// 解析 .obj 文件（也会读取其引用的 .mtl 材质文件）。下面三条分别处理：
	// 解析失败（抛异常）、错误信息、警告信息。
	if (!reader.ParseFromFile(inputfile)) DX3DLogThrowError("Mesh failed to load.");
	if (!reader.Error().empty()) DX3DLogThrowError("Failed to load mesh {}. Details: {}", inputfile, reader.Error());
	if (!reader.Warning().empty()) DX3DLogWarning("Mesh {} loaded with warnings: {}", inputfile, reader.Warning());

	// attribs 是"属性表"，含三个扁平数组：vertices（位置 xyz 连续存放）、
	// texcoords（纹理 uv）、normals（法线 xyz）。第 i 个顶点位置在 vertices[i*3+0/1/2]。
	const auto& attribs = reader.GetAttrib();
	// shapes 是"形状"列表（一个 .obj 可含多个 mesh/形状），每个形状由若干"面(face)"组成；
	// 每个面记录它用了哪些顶点/纹理坐标/法线索引，以及它属于哪个材质。
	const auto& shapes = reader.GetShapes();

	// 下面两个 vector 是本函数的输出：顶点列表与索引列表（稍后用于创建 GPU 缓冲）。
	std::vector<MeshVertex> listVertices{};
	// 总索引列表：稍后按材质顺序拼入，最终传给 IndexBuffer。
	std::vector<dx3d::ui32> listIndices{};

	// 按材质分组的索引表：键=材质编号，值=该材质下所有三角形索引。
	// 用 std::map（按键排序）而非 unordered_map，使最终材质槽位顺序稳定（按材质编号递增）。
	std::map<dx3d::i32, std::vector<dx3d::ui32>> indicesPerMaterial{};

	// 遍历每个形状（大多数简单模型只有一个形状）。
	for (const auto& shape : shapes)
	{
		auto indexOffset{ 0u };
		// 遍历该形状中的每个面（三角形/多边形）。
		// std::views::iota(0u, n) 是 C++20 范围视图，惰性生成 0,1,...,n-1 整数序列，不分配容器。
		for (auto f : std::views::iota(0u, shape.mesh.num_face_vertices.size()))
		{
			// 该面的顶点数。.obj 支持多边形（如四边形），不限于三角形；这里取出按需处理。
			auto numFaceVerts = shape.mesh.num_face_vertices[f];

			// 该面所属的材质编号（对应 .mtl 中的某条材质；可能为 -1 表示无材质）。
			// 下一行用 operator[] 取该材质的索引向量——若键不存在会自动创建空 vector，实现"按材质分组"。
			int material = shape.mesh.material_ids[f];
			auto& materialIndices = indicesPerMaterial[material];
			// 记下当前面索引在该材质分组中的起始位置，供后面做三角形顶点顺序调整(winding swap)用。
			auto startIndexMaterial{ materialIndices.size() };

			// 遍历该面的每个顶点，逐个组装成 MeshVertex 并加入顶点列表。
			for (auto v : std::views::iota(0u, numFaceVerts))
			{
				// tinyobj 的索引：分别指向 attribs 中 position/texcoord/normal 的下标（可能为 -1 表示缺失）。
				auto& index{ shape.mesh.indices[indexOffset + v] };
				
				// 组装一个输出顶点：位置、纹理坐标、法线都从 attribs 的扁平数组里取出来。
				MeshVertex vertex{};

				// 位置：从 vertices 数组取 xyz。注意 Z 分量取反（前面加负号），
				// 把 OBJ 的右手坐标系转换到 Direct3D 的左手坐标系（Z 朝向相反）。
				vertex.position =  Vec3{
					attribs.vertices[index.vertex_index * 3 + 0],
					attribs.vertices[index.vertex_index * 3 + 1],
					-attribs.vertices[index.vertex_index * 3 + 2]
				};

				// 纹理坐标：从 texcoords 数组取 uv。注意 V 分量做 1.0 - v 翻转，
				// 因为 OBJ 的纹理原点在左下角，而 Direct3D 的纹理原点在左上角。
				if (index.texcoord_index >= 0)
				{
					vertex.texcoord = Vec2{
						attribs.texcoords[index.texcoord_index * 2 + 0],
						1.0f - attribs.texcoords[index.texcoord_index * 2 + 1]
					};
				}

				// 法线：从 normals 数组取 xyz。同样对 Z 分量取反，与位置的坐标转换保持一致。
				if (index.normal_index >= 0)
				{
					vertex.normal = Vec3{
					attribs.normals[index.normal_index * 3 + 0],
					attribs.normals[index.normal_index * 3 + 1],
					-attribs.normals[index.normal_index * 3 + 2]
					};
				}

				// 把这个顶点在 listVertices 中的下标加入当前材质的索引表，
				// 再把顶点本身存入 listVertices（这里不做跨面顶点去重，每个面顶点都单独存一份）。
				materialIndices.push_back(static_cast<dx3d::ui32>(listVertices.size()));
				listVertices.push_back(vertex);
			}
			// 调整三角形顶点顺序（绕序）：交换第 2、3 个索引，把 OBJ 默认的逆时针(CCW)
			// 改成 Direct3D 默认的顺时针(CW)前向面，以配合背面剔除(back-face culling)的约定。
			std::swap(materialIndices[startIndexMaterial + 1], materialIndices[startIndexMaterial + 2]);

			indexOffset += numFaceVerts;
		}
	}

	// 遍历按材质分好的索引（std::map 已按键排序），把它们顺序拼进总索引表，
	// 并为每个材质生成一个 MaterialSlot。
	for (const auto& [materialIndex, materialIndices] : indicesPerMaterial)
	{
		// 构造一个材质槽位：startIndex=该材质索引在总表中的起点，indexCount=该材质的索引数，
		// materialIndex=材质编号。渲染时按槽位逐段绘制，每段绑定对应材质后绘制其三角形。
		MaterialSlot slot{	
			static_cast<dx3d::ui32>(listIndices.size()),
			static_cast<dx3d::ui32>(materialIndices.size()),
			materialIndex
		};

		listIndices.insert(
			listIndices.end(),
			materialIndices.begin(),
			materialIndices.end());

		m_matSlots.push_back(slot);
	}

	// 用组装好的顶点列表创建 GPU 顶点缓冲；vertexSize = sizeof(MeshVertex) 告诉 GPU 每个顶点的字节大小。
	m_vertexBuffer = desc.graphicsDevice.createVertexBuffer(
		{listVertices.data() ,static_cast<dx3d::ui32>(listVertices.size()), sizeof(MeshVertex)});
	// 用组装好的索引列表创建 GPU 索引缓冲（每个索引是一个 ui32，指向顶点缓冲中的顶点）。
	m_indexBuffer = desc.graphicsDevice.createIndexBuffer(
		{listIndices.data(), static_cast<dx3d::ui32>(listIndices.size())});
}


// 返回材质槽数组，并通过输出参数 numSlots 给出数量。供渲染器逐材质分段绘制。
const dx3d::MaterialSlot* dx3d::MeshResource::getMaterialSlots(ui32& numSlots) const noexcept
{
	numSlots = static_cast<dx3d::ui32>(m_matSlots.size());
	return m_matSlots.data();
}


// 返回材质槽位数量。
dx3d::ui32 dx3d::MeshResource::getNumMaterialSlots() const noexcept
{
	return static_cast<dx3d::ui32>(m_matSlots.size());
}

// 返回顶点缓冲的引用。
const dx3d::VertexBuffer& dx3d::MeshResource::getVertexBuffer() const noexcept
{
	return *m_vertexBuffer;
}

// 返回索引缓冲的引用。
const dx3d::IndexBuffer& dx3d::MeshResource::getIndexBuffer() const noexcept
{
	return *m_indexBuffer;
}

