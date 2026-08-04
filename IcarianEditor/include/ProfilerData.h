// Icarian Editor - Editor for the Icarian Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/MemoryUsageFrame.h"

static constexpr uint32_t ProfileMaxScopes = 512;

struct ProfileFrame
{
    uint32_t NameID;
    float Time;
};

struct ProfileSnapshot
{
    std::string Name;
    ProfileFrame* Frames;
    uint32_t* ID;
    uint32_t Index;
    uint32_t StartIndex;
    uint32_t Count;
    uint16_t FrameCount;
};

class ProfilerData
{
private:
    uint64_t                                  m_osMemoryUsage[ProfileMaxScopes];
    uint64_t                                  m_mallocMemoryUsage[ProfileMaxScopes];

    IcarianCore::MemoryUsageFrame             m_frames[ProfileMaxScopes];

    float                                     m_gpuETETime[ProfileMaxScopes];

    std::unordered_map<uint32_t, std::string> m_gpuPassNames;
    std::unordered_map<uint32_t, std::string> m_gpuItemNames;
    std::unordered_map<uint32_t, std::string> m_scopeNames;
    std::unordered_map<uint32_t, std::string> m_frameNames;
    std::unordered_map<uint32_t, uint32_t>    m_frameParents;

    std::vector<ProfileSnapshot>              m_gpuSnapshots;
    std::vector<ProfileSnapshot>              m_snapshots;

    uint32_t                                  m_totalMemoryIndex;
    uint32_t                                  m_totalMemoryStartIndex;
    uint32_t                                  m_totalMemoryCount;

    uint32_t                                  m_memoryFrameIndex;
    uint32_t                                  m_memoryFrameStartIndex;
    uint32_t                                  m_memoryFrameCount;

    uint32_t                                  m_gpuETEIndex;
    uint32_t                                  m_gpuETEStartIndex;
    uint32_t                                  m_gpuETECount;

    bool                                      m_active;

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

    static void PushNewGPUPass(uint32_t a_id, const char* a_str);
    static void PushNewGPUItem(uint32_t a_passID, uint32_t a_id, const char* a_str);
    static void PushGPUData(const void* a_data, uint32_t a_length);
    static std::vector<ProfileSnapshot> GetGPUSnapshots();
    static std::string GetGPUItemName(uint32_t a_id);

    static void PushGPUEndToEndTime(float a_time);
    static uint32_t GetGPUEndToEndCount();
    static uint32_t GetGPUEndToEndStartIndex();
    static const float* GetGPUEndToEndTimes();

    static void PushNewScope(uint32_t a_id, const char* a_str);
    static void PushNewFrame(uint32_t a_scopeId, uint32_t a_nameId, uint32_t a_parentId, const char* a_str);
    static void PushData(const void* a_data, uint32_t a_length);
    static std::vector<ProfileSnapshot> GetSnapshots();
    static std::string GetFrameName(uint32_t a_nameId);
    static uint32_t GetFrameParent(uint32_t a_frame);
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
