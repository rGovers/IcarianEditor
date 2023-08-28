#include "RenderCommand.h"

#include "Flare/IcarianAssert.h"
#include "Flare/RenderProgram.h"
#include "Logger.h"
#include "Model.h"
#include "Runtime/RuntimeManager.h"
#include "Runtime/RuntimeStorage.h"
#include "ShaderProgram.h"
#include "ShaderStorage.h"
#include "ShaderStorageObject.h"
#include "UniformBuffer.h"

static RenderCommand* Instance = nullptr;

#define RENDERCOMMAND_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define RENDERCOMMAND_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine.Rendering, RenderCommand, BindMaterial, { RenderCommand::BindMaterial(a_addr); }, uint32_t a_addr) \

RENDERCOMMAND_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(RenderCommand, DrawModel), MonoArray* a_transform, uint32_t a_modelAddr)
{
    glm::mat4 transform;

    float* f = (float*)&transform;

    for (int i = 0; i < 16; ++i)   
    {
        f[i] = mono_array_get(a_transform, float, i);
    }

    RenderCommand::DrawModel(transform, a_modelAddr);
}

static constexpr GLenum GetFormat(const FlareBase::VertexInputAttrib& a_input)
{
    switch (a_input.Type)
    {
    case FlareBase::VertexType_Float:
    {
        return GL_FLOAT;
    }
    case FlareBase::VertexType_Int:
    {
        return GL_INT;
    }
    case FlareBase::VertexType_UInt:
    {
        return GL_UNSIGNED_INT;
    }
    default:
    {
        ICARIAN_ASSERT_MSG(0, "Invalid vertex type");
    }
    }

    return GL_FLOAT;
}

RenderCommand::RenderCommand(RuntimeStorage* a_storage)
{
    m_storage = a_storage;

    m_boundShader = -1;

    CameraShaderBuffer cameraBuffer;
    ModelShaderBuffer modelBuffer;

    m_cameraBuffer = new UniformBuffer(&cameraBuffer, sizeof(cameraBuffer));
    m_transformBuffer = new UniformBuffer(&modelBuffer, sizeof(modelBuffer));
    m_transformBatchBuffer = new ShaderStorageObject(&modelBuffer, sizeof(modelBuffer));
}
RenderCommand::~RenderCommand()
{
    delete m_cameraBuffer;
    delete m_transformBuffer;
}

void RenderCommand::Init(RuntimeManager* a_runtime, RuntimeStorage* a_storage)
{
    if (Instance == nullptr)
    {
        Instance = new RenderCommand(a_storage);

        RENDERCOMMAND_BINDING_FUNCTION_TABLE(RENDERCOMMAND_RUNTIME_ATTACH);

        BIND_FUNCTION(a_runtime, IcarianEngine.Rendering, RenderCommand, DrawModel);
    }
}
void RenderCommand::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void RenderCommand::BindMaterial(uint32_t a_materialAddr)
{
    const FlareBase::RenderProgram program = Instance->m_storage->GetRenderProgram(a_materialAddr);

    if (program.VertexShader == -1 || program.PixelShader == -1)
    {
        Instance->m_boundShader = -1;

        return;
    }

    ShaderProgram* shader = nullptr;
    const auto iter = Instance->m_shaders.find(a_materialAddr);
    if (iter == Instance->m_shaders.end())
    {
        VertexShader* vShader = Instance->m_storage->GetVertexShader(program.VertexShader);
        PixelShader* pShader = Instance->m_storage->GetPixelShader(program.PixelShader);

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
    case FlareBase::CullMode_None:
    {
        glDisable(GL_CULL_FACE);

        break;
    }
    case FlareBase::CullMode_Back:
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        break;
    }
    case FlareBase::CullMode_Front:
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        break;
    }
    case FlareBase::CullMode_Both:
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);

        break;
    }
    }

    for (uint32_t i = 0; i < program.ShaderBufferInputCount; ++i)
    {
        const FlareBase::ShaderBufferInput& input = program.ShaderBufferInputs[i];

        switch (input.BufferType)
        {
        case FlareBase::ShaderBufferType_CameraBuffer:
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, input.Slot, Instance->m_cameraBuffer->GetHandle());

            break;
        }
        case FlareBase::ShaderBufferType_SSModelBuffer:
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, input.Slot, Instance->m_transformBatchBuffer->GetHandle());

            break;
        }
        case FlareBase::ShaderBufferType_ModelBuffer:
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

    ShaderStorage* storage = (ShaderStorage*)program.Data;
    if (storage != nullptr)
    {
        storage->Bind();
    }

    Instance->m_boundShader = a_materialAddr;
}
void RenderCommand::DrawModel(const glm::mat4& a_transform, uint32_t a_modelAddr)
{
    if (a_modelAddr == -1)
    {
        Logger::Warning("IcarianEditor: DrawModel null model");

        return;
    }

    if (Instance->m_boundShader == -1)
    {
        Logger::Warning("IcarianEditor: DrawModel unbound material");

        return;
    }

    Model* model = Instance->m_storage->GetModel(a_modelAddr);
    if (model == nullptr)
    {
        Logger::Warning("IcarianEditor: DrawModel invalid model address");

        return;
    }
    FlareBase::RenderProgram program = Instance->m_storage->GetRenderProgram(Instance->m_boundShader);

    ModelShaderBuffer buffer;
    buffer.Model = a_transform;
    buffer.InvModel = glm::inverse(a_transform);

    // TODO: Write to batch buffer if batched
    bool batched = false;
    for (uint32_t i = 0; i < program.ShaderBufferInputCount; ++i)
    {
        const FlareBase::ShaderBufferInput& input = program.ShaderBufferInputs[i];

        if (input.BufferType == FlareBase::ShaderBufferType_SSModelBuffer)
        {
            batched = true;

            break;
        }
    }

    if (batched)
    {
        Instance->m_transformBatchBuffer->WriteBuffer(&buffer, sizeof(buffer));   
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
        const FlareBase::VertexInputAttrib& att = program.VertexAttribs[i];
        
        glEnableVertexAttribArray(i);
        glVertexAttribFormat((GLuint)i, (GLint)att.Count, GetFormat(att), GL_FALSE, (GLuint)att.Offset);
        glVertexAttribBinding(i, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glDrawElements(GL_TRIANGLES, (GLsizei)model->GetIndexCount(), GL_UNSIGNED_INT, NULL);
}

void RenderCommand::PushCameraBuffer(const CameraShaderBuffer& a_camera)
{
    Instance->m_cameraBuffer->WriteBuffer(&a_camera, sizeof(CameraShaderBuffer));
}