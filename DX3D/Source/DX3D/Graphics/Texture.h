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
// Texture.h —— 纹理（Texture）
// 所属子系统：Direct3D 11 图形渲染 / GPU 缓冲与纹理资源。
// 职责：把一张 2D 图像（像素数组）上传到 GPU，并创建"着色器资源视图"SRV，让
//       顶点/像素着色器能采样它（贴在 3D 物体表面）。
// 架构位置：继承自 GraphicsResource；由 GraphicsDevice::createTexture 创建，
//           由 DeviceContext::setTextures 绑定（实际绑定的就是 SRV）。
// 关键概念：ID3D11Texture2D 是 GPU 里的图像内存；但着色器不能直接读它，需要一个
//           ID3D11ShaderResourceView（SRV）"视图"才能采样。格式 R8G8B8A8_UNORM
//           表示每通道 8 位、归一化到 0~1。D3D11_BIND_SHADER_RESOURCE 标记可被着色器读。
// ============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 纹理：一张 GPU 图像 + 让着色器能采样它的 SRV 视图。
	// m_texture 是图像数据本身；m_srv 是"着色器资源视图"，着色器通过它读取纹理。
	// final 表示不可再被继承。一张纹理通常对应一张贴图（漫反射贴图、法线贴图等）。
	class Texture final : public GraphicsResource
	{
	public:
		// 构造：依 desc（size 宽高、pixels 像素数组）创建 GPU 纹理并建立 SRV；
		// gDesc 提供 device 等公共依赖。像素格式固定为 R8G8B8A8_UNORM（每像素 4 字节）。
		Texture(const TextureDesc& desc, const GraphicsResourceDesc& gDesc);
	private:
		// GPU 里的 2D 图像内存（ID3D11Texture2D）；ComPtr 自动释放 COM 对象。
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture{};
		// 着色器资源视图 SRV：着色器通过它采样纹理；DeviceContext::setTextures 绑定的就是它。
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv{};
		// 友元：DeviceContext 需直接访问 m_srv 把它绑定到着色器，故授予私有访问权限。
		friend class DeviceContext;
	};
}
