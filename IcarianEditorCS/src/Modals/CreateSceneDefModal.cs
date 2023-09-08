using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Modals
{
    public class CreateSceneDefModal : Modal
    {
        struct DefData
        {
            public string Name;
            public Type Type;
        };

        int           m_selection;
        string        m_defName;
        List<DefData> m_defData;

        public CreateSceneDefModal() : base("Create Scene Def", new Vector2(250, 100))
        {
            m_defName = "";
            m_selection = 0;
            m_defData = new List<DefData>();

            Type baseType = typeof(Def);

            m_defData.Add(new DefData()
            {
                Name = "Def",
                Type = baseType
            });

            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (Assembly asm in assemblies)
            {
                Type[] types = asm.GetTypes();
                foreach (Type type in types)
                {
                    if (type.IsAbstract)
                    {
                        continue;
                    }

                    if (type.IsSubclassOf(baseType))
                    {
                        string name = type.FullName;
                        if (name.StartsWith("IcarianEngine"))
                        {
                            m_defData.Add(new DefData()
                            {
                                Name = type.Name,
                                Type = type
                            });
                        }
                        else
                        {
                            m_defData.Add(new DefData()
                            {
                                Name = name,
                                Type = type
                            });
                        }
                    }
                }
            }
        }

        public override bool Update()
        {
            GUI.StringField("Def Name", ref m_defName);

            int count = m_defData.Count;
            string[] defTypes = new string[count];
            for (int i = 0; i < count; ++i)
            {
                defTypes[i] = m_defData[i].Name;
            }

            GUI.StringSelector("Def Type", defTypes, ref m_selection);

            if (GUI.Button("Ok"))
            {
                if (string.IsNullOrWhiteSpace(m_defName))
                {
                    Logger.Error("Invalid def name");

                    return true;
                }

                if (m_selection < 0 || m_selection >= count)
                {
                    Logger.Error("Invalid def selection");

                    return true;
                }

                if (DefLibrary.GetDef(m_defName) != null)
                {
                    Logger.Error("Def already exists");

                    return true;
                }

                DefData data = m_defData[m_selection];

                Def def = (Def)Activator.CreateInstance(data.Type);
                def.DefName = m_defName;
                def.DefPath = Def.SceneDefPath;

                Type defLibraryType = typeof(DefLibrary);

                FieldInfo sceneDefsField = defLibraryType.GetField("s_sceneDefs", BindingFlags.NonPublic | BindingFlags.Static);
            
                List<Def> sceneDefs = sceneDefsField.GetValue(null) as List<Def>;
                sceneDefs.Add(def);

                FieldInfo sceneLookupField = defLibraryType.GetField("s_sceneLookup", BindingFlags.NonPublic | BindingFlags.Static);

                ConcurrentDictionary<string, Def> sceneLookup = sceneLookupField.GetValue(null) as ConcurrentDictionary<string, Def>;
                sceneLookup.TryAdd(def.DefName, def);

                Workspace.SceneDefs.Add(def);

                return false;
            }

            GUI.SameLine();

            if (GUI.Button("Cancel"))
            {
                return false;
            }

            return true;
        }
    }
}