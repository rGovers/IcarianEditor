using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;
using IcarianEditor.Modals;
using IcarianEngine;
using IcarianEngine.Definitions;

namespace IcarianEditor.Windows
{
    public static class SceneDefsWindow
    {
        static void OnGUI()
        {
            EditorScene scene = Workspace.GetScene();
            if (scene == null)
            {
                return;
            }

            if (GUI.Button("Create Def"))
            {
                new CreateSceneDefModal();
            }

            IEnumerable<string> defNames = scene.DefNames;

            foreach (string defName in defNames)
            {
                GUI.PushID(defName);

                if (GUI.Button("-"))
                {
                    new ConfirmModal("Delete " + defName + "?", () =>
                    {
                        EditorScene scene = Workspace.GetScene();

                        scene.RemoveDef(defName);
                    });

                    GUI.PopID();

                    break;
                }

                GUI.SameLine();

                if (GUI.Selectable(defName))
                {
                    Workspace.ClearSelection();

                    Workspace.AddDefSelection(defName);
                }

                GUI.PopID();
            }
        }
    }   
}