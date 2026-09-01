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
// 所属子系统：HLSL 着色器 —— 材质着色器 BasicShader.hlsl（最简材质：纯白、无纹理）
// ----------------------------------------------------------------------------
// 通过 #include 引入 Common.hlsl 提供的框架：_VSMain（顶点变换）与 _PSMain（光照合成）
// 是真正被编译的入口；本文件只重写两个材质“钩子” VSMain / PSMain 来提供材质属性。
// 渲染流程位置：顶点变换(框架) -> 像素材质属性(此处 PSMain) -> 光照计算(框架 _PSMain)。
// 本材质：VSMain 留空；PSMain 设 漫反射色=纯白、镜面色=纯白、高光指数=64，
//         不采样任何纹理（物体呈 flat 纯色再受环境光+方向光照射）。
// 关键概念：MaterialVSOut/MaterialPSOut 是与框架约定的钩子结构；寄存器 t0 见下方。
// ============================================================================

// 漫反射纹理（贴图）。register(t0) 表示绑定到纹理槽 t0；C++ 端把材质的 Diffuse 贴图作为
// ShaderResourceView 在第 0 槽设置（VSSetShaderResources/PSSetShaderResources）。
// 注意：本 BasicShader 实际未在 PSMain 采样它（始终用纯白），此处仅声明占位。
Texture2D Diffuse : register(t0);


// 顶点材质钩子：本材质无需自定义顶点处理，函数体留空。框架的 _VSMain 在做完坐标变换
// 后会调用它，让它有机会填充 MaterialVSOut（此处不需要）。
void VSMain(inout MaterialVSOut output)
{
}

// 像素材质钩子：在此设置本材质的属性（漫反射色、镜面色、高光指数），
// 框架的 _PSMain 会读取这些值去做环境光+方向光照明。
void PSMain(inout MaterialPSOut output)
{
    // diffuse：漫反射色=纯白。本材质不采样纹理，物体呈 flat 纯色再受光照。
    output.diffuse = float4(1, 1, 1, 1);
    // specular：镜面色=纯白（默认高光颜色，未乘 specularStrength）。
    output.specular = float4(1, 1, 1, 1);    
    // shininess：高光指数 64，控制高光亮斑的大小与锐利程度（越大越集中锐利）。
    output.shininess = 64.0f;
}