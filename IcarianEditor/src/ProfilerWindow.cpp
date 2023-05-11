#include "Windows/ProfilerWindow.h"

#include <implot.h>

#include "ProfilerData.h"

static uint32_t GetParent(uint32_t a_index, const ProfileSnapshot& a_snapshot)
{
    const ProfileFrame& frame = a_snapshot.Scopes[0].Frames[a_index];

    if (frame.Stack > 1)
    {
        const uint32_t parentStack = frame.Stack - 1;

        uint32_t i = a_index;
        while (i > 0)
        {
            --i;

            if (a_snapshot.Scopes[0].Frames[i].Stack == parentStack)
            {
                return i;
            }
        }
    }
    
    return 0;
}
static std::vector<uint32_t> GetChildren(uint32_t a_index, const ProfileSnapshot& a_snapshot)
{
    std::vector<uint32_t> children;

    const ProfileFrame& frame = a_snapshot.Scopes[0].Frames[a_index];
    const uint32_t childFrame = frame.Stack + 1;

    for (uint32_t i = a_index + 1; i < a_snapshot.Scopes[0].FrameCount; ++i)
    {
        const ProfileFrame& sFrame = a_snapshot.Scopes[0].Frames[i];

        if (sFrame.Stack == frame.Stack)
        {
            break;
        }
        else if (sFrame.Stack == childFrame)
        {
            children.emplace_back(i);
        }
    }

    return children;
}

ProfilerWindow::ProfilerWindow() : Window("Profiler")
{
    
}
ProfilerWindow::~ProfilerWindow()
{

}

uint32_t ProfilerWindow::GetFrameIndex(const ProfileSnapshot& a_snapshot) const
{
    for (const ProfileFrameSelection& selection : m_selections)
    {
        if (a_snapshot.Name == selection.SnapshotName)
        {
            for (uint32_t i = 0; i < ProfileFrameMax; ++i)
            {
                if (a_snapshot.Scopes[0].Frames[i].Name == selection.FrameName)
                {
                    return i;
                }
            }

            return 0;
        }
    }

    return 0;
}
void ProfilerWindow::SetFrameIndex(const std::string_view& a_snapshotName, const std::string_view& a_frameName)
{
    for (ProfileFrameSelection& selection : m_selections)
    {
        if (selection.SnapshotName == a_snapshotName)
        {
            selection.FrameName = std::string(a_frameName);

            return;
        }
    }

    ProfileFrameSelection selection;
    selection.SnapshotName = std::string(a_snapshotName);
    selection.FrameName = std::string(a_frameName);

    m_selections.emplace_back(selection);
}

void ProfilerWindow::Update(double a_delta)
{
    const std::vector<ProfileSnapshot> snapshots = ProfilerData::GetSnapshots();

    constexpr int TimeOffset = offsetof(ProfileFrame, Time);
    constexpr int FramesOffset = offsetof(ProfileScope, Frames);

    constexpr int FrameSize = sizeof(ProfileFrame);
    constexpr int ScopeSize = sizeof(ProfileScope);

    for (const ProfileSnapshot& snapshot : snapshots)
    {
        const uint32_t index = GetFrameIndex(snapshot);
        const std::vector<uint32_t> childIndices = GetChildren(index, snapshot);

        const ProfileFrame& frame = snapshot.Scopes[0].Frames[index];

        const uint32_t timeOff = FramesOffset + (index * FrameSize) + TimeOffset;

        ImGui::BeginGroup();

        if (index != 0)
        {
            if (ImGui::Button("<"))
            {
                const uint32_t parentIndex = GetParent(index, snapshot);
                const ProfileFrame& pFrame = snapshot.Scopes[0].Frames[parentIndex];

                SetFrameIndex(snapshot.Name, pFrame.Name);
            }
        }

        for (uint32_t cIndex : childIndices)
        {
            const ProfileFrame& cFrame = snapshot.Scopes[0].Frames[cIndex];
            if (ImGui::Button(cFrame.Name))
            {
                SetFrameIndex(snapshot.Name, cFrame.Name);
            }
        }

        ImGui::EndGroup();
        
        ImGui::SameLine();

        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot(snapshot.Name.c_str()))
        {
            ImPlot::PlotBars(frame.Name, (float*)((char*)&snapshot.Scopes + timeOff), (int)snapshot.Count, 1.0f, 0.0f, 0, snapshot.StartIndex, ScopeSize);

            for (uint32_t cIndex : childIndices)
            {
                const ProfileFrame &cFrame = snapshot.Scopes[0].Frames[cIndex];

                const uint32_t cTimeOff = FramesOffset + (cIndex * FrameSize) + TimeOffset;

                ImPlot::PlotLine(cFrame.Name, (float*)((char*)&snapshot.Scopes + cTimeOff), (int)snapshot.Count, 1.0f, 0.0f, 0, snapshot.StartIndex, ScopeSize);
            }

            ImPlot::EndPlot();
        }
    }
}