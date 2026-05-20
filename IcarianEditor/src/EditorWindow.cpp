// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "Windows/EditorWindow.h"

#include <charconv>
#include <imgui.h>
#include <ImGuizmo.h>
#include <thread>
#include <queue>

#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "Core/LoggerHeader.h"
#include "Core/PipeMessage.h"
#include "Core/TotalMemoryUsageFrame.h"
#include "EditorConfig.h"
#include "EngineProcess.h"
#include "FlareImGui.h"
#include "Gizmos.h"
#include "Logger.h"
#include "PixelShader.h"
#include "RenderCommand.h"
#include "Runtime/RuntimeManager.h"
#include "RuntimeAssetStore.h"
#include "ShaderProgram.h"
#include "Shaders.h"
#include "VertexShader.h"
#include "Workspace.h"

EditorWindow::EditorWindow() : Window("Editor", "Textures/WindowIcons/WindowIcon_Editor.png", true)
{
    m_process = nullptr;

    m_lightMode = EditorLightMode_Ambient;
    m_manipulationMode = ManipulationMode_Translate;

    m_translation = glm::vec3(0.0f, -1.0f, 10.0f);
    m_rotation = glm::identity<glm::quat>();

    m_moveSpeed = 10.0f;
    m_zoom = 10.0f;

    m_width = 640;
    m_height = 480;

    m_lastUpdate = 0;

    glGenTextures(1, &m_gizmosRenderTexture);
    glBindTexture(GL_TEXTURE_2D, m_gizmosRenderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &m_renderTexture);
    glBindTexture(GL_TEXTURE_2D, m_renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &m_gizmosFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gizmosFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gizmosRenderTexture, 0);

    glGenFramebuffers(1, &m_renderFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTexture, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    VertexShader* vShader = VertexShader::GenerateShader(CompositeVertexShader);
    IDEFER(delete vShader);

    PixelShader* pShader = PixelShader::GenerateShader(CompositePixelShader);
    IDEFER(delete pShader);

    m_compositeProgram = ShaderProgram::GenerateProgram(vShader, pShader);

    const std::filesystem::path path = std::filesystem::current_path();
    const uint32_t threadCount = glm::min((uint32_t)std::thread::hardware_concurrency() / 4, uint32_t(4));
    m_process = EngineProcess::CreateProcess(path, m_width, m_height, threadCount);

    RuntimeAssetStore::RegisterEngineProcess(m_process);
}
EditorWindow::~EditorWindow()
{
    if (m_process != nullptr)
    {
        delete m_process;
        m_process = nullptr;
    }

    glDeleteFramebuffers(1, &m_gizmosFramebuffer);
    glDeleteFramebuffers(1, &m_renderFramebuffer);

    glDeleteTextures(1, &m_gizmosRenderTexture);
    glDeleteTextures(1, &m_renderTexture);

    delete m_compositeProgram;
}

void EditorWindow::UpdateProcess(double a_delta)
{
    if (m_process == nullptr)
    {
        return;
    }

    if (!m_process->IsAlive())
    {
        return;
    }

    std::queue<IcarianCore::PipeMessage> messages;
    m_process->Update(a_delta, &messages);

    while (!messages.empty())
    {
        const IcarianCore::PipeMessage msg = messages.front();
        IDEFER(
        {
            if (msg.Data != nullptr)
            {
                delete[] msg.Data;
            }
        });
        messages.pop();

        switch (msg.Type)
        {
        case IcarianCore::PipeMessageType_SetCursorState:
        case IcarianCore::PipeMessageType_ProfileScope:
        case IcarianCore::PipeMessageType_MemoryFrame:
        {
            // Ignore as we are the editor window

            break;
        }
        case IcarianCore::PipeMessageType_Message:
        {
                const IcarianCore::LoggerHeader& header = *(IcarianCore::LoggerHeader*)msg.Data;

                if (header.Version != 0)
                {
                    Logger::Error("Engine Logger message header version mix match");

                    break;
                }

                if (header.MessageOffset + header.MessageSize >= msg.Length)
                {
                    Logger::Error("Engine Logger message length out of bounds");

                    break;
                }
                if (header.StackTraceOffset + header.StackTraceSize >= msg.Length)
                {
                    Logger::Error("Engine Logger message stacktrace out of bounds");

                    break;
                }

                const LoggerMessageData data = 
                {
                    .Message = "[Editor Window] " + std::string((char*)msg.Data + header.MessageOffset, header.MessageSize),
                    .Stacktrace = ILAMBDA(
                    {
                        if (header.StackTraceSize != 0)
                        {
                            std::vector<std::string> vals;

                            const char* stacktraceStart = (char*)msg.Data + header.StackTraceOffset;
                            const char* stacktraceSlider = stacktraceStart;
                            const char* stacktraceMessageBegin = stacktraceStart;
                            while (stacktraceSlider - stacktraceStart < header.StackTraceSize)
                            {
                                if (*stacktraceSlider == 0)
                                {
                                    vals.emplace_back(std::string(stacktraceMessageBegin, stacktraceSlider - stacktraceMessageBegin));

                                    stacktraceMessageBegin = stacktraceSlider + 1;
                                }

                                ++stacktraceSlider;
                            }

                            ILRETURN vals;
                        }

                        ILRETURN std::vector<std::string>();
                    }),
                    .IsEditor = false,
                    .Print = false,
                };

                switch (header.Type)
                {
                case IcarianCore::LoggerMessageType_Message:
                {
#ifdef DEBUG
                    Logger::Message(data);
#endif

                    break;
                }
                case IcarianCore::LoggerMessageType_Warning:
                {
                    Logger::Warning(data);

                    break;
                }
                case IcarianCore::LoggerMessageType_Error:
                {
                    Logger::Error(data);

                    break;
                }
                }

            break;
        }
        case IcarianCore::PipeMessageType_TotalMemoryUsage:
        {
            const IcarianCore::TotalMemoryUsageFrame* frame = (IcarianCore::TotalMemoryUsageFrame*)msg.Data;

            m_memoryUsage = frame->MallocUsage + frame->OSUsage;

            break;
        }
        case IcarianCore::PipeMessageType_RuntimeMessage:
        {
            // TODO: Implement me!~

            break;
        }
        default:
        {
            Logger::Error("Editor: Invalid Pipe Message: " + std::to_string(msg.Type) + " " + std::to_string(msg.Length));

            break;
        }
        }
    }
}
void EditorWindow::BuildFrame()
{
    const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    while (true)
    {
        const e_EngineFrameWaitStatus imageWait = m_process->WaitImage();
        switch (imageWait)
        {
        case EngineFrameWaitStatus_Success:
        {
            break;
        }
        case EngineFrameWaitStatus_Reset:
        case EngineFrameWaitStatus_NotSignaled:
        {
            return;
        }
        case EngineFrameWaitStatus_Wait:
        {
            UpdateProcess(0.0);

            const std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

            const bool timeout = now - startTime >= std::chrono::duration(std::chrono::milliseconds(100));
            if (timeout)
            {
                Logger::Warning("Editor window draw timeout");

                m_process->AdvanceImage();

                return;
            }

            // Yield in case it is a resource contention issue
            std::this_thread::yield();

            continue;
        }
        case EngineFrameWaitStatus_InvalidMode:
        {
            return;
        }
        }

        break;
    }

    // We want to composite the engines frame with the editor gizmos
    const GLuint engineTexture = m_process->GetImage();

    glBindFramebuffer(GL_FRAMEBUFFER, m_renderFramebuffer);

    glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);
    glScissor(0, 0, (GLsizei)m_width, (GLsizei)m_height);

    const glm::vec4 backgroundColor = EditorConfig::GetBackgroundColor();
    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const GLuint handle = m_compositeProgram->GetHandle();
    glUseProgram(handle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, engineTexture);
    glUniform1i(0, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gizmosRenderTexture);
    glUniform1i(1, 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void EditorWindow::TransformToolbar()
{
    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const glm::vec2 size = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    const glm::vec2 halfSize = size * 0.5f;
    constexpr glm::vec2 WinSize = glm::vec2(117.0f, 40.0f);
    constexpr glm::vec2 WinHalfSize = WinSize * 0.5f;

    const ImVec2 rectMin = ImVec2(winPos.x + halfSize.x - WinHalfSize.x, winPos.y + TrayOffset);
    const ImVec2 rectMax = ImVec2(winPos.x + halfSize.x + WinHalfSize.x, winPos.y + TrayOffset + WinSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(30, 30, 30, 150), 2.0f);

    float offset = 5.0f;

    ImGui::SetCursorPos(ImVec2(halfSize.x - WinHalfSize.x + offset, TrayOffset + 5.0f));

    const bool showTranslateBackground = m_manipulationMode == ManipulationMode_Translate;
    if (FlareImGui::ImageButton("Translate", "Textures/Icons/Icon_Translate.png", glm::vec2(25.0f), showTranslateBackground))
    {
        m_manipulationMode = ManipulationMode_Translate;
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Translate");

        ImGui::Separator();

        const ImGuiKey key = EditorConfig::GetKeyBind(KeyBindTarget_Translate);
        ImGui::Text("KeyBind: %s", ImGui::GetKeyName(key));
    }

    offset += 35.0f;

    ImGui::SetCursorPos(ImVec2(halfSize.x - WinHalfSize.x + offset, TrayOffset + 5.0f));

    const bool showRotateBackground = m_manipulationMode == ManipulationMode_Rotate;
    if (FlareImGui::ImageButton("Rotate", "Textures/Icons/Icon_Rotate.png", glm::vec2(25.0f), showRotateBackground))
    {
        m_manipulationMode = ManipulationMode_Rotate;
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Rotate");

        ImGui::Separator();

        const ImGuiKey key = EditorConfig::GetKeyBind(KeyBindTarget_Rotate);
        ImGui::Text("KeyBind: %s", ImGui::GetKeyName(key));
    }

    offset += 35.0f;

    ImGui::SetCursorPos(ImVec2(halfSize.x - WinHalfSize.x + offset, TrayOffset + 5.0f));

    const bool showScaleBackground = m_manipulationMode == ManipulationMode_Scale;
    if (FlareImGui::ImageButton("Scale", "Textures/Icons/Icon_Scale.png", glm::vec2(25.0f), showScaleBackground))
    {
        m_manipulationMode = ManipulationMode_Scale;
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Scale");

        ImGui::Separator();

        const ImGuiKey key = EditorConfig::GetKeyBind(KeyBindTarget_Scale);
        ImGui::Text("KeyBind: %s", ImGui::GetKeyName(key));
    }
}
void EditorWindow::LightModeToolbar()
{
    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const glm::vec2 size = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    constexpr float BorderOffset = 10.0f;
    constexpr glm::vec2 WinSize = glm::vec2(117.0f, 40.0f);

    const ImVec2 rectMin = ImVec2(winPos.x + size.x - WinSize.x - BorderOffset, winPos.y + TrayOffset);
    const ImVec2 rectMax = ImVec2(winPos.x + size.x - BorderOffset, winPos.y + TrayOffset + WinSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(rectMin, rectMax, IM_COL32(30, 30, 30, 150));

    float offset = 5.0f;

    ImGui::SetCursorPos(ImVec2(size.x - WinSize.x - BorderOffset + offset, TrayOffset + 5.0f));

    const bool showAmbientBackground = m_lightMode == EditorLightMode_Ambient;
    if (FlareImGui::ImageButton("Ambient Light", "Textures/Icons/Icon_Ambient.png", glm::vec2(25.0f), showAmbientBackground))
    {
        m_lightMode = EditorLightMode_Ambient;
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Ambient Light Mode");

        ImGui::Separator();

        const ImGuiKey key = EditorConfig::GetKeyBind(KeyBindTarget_AmbientLightMode);
        ImGui::Text("KeyBind: %s", ImGui::GetKeyName(key));
    }

    offset += 35.0f;

    ImGui::SetCursorPos(ImVec2(size.x - WinSize.x - BorderOffset + offset, TrayOffset + 5.0f));

    const bool showViewportBackground = m_lightMode == EditorLightMode_Viewport;
    if (FlareImGui::ImageButton
        (
            "Viewport Light",
            "Textures/Icons/Icon_Viewport.png",
            glm::vec2(25.0f),
            showViewportBackground
        )
    )
    {
        m_lightMode = EditorLightMode_Viewport;
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Viewport Light Mode");

        ImGui::Separator();

        const ImGuiKey key = EditorConfig::GetKeyBind(KeyBindTarget_ViewportLightMode);
        ImGui::Text("KeyBind: %s", ImGui::GetKeyName(key));
    }

    offset += 35.0f;

    ImGui::SetCursorPos(ImVec2(size.x - WinSize.x - BorderOffset + offset, TrayOffset + 5.0f));

    const bool showSceneBackground = m_lightMode == EditorLightMode_Scene;
    if (FlareImGui::ImageButton("Scene Light", "Textures/Icons/Icon_Scene.png", glm::vec2(25.0f), showSceneBackground))
    {
        m_lightMode = EditorLightMode_Scene;
    }

    if (ImGui::IsItemHovered() && ImGui::BeginTooltip())
    {
        IDEFER(ImGui::EndTooltip());

        ImGui::Text("Scene Light Mode");

        ImGui::Separator();

        const ImGuiKey key = EditorConfig::GetKeyBind(KeyBindTarget_SceneLightMode);
        ImGui::Text("KeyBind: %s", ImGui::GetKeyName(key));
    }
}

void EditorWindow::Refresh()
{
    if (m_process != nullptr)
    {
        delete m_process;
        m_process = nullptr;
    }

    const std::filesystem::path path = std::filesystem::current_path();
    // No need for the engine to have a lot of threads as it just needs to render the scene
    const uint32_t threadCount = glm::min((uint32_t)std::thread::hardware_concurrency() / 4, uint32_t(4));
    m_process = EngineProcess::CreateProcess(path, m_width, m_height, threadCount);

    RuntimeAssetStore::RegisterEngineProcess(m_process);
}

void EditorWindow::InternalUpdate(double a_delta)
{
    if (m_process != nullptr && !m_process->IsAlive())
    {
        Logger::Error("Editor window engine process died");

        delete m_process;
        m_process = nullptr;
    }

    UpdateProcess(a_delta);
}
// TODO: At somepoint I will have to drop ImGuizmo and write my own transform Gizmo
void EditorWindow::DisplayUpdate(double a_delta)
{
    const ImVec2 vMinIm = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMaxIm = ImGui::GetWindowContentRegionMax();
    const ImVec2 sizeIm = { vMaxIm.x - vMinIm.x, vMaxIm.y - vMinIm.y };

    // The window is too small so do not bother updating
    if (sizeIm.x < 4 || sizeIm.y < 4)
    {
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        IDEFER(ImGui::EndMenuBar());

        if (m_process != nullptr)
        {
            const float width = ImGui::GetWindowWidth();

            const double memoryUsageGiB = m_memoryUsage / (double)(1ULL << 30ULL);

            const uint32_t ups = (uint32_t)m_process->GetUPS();

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            std::to_chars(buffer, buffer + (sizeof(buffer) - 1), memoryUsageGiB, std::chars_format::fixed, 2);

            const std::string memoryUsageText = std::string("Engine Memory Usage: ") + buffer + "GiB";
            const std::string upsText = "UPS: " + std::to_string(ups);

            const ImVec2 memoryUsageSize = ImGui::CalcTextSize(memoryUsageText.c_str());
            const ImVec2 upsSize = ImGui::CalcTextSize(upsText.c_str());

            const float upsOffset = upsSize.x + 20.0f;
            const float memoryUsageOffset = upsOffset + memoryUsageSize.x + 20.0f;

            ImGui::SetCursorPosX(width - upsOffset);
            ImGui::Text("%s", upsText.c_str());

            ImGui::SetCursorPosX(width - memoryUsageOffset);
            ImGui::Text("%s", memoryUsageText.c_str());
        }
    }

    const uint32_t unfocusedFPS = EditorConfig::GetEditorUnfocusedFPS();
    const float unfocusedFrameTime = 1.0f / unfocusedFPS;

    const bool isFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
    const bool shouldUpdate = isFocused || m_lastUpdate >= unfocusedFrameTime;

    if (m_process != nullptr)
    {
        if (m_width != sizeIm.x || m_height != sizeIm.y)
        {
            const uint32_t newWidth = (uint32_t)sizeIm.x;
            IDEFER(m_width = newWidth);
            const uint32_t newHeight = (uint32_t)sizeIm.y;
            IDEFER(m_height = newHeight);

            m_process->SetSize(newWidth, newHeight);

            glBindTexture(GL_TEXTURE_2D, m_gizmosRenderTexture);
            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                (GLsizei)newWidth,
                (GLsizei)newHeight,
                0, GL_RGBA,
                GL_UNSIGNED_BYTE,
                NULL
            );

            glBindTexture(GL_TEXTURE_2D, m_renderTexture);
            glTexImage2D
            (
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                (GLsizei)newWidth,
                (GLsizei)newHeight,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                NULL
            );
        }

        if (shouldUpdate)
        {
            BuildFrame();
        }

        ImGui::Image((ImTextureID)(uintptr_t)m_renderTexture, sizeIm);
    }

    TransformToolbar();
    LightModeToolbar();

    if (isFocused)
    {
        const ImVec2 winPos = ImGui::GetWindowPos();

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(winPos.x + vMinIm.x, winPos.y + vMinIm.y, m_width, m_height);

        const ImVec2 imPos = ImGui::GetMousePos();
        const glm::vec2 mPos = glm::vec2(imPos.x, imPos.y);
        IDEFER(m_prevMousePos = mPos);

        const ImGuiIO& io = ImGui::GetIO();

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

            const float modifier = ILAMBDA(
            {
                if (ImGui::IsKeyDown(cameraModifierKey))
                {
                    ILRETURN 0.1f;
                }

                ILRETURN 1.0f;
            });

            m_moveSpeed = glm::max(0.1f, m_moveSpeed + io.MouseWheel * 2.0f * modifier);

            const glm::vec2 mMov = m_prevMousePos - mPos;

            const float editorMouseSensitivity = EditorConfig::GetEditorMouseSensitivity();

            const glm::quat horAxis = glm::angleAxis(mMov.x * editorMouseSensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
            const glm::quat verAxis = glm::angleAxis(-mMov.y * editorMouseSensitivity, m_rotation * glm::vec3(1.0f, 0.0f, 0.0f));

            m_rotation = horAxis * verAxis * m_rotation;

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

                const glm::vec3 rightAxis = m_rotation * glm::vec3(1.0f, 0.0f, 0.0f);

                const glm::quat horRot = glm::angleAxis(mMov.x * editorMouseSensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
                const glm::quat verRot = glm::angleAxis(-mMov.y * editorMouseSensitivity, rightAxis);

                const glm::quat rot = horRot * verRot;
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
            if (ImGui::IsKeyPressed(translateKey))
            {
                m_manipulationMode = ManipulationMode_Translate;
            }

            const ImGuiKey rotateKey = EditorConfig::GetKeyBind(KeyBindTarget_Rotate);
            if (ImGui::IsKeyPressed(rotateKey))
            {
                m_manipulationMode = ManipulationMode_Rotate;
            }

            const ImGuiKey scaleKey = EditorConfig::GetKeyBind(KeyBindTarget_Scale);
            if (ImGui::IsKeyPressed(scaleKey))
            {
                m_manipulationMode = ManipulationMode_Scale;
            }

            const ImGuiKey ambientLightKey = EditorConfig::GetKeyBind(KeyBindTarget_AmbientLightMode);
            if (ImGui::IsKeyPressed(ambientLightKey))
            {
                m_lightMode = EditorLightMode_Ambient;
            }

            const ImGuiKey viewportLightKey = EditorConfig::GetKeyBind(KeyBindTarget_ViewportLightMode);
            if (ImGui::IsKeyPressed(viewportLightKey))
            {
                m_lightMode = EditorLightMode_Viewport;
            }

            const ImGuiKey sceneLightKey = EditorConfig::GetKeyBind(KeyBindTarget_SceneLightMode);
            if (ImGui::IsKeyPressed(sceneLightKey))
            {
                m_lightMode = EditorLightMode_Scene;
            }
        }

        if (m_process != nullptr)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_F9))
            {
                m_process->CaptureFrame();
            }
        }
    }

    if (shouldUpdate)
    {
        IDEFER(m_lastUpdate = 0);

        if (m_process != nullptr)
        {
            // Want to set the workspace manipulation mode to the current manipulation mode of the editor window as each can have their own
            Workspace::SetManipulationMode(m_manipulationMode);

            constexpr float FOV = glm::pi<float>() * 0.4f;
            glm::mat4 proj = glm::perspective(FOV, (float)sizeIm.x / sizeIm.y, 0.01f, 1000.0f);

            const glm::mat4 rotMat = glm::toMat4(m_rotation);
            const glm::mat4 transMat = glm::translate(glm::identity<glm::mat4>(), m_translation);

            const glm::mat4 trans = transMat * rotMat;
            glm::mat4 view = glm::inverse(trans);

            Gizmos::SetMatrices(view, proj);

            m_process->SendRuntimeMessage("Editor:CameraTransform", &trans, sizeof(glm::mat4));
            m_process->SendRuntimeMessage("Editor:SceneView:LightMode", &m_lightMode, sizeof(e_EditorLightMode));

            void* args[] =
            {
                &view,
                &proj,
                &m_width,
                &m_height
            };

            RuntimeManager::ExecFunction("IcarianEditor.Windows", "EditorWindow", ":OnGUI(Matrix4,Matrix4,uint,uint)", args);

            RenderCommand::Flush(m_process);

            glBindFramebuffer(GL_FRAMEBUFFER, m_gizmosFramebuffer);

            glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);
            glScissor(0, 0, (GLsizei)m_width, (GLsizei)m_height);

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Gizmos::Render();

            m_process->SignalImage();
        }
    }
    else
    {
        m_lastUpdate += a_delta;
    }
}

// MIT License
// 
// Copyright (c) 2026 River Govers
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
