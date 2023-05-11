#pragma once

#include <filesystem>
#include <vector>

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

	void Serialize(const std::string_view& a_name, const std::filesystem::path& a_path, const std::filesystem::path& a_outPath) const;
};