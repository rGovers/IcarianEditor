// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#include "Windows/ProfilerWindow.h"

#include <implot.h>

#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "ProfilerData.h"

static std::vector<uint32_t> GetChildren(uint32_t a_index, const ProfileSnapshot& a_snapshot)
{
    const uint32_t index = a_snapshot.StartIndex * a_snapshot.FrameCount;
    const ProfileFrame& frame = a_snapshot.Frames[index + a_index];

    std::vector<uint32_t> children;
    for (uint32_t i = 0; i < a_snapshot.FrameCount; ++i)
    {
        if (i == a_index)
        {
            continue;
        }

        const ProfileFrame& sFrame = a_snapshot.Frames[index + i];

        const uint32_t parent = ProfilerData::GetFrameParent(sFrame.NameID);
        if (frame.NameID != parent)
        {
            continue;
        }

        children.emplace_back(i);
    }

    return children;
}

ProfilerWindow::ProfilerWindow() : Window("Profiler", "Textures/WindowIcons/WindowIcon_Profiler.png")
{

}
ProfilerWindow::~ProfilerWindow()
{

}

uint32_t ProfilerWindow::GetFrameIndex(const ProfileSnapshot& a_snapshot) const
{
    for (const ProfileFrameSelection& s : m_selections)
    {
        if (a_snapshot.Name != s.SnapshotName)
        {
            continue;
        }

        for (uint32_t i = 0; i < a_snapshot.FrameCount; ++i)
        {
            const ProfileFrame& frame = a_snapshot.Frames[(a_snapshot.StartIndex * a_snapshot.FrameCount) + i];
            if (frame.NameID != s.FrameID)
            {
                continue;
            }

            return i;
        }

        break;
    }

    return 0;
}
void ProfilerWindow::SetFrameID(const char* a_snapshotName, uint32_t a_frameID)
{
    for (ProfileFrameSelection& selection : m_selections)
    {
        if (selection.SnapshotName != a_snapshotName)
        {
            continue;
        }

        selection.FrameID = a_frameID;

        return;
    }

    const ProfileFrameSelection selection =
    {
        .SnapshotName = std::string(a_snapshotName),
        .FrameID = a_frameID,
    };
    m_selections.emplace_back(selection);
}

void ProfilerWindow::DisplayCPU()
{
    if (!ImGui::CollapsingHeader("CPU Time"))
    {
        return;
    }

    constexpr uint32_t TimeOffset = offsetof(ProfileFrame, Time);

    const std::vector<ProfileSnapshot> snapshots = ProfilerData::GetSnapshots();
    for (const ProfileSnapshot& s : snapshots)
    {
        if (s.Count <= 0)
        {
            continue;
        }

        const uint32_t index = GetFrameIndex(s);
        const std::vector<uint32_t> childIndices = GetChildren(index, s);

        const std::string windowName = "ProfilerWindow[" + s.Name + "]";
        ImGui::PushID(windowName.c_str());
        IDEFER(ImGui::PopID());

        const ProfileFrame& frame = s.Frames[s.StartIndex * s.FrameCount + index];

        {
            const bool show = ImGui::BeginChild("##Selector", ImVec2(128.0f, 300.0f));
            IDEFER(ImGui::EndChild());

            if (show)
            {
                const std::string idStr = std::to_string(frame.NameID);

                if (index != 0)
                {
                    const std::string backName = "Back[" + idStr + "]";
                    ImGui::PushID(backName.c_str());
                    IDEFER(ImGui::PopID());

                    if (ImGui::Button("<"))
                    {
                        const uint32_t parentID = ProfilerData::GetFrameParent(frame.NameID);

                        SetFrameID(s.Name.c_str(), parentID);
                    }
                }

                for (uint32_t cIndex : childIndices)
                {
                    const std::string cName = "Child[" + std::to_string(cIndex) + "][" + idStr + "]";
                    ImGui::PushID(cName.c_str());
                    IDEFER(ImGui::PopID());

                    const ProfileFrame& cFrame = s.Frames[s.StartIndex * s.FrameCount + cIndex];
                    const std::string name = ProfilerData::GetFrameName(cFrame.NameID);
                    if (ImGui::Button(name.c_str()))
                    {
                        SetFrameID(s.Name.c_str(), cFrame.NameID);
                    }
                }
            }
        }

        ImGui::SameLine();

        const std::string frameName = ProfilerData::GetFrameName(frame.NameID);

        const std::string plotName = ILAMBDA(
        {
            // Adding the units as we do not always use the same units for everything
            if (index != 0)
            {
                ILRETURN s.Name + "[" + frameName + "](s)";
            }

            ILRETURN s.Name + "(s)";
        });

        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot(plotName.c_str()))
        {
            IDEFER(ImPlot::EndPlot());

            const uint32_t stride = (uint32_t)s.FrameCount * sizeof(ProfileFrame);

            const uint8_t* timeDataPtr = (uint8_t*)s.Frames + (index * sizeof(ProfileFrame)) + TimeOffset;
            ImPlot::PlotBars
            (
                frameName.c_str(),
                (float*)timeDataPtr,
                (int)s.Count,
                1.0f,
                0.0f,
                0,
                (int)s.StartIndex,
                (int)stride
            );

            for (uint32_t cIndex : childIndices)
            {
                const ProfileFrame& cFrame = s.Frames[s.StartIndex * s.FrameCount + cIndex];

                const uint8_t* cTimeDataPtr = (uint8_t*)s.Frames + (cIndex * sizeof(ProfileFrame)) + TimeOffset;

                const std::string childName = ProfilerData::GetFrameName(cFrame.NameID);
                ImPlot::PlotLine
                (
                    childName.c_str(),
                    (float*)cTimeDataPtr,
                    (int)s.Count,
                    1.0f,
                    0.0f,
                    0,
                    (int)s.StartIndex,
                    (int)stride
                );
            }
        }
    }
}

