// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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

    MonoAssembly* m_projectAssembly;
    MonoAssembly* m_projectEditorAssembly;

    MonoAssembly* m_engineAssembly;
    MonoImage*    m_engineImage;

    bool          m_built;
    
    void UnloadEditorDomain();

protected:

public:
    RuntimeManager();
    ~RuntimeManager();

    inline bool IsBuilt() const
    {
        return m_built;
    }
    inline bool IsRunning() const
    {
        return m_editorDomain != NULL;
    }

    inline MonoDomain* GetEditorDomain() const
    {
        return m_editorDomain;
    }

    bool Build(const std::filesystem::path& a_path, const std::string_view& a_name);

    void Start(const std::filesystem::path& a_path, const std::string_view& a_name);

    void Update(double a_delta);

    MonoClass* GetClass(const std::string_view& a_namespace, const std::string_view& a_name) const;

    void BindFunction(const std::string_view& a_location, void* a_function);
    void ExecFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method, void** a_args) const;
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