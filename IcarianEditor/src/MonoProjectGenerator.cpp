#include "MonoProjectGenerator.h"

#include <tinyxml2.h>

#include "IO.h"
#include "Logger.h"

void MonoProjectGenerator::GetScripts(std::vector<std::filesystem::path>* a_scripts, const std::filesystem::path& a_dir, const std::filesystem::path& a_workingDir)
{
	if (!std::filesystem::exists(a_dir))
	{
		return;
	}

	for (const auto& iter : std::filesystem::directory_iterator(a_dir, std::filesystem::directory_options::skip_permission_denied))
	{
		if (iter.is_directory())
		{
			const std::filesystem::path r = IO::GetRelativePath(a_workingDir, iter);
			if (r != "Editor")
			{
				GetScripts(a_scripts, iter, a_workingDir);
			}
		}
		else if (iter.is_regular_file())
		{
			if (iter.path().extension() == ".cs")
			{
				a_scripts->emplace_back(IO::GetRelativePath(a_workingDir, iter));
			}
		}
	}
}

MonoProjectGenerator::MonoProjectGenerator(const std::filesystem::path* a_scripts, uint32_t a_scriptCount, const std::string* a_dependencies, uint32_t a_dependencyCount)
{
	m_scriptCount = a_scriptCount;
	m_scripts = new std::filesystem::path[m_scriptCount];

	for (uint32_t i = 0; i < m_scriptCount; ++i)
	{
		m_scripts[i] = a_scripts[i];
	}

	m_dependencyCount = a_dependencyCount;
	m_dependencies = new std::string[m_dependencyCount];
	
	for (uint32_t i = 0; i < m_dependencyCount; ++i)
	{
		m_dependencies[i] = a_dependencies[i];
	}
}
MonoProjectGenerator::~MonoProjectGenerator()
{
	delete[] m_scripts;
	delete[] m_dependencies;
}

