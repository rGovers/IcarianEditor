// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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
        SceneObjectArray
    }

    // May change this down the line to a abstract class if I need more
    public struct SelectionObject
    {
        public ulong ID;
        public SelectionObjectMode SelectionMode;
        public SceneObject SceneObject;
        public SceneObjectArray SceneObjectArray;

        public object Object
        {
            get
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
                    return SceneObjectArray;
                }
                }

                return null;
            }
        }

        public Vector3 Translation
        {
            get
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject.Translation;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
                    return SceneObjectArray.Translation;
                }
                }

                return Vector3.Zero;
            }
            set
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.SceneObject:
                {
                    SceneObject.Translation = value;

                    break;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
                    SceneObjectArray.Translation = value;

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
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject.Rotation;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
                    return SceneObjectArray.Rotation;
                }
                }

                return Quaternion.Identity;
            }
            set
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.SceneObject:
                {
                    SceneObject.Rotation = value;

                    break;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
                    SceneObjectArray.Rotation = value;

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
                case SelectionObjectMode.SceneObject:
                {
                    return SceneObject.Scale;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
                    return Vector3.One;
                }
                }

                return Vector3.Zero;
            }
            set
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.SceneObject:
                {
                    SceneObject.Scale = value;

                    break;
                }
                case SelectionObjectMode.SceneObjectArray:
                {
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
        public static bool SelectionContains(SceneObjectArray a_arr)
        {
            foreach (SelectionObject obj in s_selection)
            {
                if (obj.SceneObjectArray == a_arr)
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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.