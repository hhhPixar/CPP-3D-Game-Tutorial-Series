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
// Sampler.cpp —— Sampler 的实现
// 填写 D3D11_SAMPLER_DESC 描述采样策略（过滤 + 寻址），再调用
// ID3D11Device::CreateSamplerState 创建采样器状态对象。
// 当前配置：各向异性过滤 + 三方向重复(WRAP)寻址——适合大多数贴图。
// ============================================================================
#include "Sampler.h"

// 构造函数实现：填写采样器描述并创建采样器状态。
// desc 当前为空结构（配置写死在此处）；gDesc 提供 device 等公共依赖。
dx3d::Sampler::Sampler(const SamplerDesc& desc, const GraphicsResourceDesc& gDesc) : GraphicsResource(gDesc)
{
	// 填写采样器描述 D3D11_SAMPLER_DESC：决定纹理如何被采样（过滤 + 寻址）。
	D3D11_SAMPLER_DESC sampler_desc = {};
	// U/V/W 三方向都用 D3D11_TEXTURE_ADDRESS_WRAP：UV 超出 0~1 时重复平铺纹理（如地面砖墙）。
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// D3D11_FILTER_ANISOTROPIC：各向异性过滤，斜视角看纹理时质量更好（代价是更耗性能）。
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	// LOD（mip 层级）范围：MinLOD=0、MaxLOD=最大值，表示允许使用全部 mip 层级。
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	// 真正创建采样器状态：CreateSamplerState 按 sampler_desc 创建 ID3D11SamplerState，输出 m_sampler。
	// 失败则记日志并抛异常。之后由 DeviceContext::setSamplers 绑定到着色器。
	DX3DGraphicsLogThrowOnFail(m_device.CreateSamplerState(&sampler_desc, &m_sampler), "CreateSamplerState failed.");
}
