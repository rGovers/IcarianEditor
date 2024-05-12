using IcarianEditor.Modals;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Windows
{
    public static class SceneDefsWindow
    {
        static string s_searchString = string.Empty;

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

            GUI.SameLine();

            GUI.StringField("Search", ref s_searchString);

            if (GUI.BeginChild("##DefList", Vector2.Zero))
            {
                IEnumerable<string> defNames = scene.DefNames;

                foreach (string defName in defNames)
                {
                    if (!string.IsNullOrWhiteSpace(s_searchString) && !defName.Contains(s_searchString))
                    {
                        continue;
                    }

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

                GUI.EndChild();
            }
        }
    }   
}