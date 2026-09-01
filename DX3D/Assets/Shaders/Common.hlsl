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
// 所属子系统：HLSL 着色器（Shader）—— 公共头文件 Common.hlsl
// ----------------------------------------------------------------------------
// 本文件是“材质着色器框架”的公共部分，被 BasicShader.hlsl / MaterialShader.hlsl
// 通过 #include 引入。框架提供两个真正被编译进 GPU 管线的入口函数：
//   _VSMain —— 顶点着色器（Vertex Shader，编译目标 vs_5_0）：把顶点从“模型空间”
//              依次变换到 世界空间 -> 观察空间 -> 裁剪空间，并计算用于光照的世界法线。
//   _PSMain —— 像素着色器（Pixel Shader，编译目标 ps_5_0）：合成最终像素颜色 =
//              环境光(Ambient) + Phong 方向光(漫反射 Diffuse + 镜面 Specular)。
// 各材质只需重写两个“钩子”函数 VSMain / PSMain 来提供材质属性（颜色、纹理、高光等），
// 其余坐标变换与光照计算都由本框架统一完成。
// 关键概念：
//   cbuffer（常量缓冲）——对应 C++ 端 ConstantBuffer，按寄存器 b0/b1/b2/b3 绑定；
//   register(b/t/s) —— b=常量缓冲、t=纹理(ShaderResourceView)、s=采样器(Sampler)；
//   语义(SV_POSITION/POSITION0/TEXCOORD0/NORMAL0/SV_TARGET)——标注数据在管线中的用途；
//   row_major —— 矩阵按“行主序”存储，与 C++ 端 Mat4x4 内存布局一致，使
//                mul(行向量, 矩阵) 与数学上“向量乘矩阵”含义一致。
// ============================================================================

// 顶点着色器“输入”结构：描述每个顶点从顶点缓冲（Vertex Buffer）读入的数据。
// 字段的语义必须与 C++ 端 InputLayout 定义一一对应，GPU 据此把缓冲字节映射到字段。
struct VSInput
{
    // POSITION 语义：顶点在“模型空间（Model Space，又称局部空间）”中的坐标，
    //                 即建模时的原始位置，尚未经过任何世界/观察/投影变换。
    float3 position : POSITION0;
    // TEXCOORD 语义：纹理坐标（UV），一般范围 0~1，指明该顶点对应贴图的哪一点。
    //                 光栅器会在三角形内部对其插值，逐像素传给像素着色器采样纹理。
    float2 texcoord : TEXCOORD0;
    // NORMAL 语义：顶点法线，模型空间下的单位向量、垂直于表面，用于光照计算。
    //               注意：法线表示“方向”而非“位置”，变换方式与坐标不同（见 _VSMain）。
    float3 normal : NORMAL0;
};

// 顶点着色器“输出”结构：算出的数据交给光栅器（Rasterizer）。光栅器把三角形拆成像素，
// 并对这些字段在三角形内部做线性插值，于是像素着色器拿到的是逐像素插值后的值。字段含义：
//   position : SV_POSITION    —— 裁剪空间齐次坐标（必选语义），GPU 用它做透视除法与视口
//                                 映射决定屏幕像素；像素着色器读它得到的是屏幕坐标，含义不同，
//                                 故本框架另用 TEXCOORD1 传世界坐标。
//   texcoord : TEXCOORD0      —— 顶点 UV 原样透传，供像素着色器采样贴图。
//   worldPosition : TEXCOORD1 —— 世界空间坐标，用于算视线方向 V。（TEXCOORD1 被借用传自定义数据）
//   worldNormal : TEXCOORD2   —— 世界空间法线，用于漫反射 N·L 与镜面 R·V。（同上，借用）
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;    
    float3 worldPosition : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

// 摄像机数据。C++ 端每帧把当前摄像机的观察/投影矩阵与位置写入对应常量缓冲。
struct CameraData
{
    // view：观察矩阵（View Matrix），把“世界空间”变换到“观察/相机空间”
    //       （把场景平移旋转成“以相机为原点、相机朝 -Z”的坐标系）。
    row_major float4x4 view;
    // proj：投影矩阵（Projection Matrix），把观察空间变换到“裁剪空间”，
    //       同时做透视收缩（越远越小）；透视除法后即归一化设备坐标 NDC。
    row_major float4x4 proj;
    // position：摄像机在世界空间的位置。Phong 光照里用它算视线方向 V = camera - 被照点。
    float3 position;
};

