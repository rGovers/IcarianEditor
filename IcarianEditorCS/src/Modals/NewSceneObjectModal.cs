using System.Collections.Generic;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Modals
{
    public class NewSceneObjectModal : Modal
    {
        bool         m_array; 
        int          m_selection;
        List<string> m_defNames;

        public NewSceneObjectModal(bool a_array = false) : base("New Scene Object", new Vector2(300.0f, 80.0f))
        {
            m_array = a_array;
            m_selection = 0;
            m_defNames = new List<string>();

            IEnumerable<GameObjectDef> defs = EditorDefLibrary.GenerateDefs<GameObjectDef>();
            foreach (GameObjectDef def in defs)
            {
                m_defNames.Add(def.DefName);
            }
        }

        public override bool Update()
        {
            string[] names = m_defNames.ToArray();
            GUI.StringSelector("GameObjectDef", names, ref m_selection);

            if (GUI.Button("Ok"))
            {
                if (m_selection < 0 || m_selection >= m_defNames.Count)
                {
                    Logger.Error("Invalid selection index");

                    return true;
                }

                EditorScene scene = Workspace.GetScene();
                if (m_array)
                {
                    scene.AddSceneObjectArray(m_defNames[m_selection]);
                }
                else
                {
                    scene.AddSceneObject(m_defNames[m_selection]);
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