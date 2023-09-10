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
            Scene scene = Workspace.GetScene();

            if (scene == null)
            {
                return;
            }

            if (GUI.Button("Create Def"))
            {
                new CreateSceneDefModal();
            }

            IEnumerable<Def> defs = Workspace.SceneDefs;

            foreach (Def def in defs)
            {
                string defName = def.DefName;
                GUI.PushID(defName);

                if (GUI.Button("-"))
                {
                    new ConfirmModal("Delete " + defName + "?", () =>
                    {
                        Workspace.SceneDefs.Remove(def);

                        Type defLibraryType = typeof(DefLibrary);
                        
                        FieldInfo sceneDefsField = defLibraryType.GetField("s_sceneDefs", BindingFlags.NonPublic | BindingFlags.Static);

                        List<Def> sceneDefs = sceneDefsField.GetValue(null) as List<Def>;
                        sceneDefs.Remove(def);

                        FieldInfo sceneLookupField = defLibraryType.GetField("s_sceneLookup", BindingFlags.NonPublic | BindingFlags.Static);

                        ConcurrentDictionary<string, Def> sceneLookup = sceneLookupField.GetValue(null) as ConcurrentDictionary<string, Def>;
                        sceneLookup.TryRemove(defName, out Def _);
                    });

                    GUI.PopID();

                    break;
                }

                GUI.SameLine();

                if (GUI.Selectable(defName))
                {
                    Workspace.ClearSelection();
                    Workspace.AddDefSelection(def);
                }

                GUI.PopID();
            }
        }
    }   
}