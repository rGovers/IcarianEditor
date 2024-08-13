// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#include "ProfilerData.h"

ProfilerData* ProfilerData::Instance = nullptr;

ProfilerData::ProfilerData()
{

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

void ProfilerData::Clear()
{
    Instance->m_snapshots.clear();
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
            
            snapshot.Count = std::min(snapshot.Count + 1, (uint32_t)ProfileMaxScopes);

            return;
        }
    }

    // Stack is too small on windows and will crash so have to do everything on heap memory
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