// 方向光（Directional Light）数据，如太阳：所有光线互相平行、只有一个方向、无位置。
struct DirectionalLightData
{
    // color：光的颜色（RGB），如阳光偏黄、月光偏蓝。
    float3 color;
    // direction：光的“传播方向”（从光源指向被照表面）。注意着色器里算入射方向 L 时
    //            会取负值（L = -direction），因为 L 要从被照点指向光源。
    //            （C++ 端取自方向光物体刚体世界矩阵的第 2 行，即其本地 -Z 轴在世界中的朝向。）
    float3 direction;
    // intensity：光的强度（标量），最终颜色 = (漫反射+镜面) * color * intensity。
    float intensity;
};


// 采样器（Sampler）：告诉 GPU 如何在纹理上取色——如何过滤(双线性等)、如何处理超出 0~1 的坐标。
// register(s0) 表示绑定到采样器槽 s0；C++ 端 WorldRenderer 创建一个 Sampler 并在
// VSSetSamplers/PSSetSamplers 的第 0 槽设置它，与此处的 s0 对应。
sampler DefaultSampler : register(s0);

// 物体数据常量缓冲，寄存器 b0。C++ 端 WorldRenderer 每画一个物体前把它的两套世界矩阵
// 写入此缓冲；它在 C++ 端是第 0 个 ConstantBuffer（cbs[0]=&objectCb）。注意有两套矩阵，
// 分别用于“位置”与“法线”的变换，原因见字段注释。
cbuffer ObjectData : register(b0)
{
    // affineWorld：仿射世界矩阵（Affine World Matrix）——含 缩放+旋转+平移 的完整变换。
    //              用于把“顶点位置”从模型空间变换到世界空间：mul(顶点, affineWorld)。
    row_major float4x4 affineWorld;
    // rigidWorld：刚体世界矩阵（Rigid World Matrix）——只含旋转+平移，不含缩放。
    //             专门用于变换“法线”：取它的 3x3 部分（去掉平移行，因法线是方向、平移无意义），
    //             避免非等比缩放把法线扭曲（非等比缩放会破坏法线与表面的垂直关系，所以法线不能用 affineWorld）。
    row_major float4x4 rigidWorld;
};

// 摄像机常量缓冲，寄存器 b1。C++ 端是第 1 个 ConstantBuffer（cbs[1]=&cameraCb）。
// 里面装一个 CameraData 结构体（view/proj/position）。
cbuffer CameraData : register(b1)
{
    // cameraData：当前摄像机数据，顶点着色器做 view/proj 变换、像素着色器算视线 V 时都要读它。
    CameraData cameraData;
};

// 环境常量缓冲，寄存器 b2。C++ 端是第 2 个 ConstantBuffer（cbs[2]=&envCb）。
// 目前只装方向光数据（后续可扩展更多光源/环境信息）。
cbuffer EnvironmentData : register(b2)
{    
    // directionLightData：方向光数据（见上方 DirectionalLightData 结构）。
    //                     C++ 端把当前方向光的颜色/方向/强度写入此处。
    DirectionalLightData directionLightData;
};


