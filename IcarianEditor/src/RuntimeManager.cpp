#include "Runtime/RuntimeManager.h"

#include <chrono>
#include <cstring>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <mono/utils/mono-dl-fallback.h>
#include <string>

#include "ConsoleCommand.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
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

#ifndef WIN32
#include "Flare/MonoNativeImpl.h"

static constexpr char MonoNativeLibName[] = "libmono-native.so";
static constexpr uint32_t MonoNativeLibNameLength = sizeof(MonoNativeLibName) - 1;

#define MonoThisLibHandle ((void*)-1)

static void* RuntimeDLOpen(const char* a_name, int a_flags, char** a_error, void* a_userData)
{
    const uint32_t len = (uint32_t)strlen(a_name);
    const char* ptr = a_name + len - MonoNativeLibNameLength;

    if (len > MonoNativeLibNameLength && strcmp(ptr, MonoNativeLibName) == 0)
    {
        return MonoThisLibHandle;
    }

    return NULL;
}
static void* RuntimeDLSymbol(void* a_handle, const char* a_name, char** a_error, void* a_userData)
{
    if (a_handle == MonoThisLibHandle)
    {
        return FlareBase::MonoNativeImpl::GetFunction(a_name);
    }

    return NULL;
}
#endif

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

    const std::filesystem::path currentDir = std::filesystem::current_path();

    const std::filesystem::path libDir = currentDir / "lib";
    const std::filesystem::path etcDir = currentDir / "etc";

    const std::filesystem::path assemblyPath = libDir / "mono" / "4.5";

    mono_set_assemblies_path(assemblyPath.string().c_str());

    mono_set_dirs(libDir.string().c_str(), etcDir.string().c_str());

#ifndef WIN32
    FlareBase::MonoNativeImpl::Init();

    mono_dl_fallback_register(RuntimeDLOpen, RuntimeDLSymbol, NULL, NULL);
#endif

    m_mainDomain = mono_jit_init("IcarianCS");

    m_editorDomain = nullptr;

    m_built = false;
}
RuntimeManager::~RuntimeManager()
{
#ifndef WIN32
    mono_dl_fallback_unregister(NULL);

    FlareBase::MonoNativeImpl::Destroy();
#endif

    mono_jit_cleanup(m_mainDomain);
}

