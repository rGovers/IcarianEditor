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