void MonoProjectGenerator::Serialize(const std::string_view& a_name, const std::filesystem::path& a_path, const std::filesystem::path& a_outPath, const MonoExternalReference* a_externalReferences, uint32_t a_externalReferenceCount) const
{
	tinyxml2::XMLDocument doc;

	doc.InsertEndChild(doc.NewDeclaration(nullptr));

	tinyxml2::XMLElement* root = doc.NewElement("Project");
	doc.InsertEndChild(root);
	root->SetAttribute("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	tinyxml2::XMLElement* propertyGroup = doc.NewElement("PropertyGroup");
	root->InsertEndChild(propertyGroup);

	tinyxml2::XMLElement* configurationProperty = doc.NewElement("Configuration");
	propertyGroup->InsertEndChild(configurationProperty);
	configurationProperty->SetAttribute("Condition", "'$(Configuration)' == ''");
	configurationProperty->SetText("Debug");

	tinyxml2::XMLElement* outputProperty = doc.NewElement("OutputPath");
	propertyGroup->InsertEndChild(outputProperty);
	outputProperty->SetText(a_outPath.string().c_str());

	tinyxml2::XMLElement* assemblyNameProperty = doc.NewElement("AssemblyName");
	propertyGroup->InsertEndChild(assemblyNameProperty);
	assemblyNameProperty->SetText(a_name.data());

	tinyxml2::XMLElement* targetFrameworkProperty = doc.NewElement("TargetFrameworkVersion");
	propertyGroup->InsertEndChild(targetFrameworkProperty);
	targetFrameworkProperty->SetText("v4.5");

	tinyxml2::XMLElement* outputTypeProperty = doc.NewElement("OutputType");
	propertyGroup->InsertEndChild(outputTypeProperty);
	outputTypeProperty->SetText("Library");

	tinyxml2::XMLElement* debugPropertyGroup = doc.NewElement("PropertyGroup");
	root->InsertEndChild(debugPropertyGroup);
	debugPropertyGroup->SetAttribute("Condition", "'$(Configuration)'=='Debug'");

	tinyxml2::XMLElement* debugOutpath = doc.NewElement("OutputPath");
	debugPropertyGroup->InsertEndChild(debugOutpath);
	debugOutpath->SetText(a_outPath.string().c_str());

	tinyxml2::XMLElement* debugSymbols = doc.NewElement("DebugSymbols");
	debugPropertyGroup->InsertEndChild(debugSymbols);
	debugSymbols->SetText("true");

	tinyxml2::XMLElement* debugType = doc.NewElement("DebugType");
	debugPropertyGroup->InsertEndChild(debugType);
	debugType->SetText("full");

	tinyxml2::XMLElement* debugOptimize = doc.NewElement("Optimize");
	debugPropertyGroup->InsertEndChild(debugOptimize);
	debugOptimize->SetText("false");

	tinyxml2::XMLElement* releasePropertyGroup = doc.NewElement("PropertyGroup");
	root->InsertEndChild(releasePropertyGroup);
	releasePropertyGroup->SetAttribute("Condition", "'$(Configuration)'=='Release'");

	tinyxml2::XMLElement* releaseOutpath = doc.NewElement("OutputPath");
	releasePropertyGroup->InsertEndChild(releaseOutpath);
	releaseOutpath->SetText(a_outPath.string().c_str());

	tinyxml2::XMLElement* releaseSymbols = doc.NewElement("DebugSymbols");
	releasePropertyGroup->InsertEndChild(releaseSymbols);
	releaseSymbols->SetText("false");

	tinyxml2::XMLElement* releaseOptimize = doc.NewElement("Optimize");
	releasePropertyGroup->InsertEndChild(releaseOptimize);
	releaseOptimize->SetText("true");

	if (m_scriptCount > 0)
	{
		tinyxml2::XMLElement* compileItemGroup = doc.NewElement("ItemGroup");
		root->InsertEndChild(compileItemGroup);

		for (uint32_t i = 0; i < m_scriptCount; ++i)
		{
			tinyxml2::XMLElement* compileSource = doc.NewElement("Compile");
			compileItemGroup->InsertEndChild(compileSource);

			const std::filesystem::path p = std::filesystem::path("..") / "Project" / m_scripts[i];
			compileSource->SetAttribute("Include", p.string().c_str());
		}
	}

	if (m_dependencyCount > 0)
	{
		tinyxml2::XMLElement* referenceItemGroup = doc.NewElement("ItemGroup");
		root->InsertEndChild(referenceItemGroup);

		for (uint32_t i = 0; i < m_dependencyCount; ++i)
		{
			tinyxml2::XMLElement* reference = doc.NewElement("Reference");
			referenceItemGroup->InsertEndChild(reference);

			if (m_dependencies[i] == "IcarianEngine")
			{
				const std::filesystem::path cwd = std::filesystem::current_path();
				const std::filesystem::path p = cwd / "IcarianCS.dll";

				reference->SetAttribute("Include", "IcarianCS");

				tinyxml2::XMLElement* hintPath = doc.NewElement("HintPath");
				reference->InsertEndChild(hintPath);
				hintPath->SetText(p.string().c_str());

				tinyxml2::XMLElement* priv = doc.NewElement("Private");
				reference->InsertEndChild(priv);
				priv->SetText("true");
			}
			else if (m_dependencies[i] == "IcarianEditor")
			{
				const std::filesystem::path cwd = std::filesystem::current_path();
				const std::filesystem::path p = cwd / "IcarianEditorCS.dll";

				reference->SetAttribute("Include", "IcarianEditorCS");

				tinyxml2::XMLElement* hintPath = doc.NewElement("HintPath");
				reference->InsertEndChild(hintPath);
				hintPath->SetText(p.string().c_str());

				tinyxml2::XMLElement* priv = doc.NewElement("Private");
				reference->InsertEndChild(priv);
				priv->SetText("true");
			}
			else
			{
				reference->SetAttribute("Include", m_dependencies[i].c_str());
			}
		}

		if (a_externalReferences != nullptr)
		{
			for (uint32_t i = 0; i < a_externalReferenceCount; ++i)
			{
				tinyxml2::XMLElement* reference = doc.NewElement("Reference");
				referenceItemGroup->InsertEndChild(reference);
	
				reference->SetAttribute("Include", a_externalReferences[i].Name.c_str());
	
				tinyxml2::XMLElement* hintPath = doc.NewElement("HintPath");
				reference->InsertEndChild(hintPath);
				hintPath->SetText(a_externalReferences[i].Path.string().c_str());
			}
		}
	}

	tinyxml2::XMLElement* imp = doc.NewElement("Import");
	root->InsertEndChild(imp);
	imp->SetAttribute("Project", "$(MSBuildToolsPath)/Microsoft.CSharp.targets");

	doc.SaveFile(a_path.string().c_str());
}