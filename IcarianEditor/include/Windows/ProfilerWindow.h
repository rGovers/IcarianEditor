#pragma once

#include "Windows/Window.h"

#include <cstdint>
#include <string>
#include <vector>

struct ProfileSnapshot;

class ProfilerWindow : public Window
{
private:
    struct ProfileFrameSelection
    {
        std::string SnapshotName;
        std::string FrameName;
    };

    std::vector<ProfileFrameSelection> m_selections;

    uint32_t GetFrameIndex(const ProfileSnapshot& a_snapshot) const;
    void SetFrameIndex(const std::string_view& a_snapshotName, const std::string_view& a_frameName);

protected:

public:
    ProfilerWindow();
    ~ProfilerWindow();

    virtual void Update(double a_delta);
};