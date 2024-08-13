// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

#pragma once

#include <filesystem>
#include <vector>

struct MonoExternalReference
{
	std::string Name;
	std::filesystem::path Path;
};

class MonoProjectGenerator
{
private:
	uint32_t			   m_scriptCount;
	std::filesystem::path* m_scripts;

	uint32_t			   m_dependencyCount;
	std::string*		   m_dependencies;

protected:

public:
	static void GetScripts(std::vector<std::filesystem::path>* a_scripts, const std::filesystem::path& a_dir, const std::filesystem::path& a_workingDir);
	
	MonoProjectGenerator(const std::filesystem::path* a_scripts, uint32_t a_scriptCount, const std::string* a_dependencies, uint32_t a_dependencyCount);
	~MonoProjectGenerator();

	void Serialize(const std::string_view& a_name, const std::filesystem::path& a_path, const std::filesystem::path& a_outPath, const MonoExternalReference* a_externalReferences = nullptr, uint32_t a_externalReferenceCount = 0) const;
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