#include "Runtime/RuntimeManager.h"

#include <assert.h>
#include <chrono>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <string>

#include "ConsoleCommand.h"
#include "Logger.h"
#include "MonoProjectGenerator.h"

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Logger, PushMessage), MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Message(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Logger, PushWarning), MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Warning(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Logger, PushError), MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Error(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Application, GetEditorState))
{
    return 1;
}

#define ATTACH_FUNCTION(namespace, klass, function) mono_add_internal_call(RUNTIME_FUNCTION_STRING(namespace, klass, function), (void*)RUNTIME_FUNCTION_NAME(klass, function));

static std::vector<std::string> SplitString(const std::string_view& a_string)
{
    std::vector<std::string> v;

    const char* s = a_string.data();
    const char* l = s;
    while (*s != 0)
    {
        if (*s == '\n')
        {
            const std::string str = std::string(l, s - l);

            if (!str.empty())
            {
                v.emplace_back(str);
            }

            ++s;

            while (*s == ' ')
            {
                ++s;
            }

            l = s;

            continue;
        }

        ++s;
    }

    const std::string str = std::string(l, s - l);

    if (!str.empty())
    {
        v.emplace_back(str);
    }

    return v;
}

RuntimeManager::RuntimeManager()
{
    mono_config_parse(NULL);

    mono_set_dirs("./lib", "./etc");

    // m_mainDomain = mono_jit_init_version("IcarianCS", "v4.0");
    m_mainDomain = mono_jit_init("IcarianCS");

    m_editorDomain = nullptr;

    m_built = false;
}
RuntimeManager::~RuntimeManager()
{
    mono_jit_cleanup(m_mainDomain);
}

bool RuntimeManager::Build(const std::filesystem::path& a_path, const std::string_view& a_name)
{
    const std::filesystem::path cachePath = a_path / ".cache";
    const std::filesystem::path projectPath = a_path / "Project";
    const std::filesystem::path projectFile = cachePath / (std::string(a_name) + ".csproj");

    const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::filesystem::path> projectScripts;
    MonoProjectGenerator::GetScripts(&projectScripts, projectPath, projectPath);

    const std::string projectDependencies[] =
    {
        "System",
        "System.Xml",
        "IcarianEngine"
    };

    const MonoProjectGenerator project = MonoProjectGenerator(projectScripts.data(), (uint32_t)projectScripts.size(), projectDependencies, sizeof(projectDependencies) / sizeof(*projectDependencies));
    project.Serialize(a_name, projectFile, std::filesystem::path("Core") / "Assemblies");

    ConsoleCommand cmd = ConsoleCommand("xbuild");

    const std::string args[] =
    {
        projectFile.string()
    };
    
    const std::string out = cmd.Run(args, sizeof(args) / sizeof(*args));

    const std::vector<std::string> outLines = SplitString(out);

    const std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();

    const double time = std::chrono::duration<double>(endTime - startTime).count();

    Logger::Message("Project Built in " + std::to_string(time) + "s");

    bool error = false;

    for (const std::string& s : outLines)
    {
        if (s.find("Build FAILED") != std::string::npos)
        {
            error = true;

            break;
        }
        else if (s.find("Build succeeded") != std::string::npos)
        {
            break;
        }
        else if (s.find("error") != std::string::npos)
        {
            error = true;

            Logger::Error(s);
        }
        else if (s.find("warning") != std::string::npos)
        {
            Logger::Warning(s);
        }
    }

    m_built = !error;

    return m_built;
}

void RuntimeManager::Start()
{
    if (m_editorDomain != nullptr && m_editorDomain != mono_get_root_domain())
    {
        // TODO:
        // CRITICAL:
        // I have no idea what is going on I have tried alot of methods and it seems I cannot get the AppDomain to unload 
        // It crashes everytime
        // This needs to be fixed but I am not sure what needs to be done and I do not want to spin up another process
        // mono_domain_set(m_mainDomain, 1);
        
        // mono_gc_collect(mono_gc_max_generation());
        // mono_domain_finalize(m_editorDomain, -1);
        // mono_gc_collect(mono_gc_max_generation());

        // MonoException* exc = NULL;
         
        // mono_domain_try_unload(m_editorDomain, (MonoObject**)&exc);

        // mono_domain_unload(m_editorDomain);
        // mono_assembly_close(m_editorAssembly);

        mono_domain_set(m_editorDomain, 0);
        mono_jit_thread_attach(m_editorDomain);

        mono_runtime_invoke(m_editorUnloadMethod, NULL, NULL, NULL);

        m_editorDomain = nullptr;
    }

    m_editorDomain = mono_domain_create_appdomain("IcarianEditor", NULL);
    assert(m_editorDomain != nullptr);
    mono_domain_set(m_editorDomain, 1);

    m_editorAssembly = mono_domain_assembly_open(m_editorDomain, "./IcarianEditorCS.dll");
    assert(m_editorAssembly != nullptr);
    m_editorImage = mono_assembly_get_image(m_editorAssembly);
    MonoClass* editorProgramClass = mono_class_from_name(m_editorImage, "IcarianEditor", "Program");

    MonoMethodDesc* loadDesc = mono_method_desc_new(":Load()", 0);
    MonoMethodDesc* updateDesc = mono_method_desc_new(":Update(double)", 0);
    MonoMethodDesc* unloadDesc = mono_method_desc_new(":Unload()", 0);

    MonoMethod* loadMethod = mono_method_desc_search_in_class(loadDesc, editorProgramClass);
    m_editorUpdateMethod = mono_method_desc_search_in_class(updateDesc, editorProgramClass);
    m_editorUnloadMethod = mono_method_desc_search_in_class(unloadDesc, editorProgramClass);

    ATTACH_FUNCTION(IcarianEngine, Logger, PushMessage);
    ATTACH_FUNCTION(IcarianEngine, Logger, PushWarning);
    ATTACH_FUNCTION(IcarianEngine, Logger, PushError);
    ATTACH_FUNCTION(IcarianEngine, Application, GetEditorState);

    m_engineAssembly = mono_domain_assembly_open(m_editorDomain, "IcarianCS.dll");
    assert(m_engineAssembly != nullptr);
    m_engineImage = mono_assembly_get_image(m_engineAssembly);

    mono_runtime_invoke(loadMethod, NULL, NULL, NULL);

    mono_method_desc_free(loadDesc);
    mono_method_desc_free(updateDesc);
    mono_method_desc_free(unloadDesc);
}

void RuntimeManager::Update()
{
    if (m_built && m_editorDomain != nullptr)
    {
        const double d = 0.0;
        void* args[1];
        args[0] = (void*)&d;

        mono_runtime_invoke(m_editorUpdateMethod, NULL, args, NULL);
    }
}

void RuntimeManager::BindFunction(const std::string_view& a_location, void* a_function)
{
    mono_add_internal_call(a_location.data(), a_function);
}
void RuntimeManager::ExecFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method, void** a_args) const
{
    if (m_editorDomain != nullptr)
    {
        MonoClass* cls = nullptr;
        if (a_namespace.find("IcarianEngine") != std::string::npos)
        {
            cls = mono_class_from_name(m_engineImage, a_namespace.data(), a_class.data());
        }
        else
        {
            cls = mono_class_from_name(m_editorImage, a_namespace.data(), a_class.data());
        }
        assert(cls != nullptr);

        MonoMethodDesc* desc = mono_method_desc_new(a_method.data(), 0);
        MonoMethod* method = mono_method_desc_search_in_class(desc, cls);
        assert(method != nullptr);

        mono_method_desc_free(desc);

        mono_runtime_invoke(method, NULL, a_args, NULL);
    }
}