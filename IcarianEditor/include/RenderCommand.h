#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <unordered_map>

class RuntimeManager;
class RuntimeStorage;
class ShaderProgram;
class UniformBuffer;

#include "ShaderBuffers.h"

class RenderCommand
{
private:
    RuntimeStorage*                              m_storage;

    uint32_t                                     m_boundShader;
    std::unordered_map<uint32_t, ShaderProgram*> m_shaders; 

    UniformBuffer*                               m_cameraBuffer;
    UniformBuffer*                               m_transformBuffer;

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