void ProfilerWindow::DisplayGPU()
{
    if (!ImGui::CollapsingHeader("GPU Time"))
    {
        return;
    }

    ImGui::Indent();
    IDEFER(ImGui::Unindent());

    const uint32_t eteCount = ProfilerData::GetGPUEndToEndCount();

    if (eteCount > 0)
    {
        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot("End to End(s)"))
        {
            IDEFER(ImPlot::EndPlot());

            const uint32_t startIndex = ProfilerData::GetGPUEndToEndStartIndex();
            const float* times = ProfilerData::GetGPUEndToEndTimes();

            ImPlot::PlotLine("Time", times, (int)eteCount, 1.0f, 0.0f, 0, startIndex, sizeof(float));
        }

        ImGui::Separator();
    }

    constexpr uint32_t TimeOffset = offsetof(ProfileFrame, Time);

    const std::vector<ProfileSnapshot> snapshots = ProfilerData::GetGPUSnapshots();
    for (const ProfileSnapshot& s : snapshots)
    {
        if (s.Count <= 0)
        {
            continue;
        }

        const std::string plotName = s.Name + "(ms)";

        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot(plotName.c_str()))
        {
            IDEFER(ImPlot::EndPlot());

            const uint32_t stride = (uint32_t)s.FrameCount * sizeof(ProfileFrame);

            for (uint32_t i = 0; i < s.FrameCount; ++i)
            {
                const ProfileFrame& frame = s.Frames[s.StartIndex * s.FrameCount + i];

                const uint8_t* timeDataPtr = (uint8_t*)s.Frames + (i * sizeof(ProfileFrame)) + TimeOffset;

                const std::string itemName = ProfilerData::GetGPUItemName(frame.NameID);
                ImPlot::PlotLine
                (
                    itemName.c_str(),
                    (float*)timeDataPtr,
                    (int)s.Count,
                    1.0f,
                    0.0f,
                    0,
                    (int)s.StartIndex,
                    (int)stride
                );
            }
        }
    }
}

static void ShowRAMFrames
(
    const IcarianCore::MemoryUsageFrame* a_frames,
    uint32_t a_startIndex,
    uint32_t a_count,
    uint32_t a_offset,
    const char* a_name
)
{
    float data[ProfileMaxScopes];
    for (uint32_t i = 0; i < a_count; ++i)
    {
        const uint32_t index = (a_startIndex + i) % ProfileMaxScopes;

        const IcarianCore::MemoryUsageFrame* frame = a_frames + index;
        const uint64_t* d = (uint64_t*)((uint8_t*)frame + a_offset);

        data[i] = (float)(*d / (double)(1 << 20));
    }

    ImPlot::PlotLine(a_name, data, a_count);
}

