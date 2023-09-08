using System.Collections.Generic;
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
                    Workspace.SceneDefs.Remove(def);

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