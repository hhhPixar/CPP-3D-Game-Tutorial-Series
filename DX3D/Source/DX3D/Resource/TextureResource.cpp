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

// 纹理资源实现：用 stb_image 把图片解码为内存中的 RGBA 像素，再上传到 GPU 创建 Texture。
#include <DX3D/Resource/TextureResource.h>
#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <fstream>
#include <filesystem>
// stb_image 是单头文件库：在某个翻译单元中定义 STB_IMAGE_IMPLEMENTATION 宏后，
// 它的实现代码就只在本文件编译一次。本文件就是这个"唯一实现点"（不在 Vendor 范围内，仅调用）。
#define STB_IMAGE_IMPLEMENTATION
#include <stb-image/stb_image.h>


// 构造函数：加载图片文件并创建 GPU 纹理。流程：读路径→stbi_load 解码→createTexture 上传 GPU。
dx3d::TextureResource::TextureResource(const TextureResourceDesc& desc) : Resource(desc.base)
{
	// 取得图片路径并转为窄字符串（stb_image 的接口用 char*）。
	std::filesystem::path textureFile = desc.base.path;
	auto textureFileStr = textureFile.string();

	// width/height/channels 是 stbi_load 的输出参数：解码后填入图片宽、高、原始通道数。
	auto width{0}, height{0}, channels{0};
	// stbi_load 把图片解码到一块内存（返回像素指针，需后续由 stbi_image_free 释放）。
	// 第 5 个参数 STBI_rgb_alpha 强制输出为 4 通道 RGBA，保证 GPU 纹理格式统一。
	auto pixels = stbi_load(
		textureFileStr.c_str(),
		&width,
		&height,
		&channels,
		STBI_rgb_alpha // Force RGBA
	);

	// 解码失败（返回空指针）则抛异常。
	if (!pixels) DX3DLogThrowError("Failed to load texture file {}", textureFileStr.c_str());
	// 把内存中的像素上传到 GPU，创建一个 Texture 对象（{ {width,height}, pixels } 是 TextureDesc）。
	m_texture = desc.graphicsDevice.createTexture({ {width,height}, pixels });
}

// 返回内部 GPU 纹理的引用。
dx3d::Texture& dx3d::TextureResource::getTexture()
{
	return *m_texture;
}
