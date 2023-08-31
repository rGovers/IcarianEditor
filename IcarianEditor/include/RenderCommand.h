#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

class RuntimeManager;
class RuntimeStorage;
class ShaderProgram;
class ShaderStorageObject;
class UniformBuffer;

#include "ShaderBuffers.h"

struct BoneData
{
    uint32_t Parent;
    glm::mat4 InvBind;
    glm::mat4 Transform;
};
struct SkeletonData
{
    std::vector<BoneData> Bones;
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

    std::vector<SkeletonData>                    m_skeletonData;

    RenderCommand(RuntimeStorage* a_storage);
    
protected:

public:
    ~RenderCommand();

    static void Init(RuntimeManager* a_runtime, RuntimeStorage* a_storage);
    static void Destroy();

    static void BindMaterial(uint32_t a_materialAddr);
    static void DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr);

    static void PushCameraBuffer(const CameraShaderBuffer& a_buffer);
};