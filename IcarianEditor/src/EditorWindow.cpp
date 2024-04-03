#include "Windows/EditorWindow.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "EditorConfig.h"
#include "Flare/IcarianDefer.h"
#include "FlareImGui.h"
#include "Gizmos.h"
#include "PixelShader.h"
#include "RenderCommand.h"
#include "Runtime/RuntimeManager.h"
#include "ShaderProgram.h"
#include "Shaders.h"
#include "VertexShader.h"
#include "Workspace.h"

uint32_t EditorWindow::RefCount = 0;
ShaderProgram* EditorWindow::GridShader = nullptr;

EditorWindow::EditorWindow(RuntimeManager* a_runtime, Workspace* a_workspace) : Window("Editor", "Textures/WindowIcons/WindowIcon_Editor.png")
{
    if (GridShader == nullptr)
    {
        const VertexShader* v = VertexShader::GenerateShader(GridVertexShader);
        const PixelShader* p = PixelShader::GenerateShader(GridPixelShader);

        GridShader = ShaderProgram::GenerateProgram(v, p);

        delete v;
        delete p;
    }

    ++RefCount;

    m_workspace = a_workspace;
    m_runtime = a_runtime;

    m_width = -1;
    m_height = -1;

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &m_depthTextureHandle);
    glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &m_framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureHandle, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureHandle, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_translation = glm::vec3(0.0f, -1.0f, 10.0f);

    m_rotation = glm::identity<glm::quat>();

    m_moveSpeed = 10.0f;
    m_zoom = 10.0f;

    m_refresh = true;

    m_workspace->AddEditorWindow(this);
}
EditorWindow::~EditorWindow()
{
    m_workspace->RemoveEditorWindow(this);

    if (--RefCount == 0)
    {
        delete GridShader;
        GridShader = nullptr;
    }

    glDeleteTextures(1, &m_textureHandle);
    glDeleteTextures(1, &m_depthTextureHandle);

    glDeleteFramebuffers(1, &m_framebufferHandle);
}

void EditorWindow::Draw()
{
    m_refresh = false;

    ImGuizmo::SetDrawlist();

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);
    glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);
    const ImVec2 wPos = ImGui::GetWindowPos();
    const ImVec2 min = ImGui::GetWindowContentRegionMin();
    ImGuizmo::SetRect(wPos.x + min.x, wPos.y + min.y, m_width, m_height);

    const glm::vec4 backgroundColor = EditorConfig::GetBackgroundColor();

    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    const glm::mat4 proj = glm::perspective(glm::pi<float>() * 0.4f, (float)m_width / m_height, 0.01f, 1000.0f);
    const glm::mat4 invProj = glm::inverse(proj);

    const glm::vec3 zoomAxis = m_rotation * glm::vec3(0.0f, 0.0f, m_zoom);

    const glm::mat4 rotMat = glm::toMat4(m_rotation);
    const glm::mat4 transMat = glm::translate(glm::identity<glm::mat4>(), m_translation);

    const glm::mat4 trans = transMat * rotMat;
    const glm::mat4 view = glm::inverse(trans);

    CameraShaderBuffer camBuffer;
    camBuffer.View = view;
    camBuffer.Proj = proj;
    camBuffer.InvView = trans;
    camBuffer.InvProj = invProj;
    camBuffer.ViewProj = proj * view;

    RenderCommand::PushCameraBuffer(camBuffer);
    Gizmos::SetMatrices(view, proj);

    m_runtime->ExecFunction("IcarianEditor.Windows", "EditorWindow", ":OnGUI()", nullptr);

    const GLuint handle = GridShader->GetHandle();

    glUseProgram(handle);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat*)&view);
    glUniformMatrix4fv(1, 1, GL_FALSE, (GLfloat*)&proj);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_BLEND);

    Gizmos::Render();
}