static bool ParseCommandOutput(const std::string_view& a_output)
{
    const std::vector<std::string> outLines = SplitString(a_output);

    bool error = false;

    for (const std::string& s : outLines)
    {
        if (s.find("Build FAILED") != std::string::npos)
        {
            return false;
        }
        else if (s.find("Build succeeded") != std::string::npos)
        {
            return true;
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

    return !error;
}

bool RuntimeManager::Build(const std::filesystem::path& a_path, const std::string_view& a_name)
{
    const std::filesystem::path cachePath = a_path / ".cache";
    const std::filesystem::path projectPath = a_path / "Project";
    const std::filesystem::path projectFile = cachePath / (std::string(a_name) + ".csproj");
    const std::filesystem::path assemblyPath = std::filesystem::path("Core") / "Assemblies";

    const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    IDEFER(
    {
        const std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();

        const double time = std::chrono::duration<double>(endTime - startTime).count();

        Logger::Message("Project Built in " + std::to_string(time) + "s");
    });

    std::vector<std::filesystem::path> projectScripts;
    MonoProjectGenerator::GetScripts(&projectScripts, projectPath, projectPath);

    const std::string projectDependencies[] =
    {
        "System",
        "System.Xml",
        "IcarianEngine"
    };

    const MonoProjectGenerator project = MonoProjectGenerator(projectScripts.data(), (uint32_t)projectScripts.size(), projectDependencies, sizeof(projectDependencies) / sizeof(*projectDependencies));
    project.Serialize(a_name, projectFile, assemblyPath);

    ConsoleCommand cmd = ConsoleCommand("xbuild");

    const std::string projectArgs[] =
    {
        projectFile.string()
    };
    
    const std::string out = cmd.Run(projectArgs, sizeof(projectArgs) / sizeof(*projectArgs));

    m_built = ParseCommandOutput(out);

    if (m_built)
    {
        const std::filesystem::path editorPath = projectPath / "Editor";
        const std::filesystem::path editorProjectFile = cachePath / (std::string(a_name) + "Editor.csproj");

        std::vector<std::filesystem::path> editorScripts;
        MonoProjectGenerator::GetScripts(&editorScripts, editorPath, projectPath);

        const std::string editorDependencies[] = 
        {
            "System",
            "System.Xml",
            "IcarianEngine",
            "IcarianEditor"
        };

        const MonoExternalReference externalDependencies[] =
        {
            { std::string(a_name), cachePath / assemblyPath / (std::string(a_name) + ".dll") },
        };

        const MonoProjectGenerator editorProject = MonoProjectGenerator(editorScripts.data(), (uint32_t)editorScripts.size(), editorDependencies, sizeof(editorDependencies) / sizeof(*editorDependencies));
        editorProject.Serialize(std::string(a_name) + "Editor", editorProjectFile, "Editor", externalDependencies, sizeof(externalDependencies) / sizeof(*externalDependencies));

        const std::string projectEditorArgs[] =
        {
            editorProjectFile.string()
        };

        const std::string output = cmd.Run(projectEditorArgs, sizeof(projectEditorArgs) / sizeof(*projectEditorArgs));

        m_built = ParseCommandOutput(output);
    }

    return m_built;
}

static constexpr char EditorDomainName[] = "IcarianEditor";
static constexpr uint32_t EditorDomainNameLength = sizeof(EditorDomainName);

void RuntimeManager::Start(const std::filesystem::path& a_path, const std::string_view& a_name)
{
    if (m_editorDomain != nullptr && m_editorDomain != mono_get_root_domain())
    {
        mono_domain_set(m_editorDomain, 1);

        mono_runtime_invoke(m_editorUnloadMethod, NULL, NULL, NULL);
        
        mono_domain_set(m_mainDomain, 1);

        // Updated mono and magically fixed the crash related to unloading the domain
        // not going to question it
        mono_domain_unload(m_editorDomain);

        mono_gc_collect(mono_gc_max_generation());
    }

    // Copying the string to shut up the warning about C++11
    char editorDomainName[EditorDomainNameLength];
    for (uint32_t i = 0; i < EditorDomainNameLength; ++i)
    {
        editorDomainName[i] = EditorDomainName[i];
    }

    m_editorDomain = mono_domain_create_appdomain(editorDomainName, NULL);
    ICARIAN_ASSERT(m_editorDomain != NULL);
    mono_domain_set(m_editorDomain, 1);    

    m_editorAssembly = mono_domain_assembly_open(m_editorDomain, "./IcarianEditorCS.dll");
    ICARIAN_ASSERT(m_editorAssembly != NULL);

    if (m_built)
    {
        const std::filesystem::path assemblyPath = a_path / ".cache" / "Editor" / (std::string(a_name) + "Editor.dll");

        m_projectEditorAssembly = mono_domain_assembly_open(m_editorDomain, assemblyPath.string().c_str());
    }

    m_editorImage = mono_assembly_get_image(m_editorAssembly);
    MonoClass* editorProgramClass = mono_class_from_name(m_editorImage, "IcarianEditor", "Program");

    MonoMethodDesc* loadDesc = mono_method_desc_new(":Load()", 0);
    IDEFER(mono_method_desc_free(loadDesc));
    MonoMethodDesc* updateDesc = mono_method_desc_new(":Update(double)", 0);
    IDEFER(mono_method_desc_free(updateDesc));
    MonoMethodDesc* unloadDesc = mono_method_desc_new(":Unload()", 0);
    IDEFER(mono_method_desc_free(unloadDesc));

    MonoMethod* loadMethod = mono_method_desc_search_in_class(loadDesc, editorProgramClass);
    m_editorUpdateMethod = mono_method_desc_search_in_class(updateDesc, editorProgramClass);
    m_editorUnloadMethod = mono_method_desc_search_in_class(unloadDesc, editorProgramClass);

    ATTACH_FUNCTION(IcarianEngine, Logger, PushMessage);
    ATTACH_FUNCTION(IcarianEngine, Logger, PushWarning);
    ATTACH_FUNCTION(IcarianEngine, Logger, PushError);
    ATTACH_FUNCTION(IcarianEngine, Application, GetEditorState);

    m_engineAssembly = mono_domain_assembly_open(m_editorDomain, "IcarianCS.dll");
    ICARIAN_ASSERT(m_engineAssembly != NULL);
    m_engineImage = mono_assembly_get_image(m_engineAssembly);

    mono_runtime_invoke(loadMethod, NULL, NULL, NULL);
}

void RuntimeManager::Update(double a_delta)
{
    if (m_built && m_editorDomain != nullptr)
    {
        void* args[] =
        {
            &a_delta
        };

        mono_runtime_invoke(m_editorUpdateMethod, NULL, args, NULL);
    }
}

MonoClass* RuntimeManager::GetClass(const std::string_view& a_namespace, const std::string_view& a_class) const
{
    if (m_editorDomain != NULL)
    {
        if (a_namespace.find("IcarianEngine") != std::string::npos)
        {
            return mono_class_from_name(m_engineImage, a_namespace.data(), a_class.data());
        }
        else 
        {
            return mono_class_from_name(m_editorImage, a_namespace.data(), a_class.data());
        }
    }

    return NULL;
}

void RuntimeManager::BindFunction(const std::string_view& a_location, void* a_function)
{
    mono_add_internal_call(a_location.data(), a_function);
}
void RuntimeManager::ExecFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method, void** a_args) const
{
    if (m_editorDomain != nullptr)
    {
        MonoClass* cls = GetClass(a_namespace, a_class);
        ICARIAN_ASSERT(cls != nullptr);

        MonoMethodDesc* desc = mono_method_desc_new(a_method.data(), 0);
        MonoMethod* method = mono_method_desc_search_in_class(desc, cls);
        ICARIAN_ASSERT(method != nullptr);

        mono_method_desc_free(desc);

        mono_runtime_invoke(method, NULL, a_args, NULL);
    }
}