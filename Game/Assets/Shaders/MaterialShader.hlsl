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

#include "DX3D/Assets/Shaders/Common.hlsl"
// ============================================================================
// 所属子系统：HLSL 着色器 —— 材质着色器 MaterialShader.hlsl（带纹理材质）
// ----------------------------------------------------------------------------
// 通过 #include 引入 Common.hlsl 提供的框架：_VSMain（顶点变换）与 _PSMain（光照合成）
// 是真正被编译的入口；本文件只重写两个材质“钩子” VSMain / PSMain 来提供材质属性。
// 渲染流程位置：顶点变换(框架) -> 像素材质属性(此处 PSMain) -> 光照计算(框架 _PSMain)。
// 本材质：VSMain 留空；PSMain 用 Diffuse.Sample 采样纹理取漫反射色，
//         镜面色=白*specularStrength、高光指数=64，交给框架 _PSMain 做 环境光 + Phong 方向光。
// 关键概念：
//   cbuffer MaterialData(b3) —— 材质参数常量缓冲，C++ 端第 3 个 ConstantBuffer(cbs[3]=&materialCb)；
//   Texture2D Diffuse(t0)    —— 漫反射贴图，C++ 端纹理槽 0 的 ShaderResourceView；
//   TextureCoordinate        —— Common.hlsl 的全局 UV，框架在 _VSMain/_PSMain 写入，本文件读取以采样。
// ============================================================================

// 材质参数常量缓冲，寄存器 b3。C++ 端是第 3 个 ConstantBuffer（cbs[3]=&materialCb），
// WorldRenderer 每画物体前把材质参数写入此处（material->getData()）。
cbuffer MaterialData : register(b3)
{
    // specularStrength：镜面强度系数，整体缩放高光亮度（见 PSMain 里 specular = 白 * specularStrength）。
    float specularStrength;
};

// 漫反射（反照率）纹理。register(t0) 表示绑定到纹理槽 t0；C++ 端把材质的 Diffuse 贴图作为
// ShaderResourceView 在第 0 槽设置。PSMain 用 Diffuse.Sample(DefaultSampler, TextureCoordinate) 采样。
Texture2D Diffuse : register(t0);

// 顶点材质钩子：本材质无需自定义顶点处理，函数体留空。框架的 _VSMain 在做完坐标变换
// 后会调用它，让它有机会填充 MaterialVSOut（此处不需要）。
void VSMain(inout MaterialVSOut output)
{
}

// 像素材质钩子：采样纹理取漫反射色，并设置镜面色与高光指数。框架的 _PSMain 会读取这些
// 值去做 环境光 + Phong 方向光 照明。
void PSMain(inout MaterialPSOut output)
{
    // 用 DefaultSampler 在全局 TextureCoordinate（当前像素插值后的 UV）处采样 Diffuse 贴图，
    // 得到这一像素的漫反射颜色（含 alpha）。
    float4 diffuse = Diffuse.Sample(DefaultSampler, TextureCoordinate);
    // 漫反射色=采样颜色，忽略 alpha 并强制 1（不透明），交给框架做光照。
    output.diffuse = float4(diffuse.rgb, 1);
    // 镜面色=纯白 * specularStrength（来自 b3 常量缓冲），高光整体亮度由 specularStrength 控制。
    output.specular = float4(1, 1, 1, 1) * specularStrength;
    // 高光指数 64，控制高光亮斑的大小与锐利程度（越大越集中锐利）。
    output.shininess = 64.0f;
}