// Phong 方向光计算函数：返回该像素在此方向光下的颜色贡献。
// 参数含义：
//   camera/worldPosition/worldNormal —— 当前像素的摄像机、世界坐标、世界法线；
//   kd, id —— 漫反射系数 kd（表面多大程度漫反射）与漫反射颜色 id（通常取自纹理）；
//   ks, is —— 镜面系数 ks 与镜面颜色 is（高光颜色，常为白）；
//   shininess —— 高光指数（越大高光越集中越锐利）。
// 数学：环境光在外部另算；本函数只算 漫反射(diffuse) + 镜面(specular)，最后整体乘 light.color*intensity。
float3 ComputePhongDirectionalLight(
	CameraData camera,
	DirectionalLightData light,
	float3 worldPosition,
	float3 worldNormal,

	float kd,
	float3 id,

	float ks,
	float3 is,

	float shininess
)
{
    // 下面先算光照所需的几个“单位向量”与“夹角余弦”。normalize：化为长度 1 的单位向量。
    // N：归一化的世界法线（表面朝向）。
    float3 N = normalize(worldNormal);
    // L：归一化的“入射方向反方向”——从被照点指向光源。light.direction 是光传播方向，
    //    取负即得“从表面指向光源”，这正是漫反射公式 N·L 里 L 的定义。
    float3 L = normalize(-light.direction.xyz);
    // V：归一化的“视线方向”——从被照点指向摄像机眼睛，用于镜面反射判断。
    float3 V = normalize(camera.position.xyz - worldPosition);
    // R：光线关于法线的“反射方向”。reflect(I, N) 返回入射方向 I 关于法线 N 的反射；
    //    此处 I = -L（即光的传播方向），反射后得到高光指向，与 V 越接近高光越强。
    float3 R = reflect(-L, N);
    // NdotL：法线与入射反方向的夹角余弦，按 Lambert 余弦定律决定漫反射强度。
    //        saturate 把结果夹取到 [0,1]：背光面(dot<0)给 0，不会出现负亮度。
    float NdotL = saturate(dot(N, L));
    // RdotV：反射方向与视线方向的夹角余弦，决定高光亮斑大小。同样 saturate 夹到 [0,1]。
    float RdotV = saturate(dot(R, V));

    float3 diffuse = kd * id * NdotL;   
    
    // 漫反射分量 diffuse = kd * id * NdotL（上方）：材质颜色 id 越艳、NdotL 越大（越正对光）越亮。
    // 镜面分量：先算高光衰减 spec = pow(RdotV, shininess)——RdotV 越接近 1、shininess 越大
    //          高光越集中锐利；再乘 ks 与镜面颜色 is 得 specular（下一行）。
    // 合成 result（再下一行）：(漫反射 + 镜面) * light.color * light.intensity。
    float spec = pow(RdotV, shininess);
    float3 specular = ks * spec * is;

    float3 result = (diffuse + specular) * light.color.rgb * light.intensity;
    return result;
}

// 材质顶点输出钩子：留给材质着色器在 VSMain 里填充的“额外顶点数据”。当前为空——
// 若某材质需要在顶点阶段输出自定义插值数据，可在此结构里加字段。
struct MaterialVSOut
{
};

// 材质像素输出钩子：材质着色器在 PSMain 里填充，告诉光照框架本材质的属性。字段：
//   diffuse    —— 材质的漫反射颜色（基础色/反照率 albedo），通常取自纹理采样。
//   specular   —— 材质的镜面反射颜色（高光颜色），常为白色，强度由其大小控制。
//   shininess  —— 高光指数，控制高光亮斑的大小与锐利程度。
struct MaterialPSOut
{
    float4 diffuse;    
    float4 specular;
    float shininess;
};

// 两个“钩子”函数的前置声明：材质着色器（BasicShader/MaterialShader）会提供定义。
// 框架在 _VSMain / _PSMain 里调用它们，让每个材质自定义自己的顶点/像素行为。
void VSMain(inout MaterialVSOut output);
void PSMain(inout MaterialPSOut output);

// 全局纹理坐标“桥梁”变量：顶点着色器把当前顶点的 UV 写进它，材质的 PSMain 再读取它。
// 这样即便 PSMain 的参数里没有 UV，材质着色器仍能拿到当前像素（插值后）的纹理坐标，
// 从而调用 Diffuse.Sample(DefaultSampler, TextureCoordinate) 采样贴图。
static float2 TextureCoordinate = float2(0, 0);

