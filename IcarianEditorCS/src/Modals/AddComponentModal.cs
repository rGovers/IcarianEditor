using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Modals
{
    public class AddComponentModal : Modal
    {
        struct ComponentData
        {
            public string Name;
            public Type Type;
        }

        int                  m_selection;
        List<ComponentData>  m_componentNames;
        GameObjectDef        m_def;

        public AddComponentModal(GameObjectDef a_def) : base("Add Component", new Vector2(250, 80))
        {
            m_def = a_def;

            m_selection = 0;
            m_componentNames = new List<ComponentData>();

            Type baseType = typeof(ComponentDef);

            m_componentNames.Add(new ComponentData()
            {
                Name = "ComponentDef",
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
                            m_componentNames.Add(new ComponentData()
                            {
                                Name = type.Name,
                                Type = type
                            });
                        }
                        else
                        {
                            m_componentNames.Add(new ComponentData()
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
            string[] names = new string[m_componentNames.Count];
            for (int i = 0; i < m_componentNames.Count; ++i)
            {
                names[i] = m_componentNames[i].Name;
            }
            
            GUI.StringSelector("Component Type", names, ref m_selection);

            if (GUI.Button("Ok"))
            {
                if (m_selection < 0)
                {
                    Logger.Error("Invalid component selection");

                    return true;
                }

                ComponentData data = m_componentNames[m_selection];

                ComponentDef comp = (ComponentDef)Activator.CreateInstance(data.Type);
                m_def.Components.Add(comp);

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