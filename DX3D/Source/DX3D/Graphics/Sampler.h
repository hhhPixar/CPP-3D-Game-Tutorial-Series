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
// Sampler.h —— 采样器状态（Sampler State）
// 所属子系统：Direct3D 11 图形渲染 / GPU 纹理资源。
// 职责：控制"如何采样纹理"——过滤方式（缩放时怎么混合像素）与寻址方式
//       （UV 超出 0~1 时怎么处理：重复 wrap、钳制 clamp 等）。
// 架构位置：继承自 GraphicsResource；由 GraphicsDevice::createSampler 创建，
//           由 DeviceContext::setSamplers 绑定到着色器（与纹理 SRV 分属不同槽位）。
// 关键概念：纹理给"图像数据"，采样器给"怎么读这张图"。两者分开绑定：
//           纹理→着色器资源槽(SRV)，采样器→采样器槽(SamplerState)。
// ============================================================================
#pragma once
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d
{
	// 采样器：告诉 GPU 怎么采样纹理（过滤 + 寻址）。final 表示不可再被继承。
	// 与纹理是两套独立资源：纹理提供数据，采样器提供"读取策略"。
	class Sampler final : public GraphicsResource
	{
	public:
		// 构造：依 desc（当前为空结构，过滤/寻址配置写死在实现里）创建采样器状态；
		// gDesc 提供 device 等公共依赖。
		Sampler(const SamplerDesc& desc, const GraphicsResourceDesc& gDesc);

	private:
		// 底层采样器状态对象（ID3D11SamplerState）；ComPtr 自动释放 COM 对象。
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler{};
		// 友元：DeviceContext 需直接访问 m_sampler 把它绑定到着色器，故授予私有访问权限。
		friend class DeviceContext;
	};
}
