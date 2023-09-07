#pragma once

class RuntimeManager;

class EditorConfig
{
private:
    static constexpr char ConfigFile[] = "editorConfig.xml";

    bool m_useDegrees = false;

    EditorConfig();

protected:

public:
    ~EditorConfig();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static bool GetUseDegrees();
    static void SetUseDegrees(bool useDegrees);

    static void Deserialize();
    static void Serialize();
};