// 真正的顶点着色器入口（C++ 端以 "_VSMain" 为入口名编译成 vs_5_0）。
// 完成模型空间 -> 世界空间 -> 观察空间 -> 裁剪空间 的变换链。
VSOutput _VSMain(VSInput input)
{
    VSOutput output;
    // 第一步：把模型空间位置（补齐 w=1 成齐次坐标）乘 affineWorld，得到世界空间坐标。
    //         mul(float4, matrix)：行向量乘矩阵，因 row_major 而与 C++ 端约定一致。
    //         随后还会：暂存世界坐标(worldPosition)、用 rigidWorld 3x3 变换法线(worldNormal)、
    //         再依次乘 view（世界->观察）与 proj（观察->裁剪），最后透传 texcoord、
    //         把 UV 存入全局 TextureCoordinate、调用材质钩子 VSMain。
    output.position = mul(float4(input.position, 1), affineWorld);
    output.worldPosition = output.position.xyz;    
    // 变换法线到世界空间：取 rigidWorld 的 3x3 部分（去掉平移行，因法线是方向、平移无意义），
    // 再 normalize 保证单位长度。这就是“法线只用刚体矩阵变换”的原因——避免缩放扭曲法线。
    output.worldNormal = normalize(mul(input.normal, (float3x3) rigidWorld));

    // 第二步：世界空间 -> 观察空间（应用摄像机 view 矩阵）。
    output.position = mul(output.position, cameraData.view);
    // 第三步：观察空间 -> 裁剪空间（应用投影 proj 矩阵），得到 SV_POSITION 供光栅器使用。
    output.position = mul(output.position, cameraData.proj);
    // 纹理坐标原样透传（output.texcoord），由光栅器插值后供像素着色器使用。
    output.texcoord = input.texcoord;

    TextureCoordinate = input.texcoord;  
    // 把当前顶点 UV 存入全局 TextureCoordinate（见上一行），供材质钩子读取；
    // 然后调用材质钩子 VSMain，让材质着色器有机会填充 MaterialVSOut（当前框架下多为空）。
    MaterialVSOut vsOut;
    VSMain(vsOut);
    
    return output;
}

// 真正的像素着色器入口（C++ 端以 "_PSMain" 为入口名编译成 ps_5_0）。
// 返回 float4，语义 SV_TARGET 表示写入到渲染目标（Render Target，即最终画面）。
float4 _PSMain(VSOutput input) : SV_TARGET
{
    
    TextureCoordinate = input.texcoord; 
    // 光栅器已把顶点 UV 插值成“当前像素的 UV”，上方一行把它存回全局 TextureCoordinate，
    // 供材质 PSMain 采样纹理。下面准备材质输出 psOut 并先给默认值，再交给材质钩子覆盖。
    MaterialPSOut psOut;
    psOut.diffuse = float4(1, 1, 1, 1);    
    psOut.specular = float4(0,0,0,0);
    psOut.shininess = 0.0; 
    PSMain(psOut);

    
    // 最终颜色 = 环境光 + 方向光，逐项累加到 result。
    float3 result = float3(0, 0, 0);

	//ambient light
    // 环境光（Ambient）：模拟四处散射的间接光，让背光面不是纯黑。
    //   ka=0.1 —— 环境光系数（环境光占总亮度的比例，较小）；
    //   ia     —— 环境光颜色，这里用背景/天空色 (0.27,0.39,0.55) 再乘材质漫反射色，
    //             使环境光带上材质自身色调（白色材质保留天蓝色环境光，彩色材质偏自身色）。
    //   注意：本框架里环境光颜色与 C++ 端 clearAndSetBackBuffer 的清屏色一致。
    float ka = 0.1;
    float3 ia = float3(0.27f, 0.39f, 0.55f) * psOut.diffuse.rgb;
    float3 ambientLight = ka * ia;    
    result = ambientLight;

	//directional light
    // 方向光：调用上面的 Phong 函数，传入 摄像机、方向光、像素世界坐标与世界法线，
    //         以及材质的漫反射色（作 id，kd=1.）、镜面色（作 is，ks=1.0）、高光指数。
    //         结果叠加到 result 上。
    result += ComputePhongDirectionalLight(
        cameraData, 
        directionLightData,
        input.worldPosition,
        input.worldNormal,
        1.0, psOut.diffuse.rgb,
        1.0, psOut.specular.rgb,
        psOut.shininess
    );   

    // 输出最终像素颜色，alpha=1（不透明）。
    return float4(result, 1);
}