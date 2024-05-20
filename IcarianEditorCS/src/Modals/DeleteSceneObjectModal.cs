using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

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

            EditorScene scene = Workspace.GetScene();

            SceneObject obj = scene.GetSceneObject(m_id);
            m_sceneDef = EditorDefLibrary.IsSceneDef(obj.DefName);
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
                EditorScene scene = Workspace.GetScene();
                
                if (m_deleteDef)
                {
                    SceneObject obj = scene.GetSceneObject(m_id);

                    scene.RemoveDef(obj.DefName);
                }

                Workspace.ClearSelection();
                
                scene.RemoveSceneObject(m_id);

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