using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEditor.Modals
{
    public class AddComponentModal : Modal
    {
        struct ComponentData
        {
            public string Name;
            public ComponentDef Def;
        }

        int                  m_selection;
        List<ComponentData>  m_components;
        GameObjectDef        m_def;

        public AddComponentModal(GameObjectDef a_def) : base("Add Component", new Vector2(250, 80))
        {
            m_def = a_def;

            m_selection = 0;
            m_components = new List<ComponentData>();

            IEnumerable<ComponentDef> defs = DefLibrary.GetDefs<ComponentDef>();
            foreach (ComponentDef def in defs)
            {
                m_components.Add(new ComponentData()
                {
                    Name = def.DefName,
                    Def = def
                });
            }
        }

        public override bool Update()
        {
            int count = m_components.Count;
            string[] names = new string[count];
            for (int i = 0; i < count; ++i)
            {
                names[i] = m_components[i].Name;
            }
            
            GUI.StringSelector("ComponentDef", names, ref m_selection);

            if (GUI.Button("Ok"))
            {
                if (m_selection < 0 || m_selection >= count)
                {
                    Logger.Error("Invalid component selection");

                    return true;
                }

                ComponentData data = m_components[m_selection];

                m_def.Components.Add(data.Def);

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