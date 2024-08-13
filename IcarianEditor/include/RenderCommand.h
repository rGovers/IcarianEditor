// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

class RuntimeManager;
class RuntimeStorage;
class Shader;
class ShaderProgram;
class ShaderStorageObject;
class UniformBuffer;

#include "Core/ShaderBuffers.h"

struct RBoneData
{
    std::string Name;
    uint32_t Parent;
    glm::mat4 InvBind;
    glm::mat4 Transform;
};
struct SkeletonData
{
    std::vector<RBoneData> Bones;
};

class RenderCommand
{
private:
    RuntimeStorage*                              m_storage;

    uint32_t                                     m_boundShader;
    std::unordered_map<uint32_t, ShaderProgram*> m_shaders; 

    UniformBuffer*                               m_cameraBuffer;
    UniformBuffer*                               m_transformBuffer;
    ShaderStorageObject*                         m_transformBatchBuffer;
    ShaderStorageObject*                         m_skeletonBuffer;

    std::vector<SkeletonData>                    m_skeletonData;

    RenderCommand(RuntimeStorage* a_storage);
    
    static void BindBuffers(const Shader* a_shader);

protected:

public:
    ~RenderCommand();

    static void Init(RuntimeManager* a_runtime, RuntimeStorage* a_storage);
    static void Clear();
    static void Destroy();

    static void BindMaterial(uint32_t a_materialAddr);
    static void DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr);

    static void PushCameraBuffer(const IcarianCore::ShaderCameraBuffer& a_buffer);

    static uint32_t GenerateSkeletonBuffer();
    static void PushBoneData(uint32_t a_addr, const std::string_view& a_object, uint32_t a_parent, const glm::mat4& a_bindPose, const glm::mat4& a_invBindPose);
    static void SetBoneTransform(uint32_t a_addr, const std::string_view& a_object, const glm::mat4& a_transform);
    static void BindSkeletonBuffer(uint32_t a_addr);

    static void DrawBones(uint32_t a_addr, const glm::mat4& a_transform);
};

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.