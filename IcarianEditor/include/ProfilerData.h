// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Core/MemoryUsageFrame.h"

static constexpr int ProfileNameMax = 16;
static constexpr int ProfileFrameMax = 64;
static constexpr int ProfileMaxScopes = 1024;

struct ProfileFrame
{
    char Name[ProfileNameMax];
    float Time;
    uint8_t Stack;
};

struct ProfileScope
{
    char Name[ProfileNameMax];
    uint16_t FrameCount;
    ProfileFrame Frames[ProfileFrameMax];
};

struct ProfileSnapshot
{
    uint32_t Index;
    uint32_t StartIndex;
    uint32_t Count;
    std::string Name;
    ProfileScope Scopes[ProfileMaxScopes];
};

class ProfilerData
{
private:
    uint32_t                      m_totalMemoryIndex;
    uint32_t                      m_totalMemoryStartIndex;
    uint32_t                      m_totalMemoryCount;

    uint32_t                      m_memoryFrameIndex;
    uint32_t                      m_memoryFrameStartIndex;
    uint32_t                      m_memoryFrameCount;

    uint64_t                      m_osMemoryUsage[ProfileMaxScopes];
    uint64_t                      m_mallocMemoryUsage[ProfileMaxScopes];

    IcarianCore::MemoryUsageFrame m_frames[ProfileMaxScopes];

    std::vector<ProfileSnapshot>  m_snapshots;
    bool                          m_active;

    ProfilerData();

protected:

public:
    ~ProfilerData();

    static void Init();
    static void Destroy();

    static bool StartSession();
    static void EndSession();

    static void PushTotalMemoryFrame(uint64_t a_osMemoryUsage, uint64_t a_mallocMemoryUsage);
    static uint32_t GetTotalMemoryCount();
    static uint32_t GetTotalMemoryStartIndex();
    static const uint64_t* GetOSMemoryData();
    static const uint64_t* GetMallocMemoryData();

    static void PushMemoryFrame(const IcarianCore::MemoryUsageFrame& a_frame);
    static uint32_t GetMemoryFrameCount();
    static uint32_t GetMemoryFrameStartIndex();
    static const IcarianCore::MemoryUsageFrame* GetMemoryUsageFrames();

    static void PushData(const ProfileScope& a_scope);
    static std::vector<ProfileSnapshot> GetSnapshots();
};

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
