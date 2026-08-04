// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#include "ProfilerData.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <cstring>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"

static ProfilerData* Instance = nullptr;

ProfilerData::ProfilerData()
{
    m_active = false;

    m_totalMemoryIndex = 0;
    m_totalMemoryStartIndex = 0;
    m_totalMemoryCount = 0;

    m_memoryFrameIndex = 0;
    m_memoryFrameStartIndex = 0;
    m_memoryFrameCount = 0;

    m_gpuETEIndex = 0;
    m_gpuETEStartIndex = 0;
    m_gpuETECount = 0;
}
ProfilerData::~ProfilerData()
{
    for (ProfileSnapshot& s : Instance->m_gpuSnapshots)
    {
        delete[] s.Frames;
        delete[] s.ID;
    }

    for (ProfileSnapshot& s : Instance->m_snapshots)
    {
        delete[] s.Frames;
        delete[] s.ID;
    }

    Instance->m_snapshots.clear();
}

void ProfilerData::Init()
{
    if (Instance == nullptr)
    {
        Instance = new ProfilerData();
    }
}
void ProfilerData::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

bool ProfilerData::StartSession()
{
    if (Instance->m_active)
    {
        return false;
    }

    Instance->m_totalMemoryIndex = 0;
    Instance->m_totalMemoryStartIndex = 0;
    Instance->m_totalMemoryCount = 0;

    Instance->m_memoryFrameIndex = 0;
    Instance->m_memoryFrameStartIndex = 0;
    Instance->m_memoryFrameCount = 0;

    Instance->m_gpuETEIndex = 0;
    Instance->m_gpuETEStartIndex = 0;
    Instance->m_gpuETECount = 0;

    for (ProfileSnapshot& s : Instance->m_gpuSnapshots)
    {
        delete[] s.Frames;
        delete[] s.ID;
    }

    Instance->m_gpuSnapshots.clear();

    Instance->m_gpuPassNames.clear();
    Instance->m_gpuItemNames.clear();

    for (ProfileSnapshot& s : Instance->m_snapshots)
    {
        delete[] s.Frames;
        delete[] s.ID;
    }
    Instance->m_snapshots.clear();

    Instance->m_scopeNames.clear();
    Instance->m_frameNames.clear();
    Instance->m_frameParents.clear();

    Instance->m_active = true;

    return true;
}

void ProfilerData::EndSession()
{
    Instance->m_active = false;
}

void ProfilerData::PushTotalMemoryFrame(uint64_t a_osMemoryUsage, uint64_t a_mallocMemoryUsage)
{
    Instance->m_osMemoryUsage[Instance->m_totalMemoryIndex] = a_osMemoryUsage;
    Instance->m_mallocMemoryUsage[Instance->m_totalMemoryIndex] = a_mallocMemoryUsage;

    Instance->m_totalMemoryIndex = (Instance->m_totalMemoryIndex + 1) % ProfileMaxScopes;

    if (Instance->m_totalMemoryCount < ProfileMaxScopes)
    {
        ++Instance->m_totalMemoryCount;
    }
    else
    {
        Instance->m_totalMemoryStartIndex = (Instance->m_totalMemoryStartIndex + 1) % ProfileMaxScopes;
    }
}

uint32_t ProfilerData::GetTotalMemoryCount()
{
    return Instance->m_totalMemoryCount;
}
uint32_t ProfilerData::GetTotalMemoryStartIndex()
{
    return Instance->m_totalMemoryStartIndex;
}
const uint64_t* ProfilerData::GetOSMemoryData()
{
    return Instance->m_osMemoryUsage;
}
const uint64_t* ProfilerData::GetMallocMemoryData()
{
    return Instance->m_mallocMemoryUsage;
}

void ProfilerData::PushMemoryFrame(const IcarianCore::MemoryUsageFrame& a_frame)
{
    Instance->m_frames[Instance->m_memoryFrameIndex] = a_frame;

    Instance->m_memoryFrameIndex = (Instance->m_memoryFrameIndex + 1) % ProfileMaxScopes;

    if (Instance->m_memoryFrameCount < ProfileMaxScopes)
    {
        ++Instance->m_memoryFrameCount;
    }
    else
    {
        Instance->m_memoryFrameStartIndex = (Instance->m_memoryFrameStartIndex + 1) % ProfileMaxScopes;
    }
}
uint32_t ProfilerData::GetMemoryFrameCount()
{
    return Instance->m_memoryFrameCount;
}
uint32_t ProfilerData::GetMemoryFrameStartIndex()
{
    return Instance->m_memoryFrameStartIndex;
}
const IcarianCore::MemoryUsageFrame* ProfilerData::GetMemoryUsageFrames()
{
    return Instance->m_frames;
}

struct ProfileFrameData
{
    uint32_t NameID;
    float Time;
};

struct ProfileScopeHeader
{
    uint32_t Version;
    uint32_t NameID;
    uint32_t FrameCount;
};

