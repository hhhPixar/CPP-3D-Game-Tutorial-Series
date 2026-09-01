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
// =============================================================================
// 所属子系统：全引擎汇总头（All-in-one header）
// 文件职责：一次性 include 引擎所有公开头文件，方便外部使用方一行搞定。
//
// 引擎按子系统组织，每个子系统一个目录，各自有核心头文件：
//   Core       核心基础设施（Base/Logger/类型/宏/类型 ID）
//   Resource   资源系统（资源管理器 + 材质/纹理/网格资源）
//   Component  组件（立方体/相机/网格/变换/方向光组件）
//   Game       游戏层（GameObject/World/Component/Game 游戏循环）
//   Input      输入系统（键盘鼠标）
//   Math       数学库（Vec2/Vec3/Rect 等）
//
// 使用方式：外部只需 #include <DX3D/All.h> 即可获得全部公开类型。
// 注意：汇总头会增大编译单元体积，大型项目通常按需 include 单个头文件；
//       此处为教学方便而提供，便于初学者快速上手。
// =============================================================================

#pragma once


// ---- Resource 资源系统子系统 ----
// 资源系统负责按文件路径加载并缓存复用资源（材质/纹理/网格）。
#include <DX3D/Game/Component.h>

#include <DX3D/Resource/TextureResource.h>
#include <DX3D/Resource/MaterialResource.h>
#include <DX3D/Resource/MeshResource.h>
#include <DX3D/Resource/ResourceManager.h>


// ---- Component 组件子系统 ----
// 各类具体组件：实体挂载的功能单元（变换、网格、相机、立方体、方向光）。
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>
#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/DirectionalLightComponent.h>



// ---- Game 游戏层子系统 ----
// GameObject 是实体，World 容纳实体并管理更新循环。
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/World.h>



// ---- Input 输入子系统 & Game 游戏主循环 ----
// InputSystem 提供键鼠状态查询；Game 是引擎主入口与帧循环驱动。
#include <DX3D/Input/InputSystem.h>
#include <DX3D/Game/Game.h>


