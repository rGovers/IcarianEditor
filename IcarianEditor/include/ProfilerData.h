// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

static constexpr int ProfileNameMax = 16;
static constexpr int ProfileFrameMax = 64;
static constexpr int ProfileMaxScopes = 2048;

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
    static ProfilerData* Instance;

    std::vector<ProfileSnapshot> m_snapshots;

    ProfilerData();
protected:

public:
    ~ProfilerData();

    static void Init();
    static void Destroy();

    static void Clear();

    static void PushData(const ProfileScope& a_scope);
    static std::vector<ProfileSnapshot> GetSnapshots();
};

// MIT License
// 
// Copyright (c) 2024 River Govers
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