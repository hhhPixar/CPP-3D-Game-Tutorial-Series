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

#pragma once
#include <DX3D/Resource/Resource.h>
#include <vector>

namespace dx3d
{
	class MeshResource final : public Resource
	{
	public:
		explicit MeshResource(const MeshResourceDesc& desc);
		const MaterialSlot * getMaterialSlots(ui32& numSlots) const noexcept;

		ui32 getNumMaterialSlots() const noexcept;
		const VertexBuffer& getVertexBuffer() const noexcept;
		const IndexBuffer& getIndexBuffer() const noexcept;
	private:
		RefPtr<VertexBuffer> m_vertexBuffer{};
		RefPtr<IndexBuffer> m_indexBuffer{};
		std::vector<MaterialSlot> m_matSlots{};
	};
}