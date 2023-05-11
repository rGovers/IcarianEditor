#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <string>

class Modal
{
private:
    std::string m_idStr;
    std::string m_displayName;

    glm::vec2   m_size;

protected:

public:
    Modal(const std::string_view& a_displayName, const glm::vec2& a_size = glm::vec2(0.0f));
    virtual ~Modal();

    bool Display();
    virtual bool Update() = 0;
};