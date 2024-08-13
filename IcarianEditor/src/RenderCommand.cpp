// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "RenderCommand.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Gizmos.h"
#include "Logger.h"
#include "Model.h"
#include "PixelShader.h"
#include "Runtime/RuntimeManager.h"
#include "Runtime/RuntimeStorage.h"
#include "ShaderProgram.h"
#include "ShaderStorage.h"
#include "ShaderStorageObject.h"
#include "UniformBuffer.h"
#include "VertexShader.h"

#include "EngineMaterialInteropStructures.h"

static RenderCommand* Instance = nullptr;

#define RENDERCOMMAND_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define RENDERCOMMAND_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { RenderCommand::BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering, RenderCommand, DrawModel, { RenderCommand::DrawModel(a_transform, a_addr); }, glm::mat4 a_transform, uint32_t a_addr) \
    \
    F(uint32_t, IcarianEditor, AnimationMaster, GenerateSkeletonBuffer, { return RenderCommand::GenerateSkeletonBuffer(); }) \
    F(void, IcarianEditor, AnimationMaster, BindSkeletonBuffer, { RenderCommand::BindSkeletonBuffer(a_addr); }, uint32_t a_addr) \
    \
    \
    F(void, IcarianEngine.Rendering.Animation, Animator, DestroyBuffer, { }, uint32_t a_addr) \

RENDERCOMMAND_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(void, SkeletonAnimator, PushTransform,
{
    char* str = mono_string_to_utf8(a_object);
    IDEFER(mono_free(str));

    glm::mat4 transform;
    float* f = (float*)&transform;

    for (int i = 0; i < 16; ++i)   
    {
        f[i] = mono_array_get(a_transform, float, i);
    }

    RenderCommand::SetBoneTransform(a_addr, str, transform);
}, uint32_t a_addr, MonoString* a_object, MonoArray* a_transform)

RUNTIME_FUNCTION(void, AnimationMaster, PushBoneData, 
{
    char* objectName = mono_string_to_utf8(a_object);
    IDEFER(mono_free(objectName));

    glm::mat4 bindPose;
    float* f = (float*)&bindPose;

    for (int i = 0; i < 16; ++i)   
    {
        f[i] = mono_array_get(a_bindPose, float, i);
    }

    glm::mat4 invBindPose;
    f = (float*)&invBindPose;

    for (int i = 0; i < 16; ++i)   
    {
        f[i] = mono_array_get(a_invBindPose, float, i);
    }

    RenderCommand::PushBoneData(a_addr, objectName, a_parent, bindPose, invBindPose);
}, uint32_t a_addr, MonoString* a_object, uint32_t a_parent, MonoArray* a_bindPose, MonoArray* a_invBindPose)
RUNTIME_FUNCTION(void, AnimationMaster, DrawBones, 
{
    glm::mat4 transform;
    float* f = (float*)&transform;

    for (int i = 0; i < 16; ++i)   
    {
        f[i] = mono_array_get(a_transform, float, i);
    }

    RenderCommand::DrawBones(a_addr, transform);
}, uint32_t a_addr, MonoArray* a_transform)

RenderCommand::RenderCommand(RuntimeStorage* a_storage)
{
    m_storage = a_storage;

    m_boundShader = -1;

    IcarianCore::ShaderCameraBuffer cameraBuffer;
    IcarianCore::ShaderModelBuffer modelBuffer;

    m_cameraBuffer = new UniformBuffer(&cameraBuffer, sizeof(cameraBuffer));
    m_transformBuffer = new UniformBuffer(&modelBuffer, sizeof(modelBuffer));
    m_transformBatchBuffer = new ShaderStorageObject();
    m_skeletonBuffer = new ShaderStorageObject();
}
RenderCommand::~RenderCommand()
{
    Clear();

    delete m_cameraBuffer;
    delete m_transformBuffer;
    delete m_transformBatchBuffer;
    delete m_skeletonBuffer;
}