void ProfilerWindow::DisplayRAM()
{
    if (!ImGui::CollapsingHeader("RAM Usage"))
    {
        return;
    }

    ImGui::Indent();
    IDEFER(ImGui::Unindent());

    const uint32_t totalMemoryCount = ProfilerData::GetTotalMemoryCount();
    if (totalMemoryCount > 0)
    {
        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot("Total RAM Usage(GiB)"))
        {
            IDEFER(ImPlot::EndPlot());

            const uint32_t startIndex = ProfilerData::GetTotalMemoryStartIndex();

            const uint64_t* osMemoryData = ProfilerData::GetOSMemoryData();
            const uint64_t* mallocMemoryData = ProfilerData::GetMallocMemoryData();

            float data[ProfileMaxScopes];
            for (uint32_t i = 0; i < totalMemoryCount; ++i)
            {
                const uint32_t index = (startIndex + i) % ProfileMaxScopes;

                const uint64_t osData = osMemoryData[index];
                const uint64_t mallocData = mallocMemoryData[index];

                const uint64_t totalSize = osData + mallocData;

                data[i] = (float)(totalSize / (double)(1 << 30));
            }

            ImPlot::PlotBars("Total RAM Usage", data, totalMemoryCount, 1.0f);

            for (uint32_t i = 0; i < totalMemoryCount; ++i)
            {
                const uint32_t index = (startIndex + i) % ProfileMaxScopes;

                const uint64_t osData = osMemoryData[index];

                data[i] = (float)(osData / (double)(1 << 30));
            }

            ImPlot::PlotLine("OS RAM Usage", data, totalMemoryCount);

            for (uint32_t i = 0; i < totalMemoryCount; ++i)
            {
                const uint32_t index = (startIndex + i) % ProfileMaxScopes;

                const uint64_t mallocData = mallocMemoryData[index];

                data[i] = (float)(mallocData / (double)(1 << 30));
            }

            ImPlot::PlotLine("Malloc RAM Usage", data, totalMemoryCount);
        }
    }

    const uint32_t memoryFrameCount = ProfilerData::GetMemoryFrameCount();
    if (memoryFrameCount > 0)
    {
        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot("System RAM Usage(MiB)"))
        {
            IDEFER(ImPlot::EndPlot());

            const uint32_t startIndex = ProfilerData::GetMemoryFrameStartIndex();

            const IcarianCore::MemoryUsageFrame* frames = ProfilerData::GetMemoryUsageFrames();

            ShowRAMFrames
            (
                frames,
                startIndex,
                memoryFrameCount,
                (uint32_t)offsetof(IcarianCore::MemoryUsageFrame, CSharpUsage),
                "C# Native RAM Usage"
            );

            ShowRAMFrames
            (
                frames,
                startIndex,
                memoryFrameCount,
                (uint32_t)offsetof(IcarianCore::MemoryUsageFrame, AudioUsage),
                "Audio RAM Usage"
            );

            ShowRAMFrames
            (
                frames,
                startIndex,
                memoryFrameCount,
                (uint32_t)offsetof(IcarianCore::MemoryUsageFrame, RenderingUsage),
                "Rendering RAM Usage"
            );

            ShowRAMFrames
            (
                frames,
                startIndex,
                memoryFrameCount,
                (uint32_t)offsetof(IcarianCore::MemoryUsageFrame, PhysicsUsage),
                "Physics RAM Usage"
            );

            ShowRAMFrames
            (
                frames,
                startIndex,
                memoryFrameCount,
                (uint32_t)offsetof(IcarianCore::MemoryUsageFrame, FileCacheUsage),
                "FileCache RAM Usage"
            );
        }
    }
}

void ProfilerWindow::DisplayUpdate(double a_delta)
{
    DisplayCPU();
    DisplayGPU();
    DisplayRAM();
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
