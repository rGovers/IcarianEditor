// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <filesystem>
#include <functional>
#include <glad/glad.h>
#include <unordered_map>

class AssetLibrary;
class RuntimeStorage;
class Texture;
class Workspace;

#include "EditorFileHandlerInteropStructures.h"

class FileHandler
{
public:
    using FileCallback = std::function<void(const std::filesystem::path&, const std::filesystem::path&, uint32_t, const uint8_t*)>;

private:
    FileTextureHandle                             m_runtimeTexHandle;

    AssetLibrary*                                 m_assets;
    RuntimeStorage*                               m_storage;

    std::unordered_map<std::string, Texture*>     m_extTex;
    std::unordered_map<std::string, FileCallback> m_extOpenCallback;
    std::unordered_map<std::string, FileCallback> m_extDragCallback;

    FileHandler(AssetLibrary* a_assets, RuntimeStorage* a_storage, Workspace* a_workspace);
    
protected:

public:
    ~FileHandler();

    static void SetFileHandle(const FileTextureHandle& a_handle);

    static void Init(AssetLibrary* a_assets, RuntimeStorage* a_storage, Workspace* a_workspace);
    static void Destroy();

    static void GetFileData(const std::filesystem::path& a_path, FileCallback** a_openCallback, FileCallback** a_dragCallback, GLuint* a_texture);
};

// MIT License
// 
// Copyright (c) 2025 River Govers
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