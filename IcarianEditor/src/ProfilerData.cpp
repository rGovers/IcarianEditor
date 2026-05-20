// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ProfilerData.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

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
}
ProfilerData::~ProfilerData()
{

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

    Instance->m_snapshots.clear();

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

void ProfilerData::PushData(const ProfileScope& a_scope)
{
    // Realised that dynamic memory was too slow so doing a rotating array
    // Overwrite the old value when data gets too old
    // Dynamic allocation seems to cause it to be unable to keep up with input data and getting stuck writing causing application lockup
    // Not the most memory efficent but seems that allocation is just too expensive
    // Once again sometimes better to write a solution then to use an existing data type
    // Should now hopefully be fast enough for the ~1000 calls a second I need
    for (ProfileSnapshot& snapshot : Instance->m_snapshots)
    {
        if (snapshot.Name == a_scope.Name)
        {
            snapshot.Scopes[snapshot.Index] = a_scope;

            snapshot.Index = (snapshot.Index + 1) % ProfileMaxScopes;
            if (snapshot.Index == snapshot.StartIndex)
            {
                snapshot.StartIndex = (snapshot.StartIndex + 1) % ProfileMaxScopes;
            }

            snapshot.Count = glm::min(snapshot.Count + 1, (uint32_t)ProfileMaxScopes);

            return;
        }
    }

    // Stack is too small on Windows and will crash so have to do everything on heap memory
    ProfileSnapshot& snapshot = Instance->m_snapshots.emplace_back();
    snapshot.Name = a_scope.Name;
    snapshot.Index = 1;
    snapshot.StartIndex = 0;
    snapshot.Count = 1;
    snapshot.Scopes[0] = a_scope;
}
std::vector<ProfileSnapshot> ProfilerData::GetSnapshots()
{
    return Instance->m_snapshots;
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