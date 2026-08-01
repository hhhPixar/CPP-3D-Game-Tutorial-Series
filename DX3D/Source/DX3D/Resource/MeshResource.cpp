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


dx3d::MeshResource::MeshResource(const MeshResourceDesc& desc) : Resource(desc.base)
{
	auto inputfile{ std::filesystem::path(desc.base.path).string() };
	tinyobj::ObjReader reader{};

	if (!reader.ParseFromFile(inputfile)) DX3DLogThrowError("Mesh failed to load.");
	if (!reader.Error().empty()) DX3DLogThrowError("Failed to load mesh {}. Details: {}", inputfile, reader.Error());
	if (!reader.Warning().empty()) DX3DLogWarning("Mesh {} loaded with warnings: {}", inputfile, reader.Warning());

	const auto& attribs = reader.GetAttrib();
	const auto& shapes = reader.GetShapes();

	std::vector<MeshVertex> listVertices{};
	std::vector<dx3d::ui32> listIndices{};

	std::map<dx3d::i32, std::vector<dx3d::ui32>> indicesPerMaterial{};

	for (const auto& shape : shapes)
	{
		auto indexOffset{ 0u };
		for (auto f : std::views::iota(0u, shape.mesh.num_face_vertices.size()))
		{
			auto numFaceVerts = shape.mesh.num_face_vertices[f];

			int material = shape.mesh.material_ids[f];
			auto& materialIndices = indicesPerMaterial[material];
			auto startIndexMaterial{ materialIndices.size() };

			for (auto v : std::views::iota(0u, numFaceVerts))
			{
				auto& index{ shape.mesh.indices[indexOffset + v] };
				
				MeshVertex vertex{};

				vertex.position =  Vec3{
					attribs.vertices[index.vertex_index * 3 + 0],
					attribs.vertices[index.vertex_index * 3 + 1],
					-attribs.vertices[index.vertex_index * 3 + 2]
				};

				if (index.texcoord_index >= 0)
				{
					vertex.texcoord = Vec2{
						attribs.texcoords[index.texcoord_index * 2 + 0],
						1.0f - attribs.texcoords[index.texcoord_index * 2 + 1]
					};
				}

				materialIndices.push_back(static_cast<dx3d::ui32>(listVertices.size()));
				listVertices.push_back(vertex);
			}
			std::swap(materialIndices[startIndexMaterial + 1], materialIndices[startIndexMaterial + 2]);

			indexOffset += numFaceVerts;
		}
	}

	for (const auto& [materialIndex, materialIndices] : indicesPerMaterial)
	{
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

	m_vertexBuffer = desc.graphicsDevice.createVertexBuffer(
		{listVertices.data() ,static_cast<dx3d::ui32>(listVertices.size()), sizeof(MeshVertex)});
	m_indexBuffer = desc.graphicsDevice.createIndexBuffer(
		{listIndices.data(), static_cast<dx3d::ui32>(listIndices.size())});
}


const dx3d::MaterialSlot* dx3d::MeshResource::getMaterialSlots(ui32& numSlots) const noexcept
{
	numSlots = static_cast<dx3d::ui32>(m_matSlots.size());
	return m_matSlots.data();
}


dx3d::ui32 dx3d::MeshResource::getNumMaterialSlots() const noexcept
{
	return static_cast<dx3d::ui32>(m_matSlots.size());
}

const dx3d::VertexBuffer& dx3d::MeshResource::getVertexBuffer() const noexcept
{
	return *m_vertexBuffer;
}

const dx3d::IndexBuffer& dx3d::MeshResource::getIndexBuffer() const noexcept
{
	return *m_indexBuffer;
}

