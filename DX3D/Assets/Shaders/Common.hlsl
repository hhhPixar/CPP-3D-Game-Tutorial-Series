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

struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;    
    float3 worldPosition : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

struct CameraData
{
    row_major float4x4 view;
    row_major float4x4 proj;
    float3 position;
};

struct DirectionalLightData
{
    float3 color;
    float3 direction;
    float intensity;
};


sampler DefaultSampler : register(s0);

cbuffer ObjectData : register(b0)
{
    row_major float4x4 affineWorld;
    row_major float4x4 rigidWorld;
};

cbuffer CameraData : register(b1)
{
    CameraData cameraData;
};

cbuffer EnvironmentData : register(b2)
{    
    DirectionalLightData directionLightData;
};


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
    float3 N = normalize(worldNormal);
    float3 L = normalize(-light.direction.xyz);
    float3 V = normalize(camera.position.xyz - worldPosition);
    float3 R = reflect(-L, N);
    float NdotL = saturate(dot(N, L));
    float RdotV = saturate(dot(R, V));

    float3 diffuse = kd * id * NdotL;   
    
    float spec = pow(RdotV, shininess);
    float3 specular = ks * spec * is;

    float3 result = (diffuse + specular) * light.color.rgb * light.intensity;
    return result;
}

struct MaterialVSOut
{
};

struct MaterialPSOut
{
    float4 diffuse;    
    float4 specular;
    float shininess;
};

void VSMain(inout MaterialVSOut output);
void PSMain(inout MaterialPSOut output);

static float2 TextureCoordinate = float2(0, 0);

VSOutput _VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1), affineWorld);
    output.worldPosition = output.position.xyz;    
    output.worldNormal = normalize(mul(input.normal, (float3x3) rigidWorld));

    output.position = mul(output.position, cameraData.view);
    output.position = mul(output.position, cameraData.proj);
    output.texcoord = input.texcoord;

    TextureCoordinate = input.texcoord;  
    MaterialVSOut vsOut;
    VSMain(vsOut);
    
    return output;
}

float4 _PSMain(VSOutput input) : SV_TARGET
{
    
    TextureCoordinate = input.texcoord; 
    MaterialPSOut psOut;
    psOut.diffuse = float4(1, 1, 1, 1);    
    psOut.specular = float4(0,0,0,0);
    psOut.shininess = 0.0; 
    PSMain(psOut);

    
    float3 result = float3(0, 0, 0);

	//ambient light
    float ka = 0.1;
    float3 ia = float3(0.27f, 0.39f, 0.55f) * psOut.diffuse.rgb;
    float3 ambientLight = ka * ia;    
    result = ambientLight;

	//directional light
    result += ComputePhongDirectionalLight(
        cameraData, 
        directionLightData,
        input.worldPosition,
        input.worldNormal,
        1.0, psOut.diffuse.rgb,
        1.0, psOut.specular.rgb,
        psOut.shininess
    );   

    return float4(result, 1);
}