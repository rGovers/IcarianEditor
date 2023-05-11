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