void EditorWindow::Update(double a_delta)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImGuiIO& io = ImGui::GetIO();

    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const ImVec2 sizeIm = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    if (sizeIm.x != m_width || sizeIm.y != m_height)
    {
        m_width = (uint32_t)sizeIm.x;
        m_height = (uint32_t)sizeIm.y;

        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

        m_refresh = true;
    }

    if (m_width <= 0 || m_height <= 0)
    {
        return;
    }

    ImGui::Image((ImTextureID)(uintptr_t)m_textureHandle, sizeIm);

    const ImVec2 halfSize = ImVec2(sizeIm.x * 0.5f, sizeIm.y * 0.5f);
    constexpr glm::vec2 WinSize = glm::vec2(117.0f, 40.0f);
    constexpr glm::vec2 WinHalfSize = WinSize * 0.5f;

    const ImVec2 rectMin = ImVec2(winPos.x + halfSize.x - WinHalfSize.x, winPos.y + 40.0f);
    const ImVec2 rectMax = ImVec2(winPos.x + halfSize.x + WinHalfSize.x, winPos.y + 40.0f + WinSize.y);

    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(30, 30, 30, 150), 2.0f);

    float offset = 5.0f;

    const e_ManipulationMode mode = m_workspace->GetManipulationMode();

    ImGui::SetCursorPos(ImVec2(halfSize.x - WinHalfSize.x + offset, 45.0f));

    if (FlareImGui::ImageButton("Translate", "Textures/Icons/Icon_Translate.png", glm::vec2(25.0f), mode == ManipulationMode_Translate))
    {
        m_workspace->SetManipulationMode(ManipulationMode_Translate);
    }    

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Translate");
    }

    offset += 35.0f;

    ImGui::SetCursorPos(ImVec2(halfSize.x - WinHalfSize.x + offset, 45.0f));

    if (FlareImGui::ImageButton("Rotate", "Textures/Icons/Icon_Rotate.png", glm::vec2(25.0f), mode == ManipulationMode_Rotate))
    {
        m_workspace->SetManipulationMode(ManipulationMode_Rotate);
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Rotate");
    }

    offset += 35.0f;

    ImGui::SetCursorPos(ImVec2(halfSize.x - WinHalfSize.x + offset, 45.0f));

    if (FlareImGui::ImageButton("Scale", "Textures/Icons/Icon_Scale.png", glm::vec2(25.0f), mode == ManipulationMode_Scale))
    {
        m_workspace->SetManipulationMode(ManipulationMode_Scale);
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Scale");
    }

    if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
    {
        const ImVec2 imPos = ImGui::GetMousePos();

        const glm::vec2 mPos = glm::vec2(imPos.x, imPos.y);

        const ImGuiKey cameraModifierKey = EditorConfig::GetKeyBind(KeyBindTarget_CameraModifier);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            glm::vec3 mov = glm::vec3(0.0f);

            if (ImGui::IsKeyDown(ImGuiKey_W))
            {
                mov += m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
            }
            if (ImGui::IsKeyDown(ImGuiKey_S))
            {
                mov += m_rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            }
            if (ImGui::IsKeyDown(ImGuiKey_A))
            {
                mov += m_rotation * glm::vec3(-1.0f, 0.0f, 0.0f);
            }
            if (ImGui::IsKeyDown(ImGuiKey_D))
            {
                mov += m_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
            }

            const ImGuiKey moveUpKey = EditorConfig::GetKeyBind(KeyBindTarget_MoveUp);
            const ImGuiKey moveDownKey = EditorConfig::GetKeyBind(KeyBindTarget_MoveDown);
            
            if (ImGui::IsKeyDown(moveUpKey))
            {
                mov += m_rotation * glm::vec3(0.0f, -1.0f, 0.0f);
            }
            if (ImGui::IsKeyDown(moveDownKey))
            {
                mov += m_rotation * glm::vec3(0.0f, 1.0f, 0.0f);
            }

            float modifier = 1.0f;
            if (ImGui::IsKeyDown(cameraModifierKey))
            {
                modifier = 0.1f;
            }

            m_moveSpeed = glm::max(0.1f, m_moveSpeed + io.MouseWheel * 2.0f * modifier);

            const glm::vec2 mMov = m_prevMousePos - mPos;

            const float editorMouseSensitivity = EditorConfig::GetEditorMouseSensitivity();

            m_rotation = glm::angleAxis(mMov.x * editorMouseSensitivity, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(-mMov.y * editorMouseSensitivity, m_rotation * glm::vec3(1.0f, 0.0f, 0.0f)) * m_rotation;

            m_translation += mov * m_moveSpeed * (float)a_delta;
        }
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            const glm::vec2 mMov = m_prevMousePos - mPos;

            if (ImGui::IsKeyDown(cameraModifierKey))
            {
                constexpr float Sensitivity = 0.01f;

                const glm::vec3 up = m_rotation * glm::vec3(0.0f, 1.0f, 0.0f);
                const glm::vec3 right = m_rotation * glm::vec3(1.0f, 0.0f, 0.0f);

                m_translation += up * mMov.y * m_zoom * Sensitivity;
                m_translation += right * mMov.x * m_zoom * Sensitivity;
            }
            else
            {   
                const float editorMouseSensitivity = EditorConfig::GetEditorMouseSensitivity();

                const glm::quat rot = glm::angleAxis(mMov.x * editorMouseSensitivity, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(-mMov.y * editorMouseSensitivity, m_rotation * glm::vec3(1.0f, 0.0f, 0.0f));
                const glm::quat invRot = glm::inverse(rot);

                const glm::vec3 forward = m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);

                const glm::vec3 pos = m_translation - forward * m_zoom;

                m_translation = pos + invRot * forward * m_zoom;
                m_rotation = rot * m_rotation;
            }
        }
        else
        {
            const float startZoom = m_zoom;
            m_zoom = glm::clamp(m_zoom + io.MouseWheel * 0.1f, 0.1f, 100.0f);

            const float zoomDelta = m_zoom - startZoom;

            const glm::vec3 forward = m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);

            m_translation += forward * zoomDelta;

            const ImGuiKey translateKey = EditorConfig::GetKeyBind(KeyBindTarget_Translate);
            const ImGuiKey rotateKey = EditorConfig::GetKeyBind(KeyBindTarget_Rotate);
            const ImGuiKey scaleKey = EditorConfig::GetKeyBind(KeyBindTarget_Scale);

            if (ImGui::IsKeyPressed(translateKey))
            {
                m_workspace->SetManipulationMode(ManipulationMode_Translate);
            }
            if (ImGui::IsKeyPressed(rotateKey))
            {
                m_workspace->SetManipulationMode(ManipulationMode_Rotate);
            }
            if (ImGui::IsKeyPressed(scaleKey))
            {
                m_workspace->SetManipulationMode(ManipulationMode_Scale);
            }
        }

        m_prevMousePos = mPos;

        Draw();
    }   
    else if (m_refresh)
    {
        Draw();
    }
}