void RenderCommand::Init(RuntimeManager* a_runtime, RuntimeStorage* a_storage)
{
    if (Instance == nullptr)
    {
        Instance = new RenderCommand(a_storage);

        RENDERCOMMAND_BINDING_FUNCTION_TABLE(RENDERCOMMAND_RUNTIME_ATTACH);
        
        BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.Animation, SkeletonAnimator, PushTransform);

        BIND_FUNCTION(a_runtime, IcarianEditor, AnimationMaster, PushBoneData);
        BIND_FUNCTION(a_runtime, IcarianEditor, AnimationMaster, DrawBones);
    }
}
void RenderCommand::Clear()
{
    for (auto iter = Instance->m_shaders.begin(); iter != Instance->m_shaders.end(); ++iter)
    {
        delete iter->second;
    }
    Instance->m_shaders.clear();

    Instance->m_skeletonData.clear();

    Instance->m_boundShader = -1;
}
void RenderCommand::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void RenderCommand::BindBuffers(const Shader* a_shader)
{
    const uint32_t count = a_shader->GetInputCount();
    for (uint32_t i = 0; i < count; ++i)
    {
        const ShaderBufferInput input = a_shader->GetInput(i);

        switch (input.BufferType)
        {
        case ShaderBufferType_CameraBuffer:
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, input.Slot, Instance->m_cameraBuffer->GetHandle());

            break;
        }
        case ShaderBufferType_SSModelBuffer:
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input.Slot, Instance->m_transformBatchBuffer->GetHandle());

            break;
        }
        case ShaderBufferType_PModelBuffer:
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, 64, Instance->m_transformBuffer->GetHandle());

            break;
        }
        default:
        {
            continue;
        }
        }
    }
}
void RenderCommand::BindMaterial(uint32_t a_materialAddr)
{
    const RenderProgram program = Instance->m_storage->GetRenderProgram(a_materialAddr);

    if (program.VertexShader == -1 || program.PixelShader == -1)
    {
        Instance->m_boundShader = -1;

        return;
    }

    VertexShader* vShader = Instance->m_storage->GetVertexShader(program.VertexShader);
    PixelShader* pShader = Instance->m_storage->GetPixelShader(program.PixelShader);

    ShaderProgram* shader = nullptr;
    const auto iter = Instance->m_shaders.find(a_materialAddr);
    if (iter == Instance->m_shaders.end())
    {
        shader = ShaderProgram::GenerateProgram(vShader, pShader);
        if (shader == nullptr)
        {
            return;
        }

        Instance->m_shaders.emplace(a_materialAddr, shader);
    }
    else
    {
        shader = iter->second;
    }

    const GLuint handle = shader->GetHandle();

    glUseProgram(handle);

    switch (program.CullingMode) 
    {
    case CullMode_None:
    {
        glDisable(GL_CULL_FACE);

        break;
    }
    case CullMode_Back:
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        break;
    }
    case CullMode_Front:
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        break;
    }
    case CullMode_Both:
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);

        break;
    }
    }

    BindBuffers(vShader);
    BindBuffers(pShader);

    ShaderStorage* storage = (ShaderStorage*)program.Data;
    if (storage != nullptr)
    {
        storage->Bind();
    }

    Instance->m_boundShader = a_materialAddr;
}

