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
                        if (type.Namespace == EditorDefLibrary.DefintionNamespace)
                        {
                            name = type.Name;
                        }
                        
                        m_defData.Add(new DefData()
                        {
                            Name = name,
                            Type = type
                        });
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

                if (EditorDefLibrary.Exists(m_defName))
                {
                    Logger.Error("Def already exists");

                    return true;
                }

                DefData data = m_defData[m_selection];

                Def def = Activator.CreateInstance(data.Type) as Def;
                if (def == null)
                {
                    Logger.Error("Failed to create def");

                    return true;
                }
                
                def.DefName = m_defName;
                def.DefPath = Def.SceneDefPath;

                EditorScene scene = Workspace.GetScene();
                scene.AddDef(def);

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