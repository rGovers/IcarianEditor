#pragma once

#include <filesystem>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string_view>

#if WIN32
#define FLARE_MONO_EXPORT(ret, func, ...) __declspec(dllexport) ret func(__VA_ARGS__)
#else
#define FLARE_MONO_EXPORT(ret, func, ...) static ret func(__VA_ARGS__)
#endif

#define RUNTIME_FUNCTION_NAME(klass, name) MRF_##klass##_##name
#define RUNTIME_FUNCTION_STRING(namespace, klass, name) #namespace "." #klass "::" #name

#define RUNTIME_FUNCTION(ret, klass, name, code, ...) FLARE_MONO_EXPORT(ret, RUNTIME_FUNCTION_NAME(klass, name), __VA_ARGS__) code

#define RUNTIME_FUNCTION_DEFINITION(ret, namespace, klass, name, code, ...) RUNTIME_FUNCTION(ret, klass, name, code, __VA_ARGS__)

#define BIND_FUNCTION(runtime, namespace, klass, name) runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name))

class RuntimeManager
{
private:
    MonoDomain*   m_mainDomain;
  
    MonoDomain*   m_editorDomain;
    MonoAssembly* m_editorAssembly;
    MonoImage*    m_editorImage;
    MonoMethod*   m_editorUpdateMethod;
    MonoMethod*   m_editorUnloadMethod;

    MonoAssembly* m_engineAssembly;
    MonoImage*    m_engineImage;

    bool          m_built;
    
protected:

public:
    RuntimeManager();
    ~RuntimeManager();

    inline bool IsBuilt() const
    {
        return m_built;
    }

    inline MonoDomain* GetEditorDomain() const
    {
        return m_editorDomain;
    }

    bool Build(const std::filesystem::path& a_path, const std::string_view& a_name);

    void Start();

    void Update();

    void BindFunction(const std::string_view& a_location, void* a_function);
    void ExecFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method, void** a_args) const;
};