static void GrowSnapshot(ProfileSnapshot* a_snapshot, uint32_t a_id)
{
    const uint32_t newFrameCount = a_snapshot->FrameCount + 1;
    IDEFER(a_snapshot->FrameCount = newFrameCount);

    const uint32_t* oldIds = a_snapshot->ID;
    IDEFER(delete[] oldIds);

    a_snapshot->ID = new uint32_t[newFrameCount];
    memcpy(a_snapshot->ID, oldIds, a_snapshot->FrameCount * sizeof(uint32_t));
    a_snapshot->ID[a_snapshot->FrameCount] = a_id;

    const ProfileFrame* oldFrames = a_snapshot->Frames;
    IDEFER(delete[] oldFrames);

    const uint32_t length = newFrameCount * ProfileMaxScopes;

    a_snapshot->Frames = new ProfileFrame[length];
    memset(a_snapshot->Frames, 0, length * sizeof(ProfileFrame));

    for (uint32_t i = 0; i < a_snapshot->Count; ++i)
    {
        const uint32_t index = (a_snapshot->StartIndex + i) % ProfileMaxScopes;

        const uint32_t oldOffset = index * a_snapshot->FrameCount;
        const uint32_t newOffset = index * newFrameCount;

        for (uint32_t j = 0; j < a_snapshot->FrameCount; ++j)
        {
            a_snapshot->Frames[newOffset + j] = oldFrames[oldOffset + j];
        }

        const ProfileFrame frame =
        {
            .NameID = a_id,
            .Time = 0.0f,
        };
        a_snapshot->Frames[newOffset + a_snapshot->FrameCount] = frame;
    }
}
static void PushSnapshotData(ProfileSnapshot* a_snapshot, const ProfileScopeHeader& a_header, const ProfileFrameData* a_data)
{
    ICARIAN_ASSERT(a_snapshot != nullptr);
    ICARIAN_ASSERT(a_data != nullptr);

    // Need to prefill the buffer as if the profile never reaches a code branch it will never get added
    // It is safe to say if the profiler never hit something the time spent is 0
    const uint32_t frameIndex = a_snapshot->Index * a_snapshot->FrameCount;
    for (uint32_t i = 0; i < a_snapshot->FrameCount; ++i)
    {
        const ProfileFrame frame =
        {
            .NameID = a_snapshot->ID[i],
            .Time = 0.0f,
        };

        a_snapshot->Frames[frameIndex + i] = frame;
    }

    for (uint32_t i = 0; i < a_header.FrameCount; ++i)
    {
        const ProfileFrameData& f = a_data[i];

        const uint32_t idIndex = ILAMBDA(
        {
            for (uint32_t j = 0; j < a_snapshot->FrameCount; ++j)
            {
                if (a_snapshot->ID[j] != f.NameID)
                {
                    continue;
                }

                ILRETURN j;
            }

            ILRETURN uint32_t(-1);
        });

        ICARIAN_ASSERT(idIndex != uint32_t(-1));

        a_snapshot->Frames[frameIndex + idIndex].Time = f.Time;
    }

    a_snapshot->Index = (a_snapshot->Index + 1) % ProfileMaxScopes;
    if (a_snapshot->Index == a_snapshot->StartIndex)
    {
        a_snapshot->StartIndex = (a_snapshot->StartIndex + 1) % ProfileMaxScopes;
    }

    if (++a_snapshot->Count > ProfileMaxScopes)
    {
        a_snapshot->Count = ProfileMaxScopes;
    }
}

void ProfilerData::PushNewGPUPass(uint32_t a_id, const char* a_str)
{
    const std::string str = std::string(a_str);
    Instance->m_gpuPassNames.emplace(a_id, str);
}
void ProfilerData::PushNewGPUItem(uint32_t a_passID, uint32_t a_id, const char* a_str)
{
    ICARIAN_ASSERT(Instance->m_gpuPassNames.find(a_passID) != Instance->m_gpuPassNames.end());

    const std::string str = std::string(a_str);

    Instance->m_gpuItemNames.emplace(a_id, str);

    const std::string passName = Instance->m_gpuPassNames[a_passID];
    for (ProfileSnapshot& s : Instance->m_gpuSnapshots)
    {
        if (s.Name != passName)
        {
            continue;
        }

        GrowSnapshot(&s, a_id);

        return;
    }

    const ProfileSnapshot snapshot =
    {
        .Name = passName,
        .Frames = new ProfileFrame[1 * ProfileMaxScopes],
        .ID = ILAMBDA(
        {
            uint32_t* val = new uint32_t[1];

            val[0] = a_id;

            ILRETURN val;
        }),
        .Index = 0,
        .StartIndex = 0,
        .Count = 0,
        .FrameCount = 1,
    };
    Instance->m_gpuSnapshots.emplace_back(snapshot);
}
void ProfilerData::PushGPUData(const void* a_data, uint32_t a_length)
{
    ICARIAN_ASSERT(a_length >= sizeof(ProfileScopeHeader));

    const ProfileScopeHeader* header = (ProfileScopeHeader*)a_data;
    if (header->Version != 0)
    {
        return;
    }

    ICARIAN_ASSERT(a_length == sizeof(ProfileScopeHeader) + header->FrameCount * sizeof(ProfileFrame));
    ICARIAN_ASSERT(Instance->m_gpuPassNames.find(header->NameID) != Instance->m_gpuPassNames.end());

    const ProfileFrameData* frames = (ProfileFrameData*)((uint8_t*)a_data + sizeof(ProfileScopeHeader));

    const std::string passName = Instance->m_gpuPassNames[header->NameID];
    for (ProfileSnapshot& s : Instance->m_gpuSnapshots)
    {
        if (s.Name != passName)
        {
            continue;
        }

        PushSnapshotData(&s, *header, frames);

        return;
    }

    ICARIAN_ASSERT(0);
}
std::vector<ProfileSnapshot> ProfilerData::GetGPUSnapshots()
{
    return Instance->m_gpuSnapshots;
}
std::string ProfilerData::GetGPUItemName(uint32_t a_id)
{
    return Instance->m_gpuItemNames[a_id];
}

