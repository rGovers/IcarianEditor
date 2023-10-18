using System.Collections.Generic;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Modals
{
    public class GUIGetDefSelectorModal<T> : Modal where T : Def
    {
        string       m_input;
        int          m_seletedIndex;
        List<string> m_defNames;

        public GUIGetDefSelectorModal(string a_input, Def a_currentDef, bool a_useSceneDefs) : base("Def Selector", new Vector2(250, 80))
        {
            m_input = a_input;
            m_seletedIndex = 0;

            m_defNames = new List<string>();

            IEnumerable<T> defs = EditorDefLibrary.GenerateDefs<T>();

            m_defNames.Add("Null");

            if (a_useSceneDefs)
            {
                foreach (T def in defs)
                {
                    if (def == a_currentDef)
                    {
                        m_seletedIndex = m_defNames.Count;
                    }

                    m_defNames.Add(def.DefName);                    
                }
            }
            else
            {
                foreach (T def in defs)
                {
                    if (def.IsSceneDef)
                    {
                        continue;
                    }

                    if (def == a_currentDef)
                    {
                        m_seletedIndex = m_defNames.Count;
                    }

                    m_defNames.Add(def.DefName);
                }
            }
        }

        public override bool Update()
        {
            GUI.StringSelector("Def", m_defNames.ToArray(), ref m_seletedIndex);

            if (GUI.Button("Ok"))
            {
                if (m_seletedIndex <= 0)
                {
                    GUI.PushDef(m_input, null);
                }
                else
                {
                    GUI.PushDef(m_input, EditorDefLibrary.GenerateDef(m_defNames[m_seletedIndex]));
                }

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