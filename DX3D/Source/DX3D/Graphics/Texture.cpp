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
// Texture.cpp —— Texture 的实现
// 两步：1) ID3D11Device::CreateTexture2D 创建 GPU 2D 纹理（图像内存）并填入像素；
//       2) ID3D11Device::CreateShaderResourceView 创建 SRV，让着色器能采样这张纹理。
// 纹理内存本身不能被着色器直接读，必须通过 SRV"视图"才能采样。
// ============================================================================
#include "Texture.h"

// 构造函数实现：创建 GPU 纹理 + SRV。
// desc 含 size（宽高）与 pixels（CPU 端像素数组）；gDesc 提供 device 等公共依赖。
dx3d::Texture::Texture(const TextureDesc& desc, const GraphicsResourceDesc& gDesc) : GraphicsResource(gDesc)
{
	// 参数校验：宽 / 高 / 像素指针任一为 0 或空就抛异常（带日志）。
	if (!desc.size.width) DX3DLogThrowInvalidArg("Width must be non-zero.");
	if (!desc.size.height) DX3DLogThrowInvalidArg("Height must be non-zero.");
	if (!desc.pixels) DX3DLogThrowInvalidArg("Pixels must be a valid array.");

	// 填写 2D 纹理描述 D3D11_TEXTURE2D_DESC：告诉 GPU 这张图多大、什么格式、怎么用。
	D3D11_TEXTURE2D_DESC texDesc{};
	// 纹理宽 / 高（像素）。
	texDesc.Width = desc.size.width;
	texDesc.Height = desc.size.height;
	// MipLevels=1：只用一层 mip（不自动生成多级渐变）。ArraySize=1：单张纹理（非纹理数组）。
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	// 像素格式 R8G8B8A8_UNORM：每通道 8 位、4 通道，归一化到 0~1（最常见的 RGBA 贴图格式）。
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// SampleDesc.Count=1：不使用多重采样（MSAA），普通纹理。
	texDesc.SampleDesc.Count = 1;
	// D3D11_USAGE_DEFAULT：GPU 可读写、CPU 不能直接 Map（创建时一次性填入初值即可）。
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	// D3D11_BIND_SHADER_RESOURCE：标记此纹理可被着色器读取（需配合下面的 SRV）。
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 初始数据 D3D11_SUBRESOURCE_DATA：创建纹理时把 CPU 端像素数组一次性拷进 GPU。
	D3D11_SUBRESOURCE_DATA initData{};
	// pSysMem 指向 CPU 端像素数组作为拷贝源。
	initData.pSysMem = desc.pixels;
	// SysMemPitch：一行像素占多少字节 = 宽度 × 4（每像素 4 字节 RGBA）。GPU 据此按行拷贝。
	initData.SysMemPitch = desc.size.width * 4;

	// 真正创建 GPU 纹理：CreateTexture2D 按 texDesc 分配显存、用 initData 填充，输出 m_texture。
	// 失败则记日志并抛异常。
	DX3DGraphicsLogThrowOnFail(m_device.CreateTexture2D(&texDesc, &initData, &m_texture),
		"CreateTexture2D failed.");

	// 写着色器资源视图描述 D3D11_SHADER_RESOURCE_VIEW_DESC：说明这个 SRV 怎么解读纹理。
	// 视图（view）是 D3D11 解耦"资源内存"与"如何使用"的机制：同一块纹理可被不同视图使用。
	D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	// 视图格式与纹理一致：R8G8B8A8_UNORM。
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// ViewDimension=TEXTURE2D：这是 2D 纹理的 SRV（不是立方体、数组等）。
	resDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	// 只暴露 1 层 mip（从最详细层 MostDetailedMip=0 开始）。
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Texture2D.MostDetailedMip = 0;

	// 创建 SRV：基于已创建的 m_texture 生成着色器可采样的视图，输出 m_srv。
	// 之后 DeviceContext::setTextures 绑定的就是 m_srv。失败则记日志并抛异常。
	DX3DGraphicsLogThrowOnFail(m_device.CreateShaderResourceView(m_texture.Get(), &resDesc,
		&m_srv), "CreateShaderResourceView failed.");
}
