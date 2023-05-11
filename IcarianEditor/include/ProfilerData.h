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