static bool IsBatched(const Shader* a_shader)
{
    const uint32_t inputCount = a_shader->GetInputCount();
    for (uint32_t i = 0; i < inputCount; ++i) 
    {
        const ShaderBufferInput input = a_shader->GetInput(i);

        if (input.BufferType == ShaderBufferType_SSModelBuffer) 
        {
            return true;
        }
    }

    return false;
}
void RenderCommand::DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr)
{
    if (Instance->m_boundShader == -1)
    {
        Logger::Warning("IcarianEditor: DrawModel unbound material");

        return;
    }

    if (a_modelAddr == -1)
    {
        Logger::Warning("IcarianEditor: DrawModel null model");

        return;
    }

    const Model* model = Instance->m_storage->GetModel(a_modelAddr);
    if (model == nullptr)
    {
        Logger::Warning("IcarianEditor: DrawModel invalid model address");

        return;
    }
    const RenderProgram program = Instance->m_storage->GetRenderProgram(Instance->m_boundShader);

    const IcarianCore::ShaderModelBuffer buffer =
    {
        .Model = a_transform,
        .InvModel = glm::inverse(a_transform)
    };

    // TODO: Write to batch buffer if batched
    bool batched = false;

    const VertexShader* vShader = Instance->m_storage->GetVertexShader(program.VertexShader);
    if (vShader != nullptr && IsBatched(vShader))
    {
        batched = true;
    }
    else
    {
        const PixelShader* pShader = Instance->m_storage->GetPixelShader(program.PixelShader);
        if (pShader != nullptr && IsBatched(pShader))
        {
            batched = true;
        }
    }

    if (batched)
    {
        Instance->m_transformBatchBuffer->WriteBuffer(&buffer, sizeof(buffer), 1);   
    }
    else 
    {
        Instance->m_transformBuffer->WriteBuffer(&buffer, sizeof(buffer));
    }

    const GLuint vbo = model->GetVBO();
    const GLuint ibo = model->GetIBO();

    glBindVertexBuffer(0, vbo, 0, (GLsizei)program.VertexStride);
    for (uint16_t i = 0; i < program.VertexInputCount; ++i)
    {
        const VertexInputAttribute& att = program.VertexAttributes[i];
        
        glEnableVertexAttribArray(i);
        switch (att.Type)
        {
        case VertexType_Float:
        {
            glVertexAttribFormat((GLuint)i, (GLint)att.Count, GL_FLOAT, GL_FALSE, (GLuint)att.Offset);

            break;
        }
        case VertexType_Int:
        {
            glVertexAttribIFormat((GLuint)i, (GLint)att.Count, GL_INT, (GLuint)att.Offset);

            break;
        }
        case VertexType_UInt:
        {
            glVertexAttribIFormat((GLuint)i, (GLint)att.Count, GL_UNSIGNED_INT, (GLuint)att.Offset);

            break;
        }
        default:
        {
            ICARIAN_ASSERT_MSG(0, "Invalid vertex type");
        }
        }
        glVertexAttribBinding(i, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glDrawElements(GL_TRIANGLES, (GLsizei)model->GetIndexCount(), GL_UNSIGNED_INT, NULL);
}

void RenderCommand::PushCameraBuffer(const IcarianCore::ShaderCameraBuffer& a_camera)
{
    Instance->m_cameraBuffer->WriteBuffer(&a_camera, sizeof(IcarianCore::ShaderCameraBuffer));
}

uint32_t RenderCommand::GenerateSkeletonBuffer()
{
    SkeletonData data;
    const uint32_t addr = (uint32_t)Instance->m_skeletonData.size();

    Instance->m_skeletonData.push_back(data);

    return addr;
}
void RenderCommand::PushBoneData(uint32_t a_addr, const std::string_view& a_object, uint32_t a_parent, const glm::mat4& a_bindPose, const glm::mat4& a_invBindPose)
{
    ICARIAN_ASSERT(a_addr < Instance->m_skeletonData.size());

    RBoneData data;
    data.Name = std::string(a_object);
    data.Parent = a_parent;
    data.InvBind = a_invBindPose;
    data.Transform = a_bindPose;

    Instance->m_skeletonData[a_addr].Bones.push_back(data);
}
void RenderCommand::SetBoneTransform(uint32_t a_addr, const std::string_view& a_object, const glm::mat4& a_transform)
{
    ICARIAN_ASSERT(a_addr < Instance->m_skeletonData.size());

    for (RBoneData& bone : Instance->m_skeletonData[a_addr].Bones)
    {
        if (bone.Name == a_object)
        {
            bone.Transform = a_transform;

            break;
        }
    }
}

static glm::mat4 GetBoneTransform(const SkeletonData& a_skeleton, uint32_t a_boneAddr)
{
    ICARIAN_ASSERT(a_boneAddr < a_skeleton.Bones.size());

    const RBoneData& bone = a_skeleton.Bones[a_boneAddr];

    const glm::mat4 transform = bone.Transform;
    
    if (bone.Parent != -1)
    {
        const glm::mat4 parent = GetBoneTransform(a_skeleton, bone.Parent);

        return parent * transform;
    }

    return transform;
}

static void BindSkeletonObject(const Shader* a_shader, const ShaderStorageObject* a_skeletonBuffer)
{
    const uint32_t inputCount = a_shader->GetInputCount();
    for (uint32_t i = 0; i < inputCount; ++i) 
    {
        const ShaderBufferInput input = a_shader->GetInput(i);
        if (input.BufferType == ShaderBufferType_SSBoneBuffer) 
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input.Slot, a_skeletonBuffer->GetHandle());

            return;
        }
    }
}
void RenderCommand::BindSkeletonBuffer(uint32_t a_addr)
{
    if (a_addr >= Instance->m_skeletonData.size())
    {
        Logger::Warning("IcarianEditor: BindSkeletonBuffer invalid skeleton address");

        return;
    }

    const SkeletonData& data = Instance->m_skeletonData[a_addr];

    const uint32_t count = (uint32_t)data.Bones.size();

    glm::mat4* transforms = new glm::mat4[count];
    IDEFER(delete[] transforms);

    for (uint32_t i = 0; i < count; ++i)
    {
        transforms[i] = GetBoneTransform(data, i) * data.Bones[i].InvBind;
    }

    Instance->m_skeletonBuffer->WriteBuffer(transforms, sizeof(glm::mat4), count);

    const RenderProgram program = Instance->m_storage->GetRenderProgram(Instance->m_boundShader);

    const VertexShader* vShader = Instance->m_storage->GetVertexShader(program.VertexShader);
    if (vShader != nullptr) 
    {
        BindSkeletonObject(vShader, Instance->m_skeletonBuffer);
    }
    const PixelShader* pShader = Instance->m_storage->GetPixelShader(program.PixelShader);
    if (pShader != nullptr)
    {
        BindSkeletonObject(pShader, Instance->m_skeletonBuffer);
    }
}

void RenderCommand::DrawBones(uint32_t a_addr, const glm::mat4& a_transform)
{
    ICARIAN_ASSERT(a_addr < Instance->m_skeletonData.size());

    const SkeletonData& data = Instance->m_skeletonData[a_addr];

    for (const RBoneData& bone : data.Bones)
    {
        glm::mat4 transform = bone.Transform;
        uint32_t parent = bone.Parent;
        while (parent != -1)
        {
            const RBoneData& parentBone = data.Bones[parent];

            transform = parentBone.Transform * transform;

            parent = parentBone.Parent;
        }

        const glm::mat4 mat = a_transform * transform;

        const glm::vec3 forward = mat[2].xyz();
        const glm::vec3 up = mat[1].xyz();
        const glm::vec3 right = mat[0].xyz();

        const glm::vec3 pos = mat[3].xyz();

        Gizmos::DrawIcoSphere(pos, 0.01f, 0, 0.001f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        
        Gizmos::DrawLine(pos, pos + forward * 0.02f, 0.001f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        Gizmos::DrawLine(pos, pos + up * 0.02f, 0.001f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        Gizmos::DrawLine(pos, pos + right * 0.02f, 0.001f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    }
}

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