void ProfilerData::PushGPUEndToEndTime(float a_time)
{
    Instance->m_gpuETETime[Instance->m_gpuETEIndex] = a_time;

    Instance->m_gpuETEIndex = (Instance->m_gpuETEIndex + 1) % ProfileMaxScopes;

    if (Instance->m_gpuETEIndex == Instance->m_gpuETEStartIndex)
    {
        Instance->m_gpuETEStartIndex = (Instance->m_gpuETEStartIndex + 1) % ProfileMaxScopes;
    }

    if (++Instance->m_gpuETECount > ProfileMaxScopes)
    {
        Instance->m_gpuETECount = ProfileMaxScopes;
    }
}
uint32_t ProfilerData::GetGPUEndToEndCount()
{
    return Instance->m_gpuETECount;
}
uint32_t ProfilerData::GetGPUEndToEndStartIndex()
{
    return Instance->m_gpuETEStartIndex;
}
const float* ProfilerData::GetGPUEndToEndTimes()
{
    return Instance->m_gpuETETime;
}

void ProfilerData::PushNewScope(uint32_t a_id, const char* a_str)
{
    const std::string str = std::string(a_str);
    Instance->m_scopeNames.emplace(a_id, str);
}
void ProfilerData::PushNewFrame(uint32_t a_scopeId, uint32_t a_nameId, uint32_t a_parentId, const char* a_str)
{
    ICARIAN_ASSERT(Instance->m_scopeNames.find(a_scopeId) != Instance->m_scopeNames.end());

    const std::string str = std::string(a_str);

    Instance->m_frameParents.emplace(a_nameId, a_parentId);
    Instance->m_frameNames.emplace(a_nameId, str);

    const std::string scopeName = Instance->m_scopeNames[a_scopeId];
    for (ProfileSnapshot& s : Instance->m_snapshots)
    {
        if (s.Name != scopeName)
        {
            continue;
        }

        GrowSnapshot(&s, a_nameId);

        return;
    }

    const ProfileSnapshot snapshot =
    {
        .Name = scopeName,
        .Frames = new ProfileFrame[1 * ProfileMaxScopes],
        .ID = ILAMBDA(
        {
            uint32_t* val = new uint32_t[1];

            val[0] = a_nameId;

            ILRETURN val;
        }),
        .Index = 0,
        .StartIndex = 0,
        .Count = 0,
        .FrameCount = 1,
    };
    Instance->m_snapshots.emplace_back(snapshot);
}

void ProfilerData::PushData(const void* a_data, uint32_t a_length)
{
    ICARIAN_ASSERT(a_length >= sizeof(ProfileScopeHeader));

    const ProfileScopeHeader* header = (ProfileScopeHeader*)a_data;
    if (header->Version != 0)
    {
        return;
    }

    ICARIAN_ASSERT(a_length == sizeof(ProfileScopeHeader) + header->FrameCount * sizeof(ProfileFrame));
    ICARIAN_ASSERT(Instance->m_scopeNames.find(header->NameID) != Instance->m_scopeNames.end());

    const ProfileFrameData* frames = (ProfileFrameData*)((uint8_t*)a_data + sizeof(ProfileScopeHeader));

    const std::string scopeName = Instance->m_scopeNames[header->NameID];
    for (ProfileSnapshot& s : Instance->m_snapshots)
    {
        if (s.Name != scopeName)
        {
            continue;
        }

        PushSnapshotData(&s, *header, frames);

        return;
    }

    ICARIAN_ASSERT(0);
}
std::vector<ProfileSnapshot> ProfilerData::GetSnapshots()
{
    return Instance->m_snapshots;
}
std::string ProfilerData::GetFrameName(uint32_t a_nameId)
{
    return Instance->m_frameNames[a_nameId];
}
uint32_t ProfilerData::GetFrameParent(uint32_t a_frame)
{
    return Instance->m_frameParents[a_frame];
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
