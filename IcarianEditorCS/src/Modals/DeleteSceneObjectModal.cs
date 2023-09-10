using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Modals
{
    public class DeleteSceneObjectModal : Modal
    {
        ulong m_id;
        bool  m_sceneDef;
        bool  m_deleteDef;

        public DeleteSceneObjectModal(ulong a_id) : base("Delete Scene Object", new Vector2(375.0f, 100.0f))
        {
            m_id = a_id;

            m_sceneDef = false;
            m_deleteDef = false;

            SceneObject obj = Workspace.GetSceneObject(m_id);
            GameObjectDef def = DefLibrary.GetDef(obj.DefName) as GameObjectDef;
            m_sceneDef = def.IsSceneDef;
        }

        public override bool Update()
        {
            GUI.Label("Are you sure you want to delete this scene object?");

            GUI.Separator();

            if (m_sceneDef)
            {
                GUI.Checkbox("Delete Def", ref m_deleteDef);
            }

            if (GUI.Button("Ok"))
            {
                SceneObject obj = Workspace.GetSceneObject(m_id);
                Workspace.DeleteSceneObject(obj);

                if (m_deleteDef)
                {
                    GameObjectDef def = DefLibrary.GetDef(obj.DefName) as GameObjectDef;

                    Workspace.SceneDefs.Remove(def);

                    Type defLibraryType = typeof(DefLibrary);
                        
                    FieldInfo sceneDefsField = defLibraryType.GetField("s_sceneDefs", BindingFlags.NonPublic | BindingFlags.Static);

                    List<Def> sceneDefs = sceneDefsField.GetValue(null) as List<Def>;
                    sceneDefs.Remove(def);

                    FieldInfo sceneLookupField = defLibraryType.GetField("s_sceneLookup", BindingFlags.NonPublic | BindingFlags.Static);

                    ConcurrentDictionary<string, Def> sceneLookup = sceneLookupField.GetValue(null) as ConcurrentDictionary<string, Def>;
                    sceneLookup.TryRemove(def.DefName, out Def _);
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