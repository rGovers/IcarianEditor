using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

#include "InteropBinding.h"
#include "WorkspaceInterop.h"

WORKSPACE_EXPORT_TABLE(IOP_BIND_FUNCTION)

namespace IcarianEditor
{
    

    public enum SelectionObjectMode
    {
        SceneObject,
        GameObjectDef
    }

    public struct SelectionObject
    {
        public ulong ID;
        public SelectionObjectMode SelectionMode;
        public SceneObject SceneObject;
        public string GameObjectDefName;

        public Vector3 Translation
        {
            get
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(GameObjectDefName);

                    return def.Translation;
                }
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject.Translation;
                }
                }

                return Vector3.Zero;
            }
            set
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(GameObjectDefName);

                    def.Translation = value;

                    EditorDefLibrary.RebuildDefData(def);

                    break;
                }
                case SelectionObjectMode.SceneObject:
                {
                    SceneObject.Translation = value;

                    break;
                }
                }
            }
        }

        public Quaternion Rotation
        {
            get
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(GameObjectDefName);

                    return def.Rotation;
                }
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject.Rotation;
                }
                }

                return Quaternion.Identity;
            }
            set
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(GameObjectDefName);

                    def.Rotation = value;

                    EditorDefLibrary.RebuildDefData(def);

                    break;
                }
                case SelectionObjectMode.SceneObject:
                {
                    SceneObject.Rotation = value;

                    break;
                }
                }
            }
        }

        public Vector3 Scale
        {
            get
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(GameObjectDefName);

                    return def.Scale;
                }
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject.Scale;
                }
                }

                return Vector3.Zero;
            }
            set
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(GameObjectDefName);

                    def.Scale = value;

                    EditorDefLibrary.RebuildDefData(def);

                    break;
                }
                case SelectionObjectMode.SceneObject:
                {
                    SceneObject.Scale = value;

                    break;
                }
                }
            }
        }
    }

    public static class Workspace
    {
        static ulong                 s_id = 0;

        static List<SelectionObject> s_selection = new List<SelectionObject>();
        static List<string>          s_selectedDefs = new List<string>();
        
        public static IEnumerable<SelectionObject> Selection
        {
            get
            {
                return s_selection;
            }
        }
        public static IEnumerable<string> SelectedDefs
        {
            get
            {
                return s_selectedDefs;
            }
        }

        public static bool IsSelectionEmpty
        {
            get
            {
                return s_selection.Count == 0;
            }
        }
        public static bool IsDefSelectionEmpty
        {
            get
            {
                return s_selectedDefs.Count == 0;
            }
        }

        public static string CurrentScenePath
        {
            get
            {
                return WorkspaceInterop.GetCurrentScene();
            }
            set
            {
                WorkspaceInterop.SetCurrentScene(value);
            }
        }

        public static ManipulationMode ManipulationMode
        {
            get
            {
                return (ManipulationMode)WorkspaceInterop.GetManipulationMode();
            }
        }

        public static void AddSelection(SelectionObject a_object)
        {
            s_selection.Add(a_object);
        }
        public static void AddSelection(IEnumerable<SelectionObject> a_objects)
        {
            s_selection.AddRange(a_objects);
        }
        public static void AddDefSelection(string a_name)
        {
            s_selectedDefs.Add(a_name);
        }
        public static void AddDefSelection(IEnumerable<string> a_names)
        {
            s_selectedDefs.AddRange(a_names);
        }

        public static void ClearSelection()
        {
            s_selection.Clear();
            s_selectedDefs.Clear();
        }

        public static bool SelectionContains(SceneObject a_obj)
        {
            foreach (SelectionObject obj in s_selection)
            {
                if (obj.SceneObject == a_obj)
                {
                    return true;
                }
            }

            return false;
        }
        public static bool SelectionContains(SceneObject a_obj, string a_def)
        {
            foreach (SelectionObject obj in s_selection)
            {
                if (obj.SceneObject == a_obj && obj.GameObjectDefName == a_def)
                {
                    return true;
                }
            }

            return false;
        }
        public static bool SelectionContains(SceneObject a_obj, GameObjectDef a_def)
        {
            foreach (SelectionObject obj in s_selection)
            {
                if (obj.SceneObject == a_obj && obj.GameObjectDefName == a_def.DefName)
                {
                    return true;
                }
            }

            return false;
        }

        public static ulong NewID()
        {
            return s_id++;
        }

        static void PushDef(string a_path)
        {
            ClearSelection();

            string defName = EditorDefLibrary.GetDefName(a_path);
            if (!string.IsNullOrWhiteSpace(defName))
            {
                AddDefSelection(defName);
            }
        }

        public static EditorScene GetScene()
        {
            string curPath = CurrentScenePath;
            if (string.IsNullOrWhiteSpace(curPath))
            {
                return null;
            }

            return EditorScene.GetScene(curPath);
        }
    }
}