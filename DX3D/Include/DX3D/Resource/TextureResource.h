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
// 纹理资源（TextureResource）—— 把一张图片文件加载成 GPU 可采样的纹理
// 职责：用第三方库 stb_image 读取 .jpg/.png 等图片（强制转为 RGBA 像素），
//       再用 GraphicsDevice 把像素数据上传到 GPU，创建一个 Texture 对象。
// 架构位置：资源系统，派生自 Resource；由 ResourceManager 在 .jpg/.png 扩展名时创建并缓存。
// 关键概念：纹理是只读共享数据，同一图片在缓存中只加载一次，可被多个材质共用。
// ============================================================================
#pragma once
#include <DX3D/Resource/Resource.h>

namespace dx3d
{
	// 纹理资源类：final 表示不可再被继承。持有一个 GPU 纹理对象。
	class TextureResource final : public Resource
	{
	public:
		// 构造函数：加载图片并创建 GPU 纹理。详见 .cpp。
		explicit TextureResource(const TextureResourceDesc& desc);
		// 取内部 GPU 纹理对象的引用，供材质/采样器绑定时使用。
		dx3d::Texture& getTexture();
	private:
		// GPU 纹理对象（用 shared_ptr 管理生命周期）。
		RefPtr<Texture> m_texture{};
	};
}

