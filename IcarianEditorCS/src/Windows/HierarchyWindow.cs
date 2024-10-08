// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEditor;
using IcarianEditor.Modals;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEditor.Windows
{
    public static class HierarchyWindow
    {
        static ulong s_startID = uint.MaxValue;

        static void Select(ulong a_id, List<SelectionObject> a_selections)
        {
            List<SelectionObject> selection = new List<SelectionObject>();

            if (GUI.CtrlModifier)
            {
                selection.AddRange(Workspace.Selection);
            }

            int index = 0;
            int sIndex = int.MaxValue;
            for (int i = 0; i < a_selections.Count; ++i)
            {
                SelectionObject obj = a_selections[i];

                ulong id = obj.ID;

                if (id == s_startID)
                {
                    sIndex = i;
                }

                if (id == a_id)
                {
                    if (!selection.Contains(obj))
                    {
                        selection.Add(obj);
                    }

                    index = i;

                    break;
                }
            }

            if (GUI.ShiftModifier)
            {
                if (index < sIndex)
                {
                    for (int i = index + 1; i < a_selections.Count; ++i)
                    {
                        SelectionObject obj = a_selections[i];

                        if (!selection.Contains(obj))
                        {
                            selection.Add(obj);
                        }

                        if (obj.ID == s_startID)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    for (int i = index - 1; i >= sIndex; --i)
                    {
                        SelectionObject obj = a_selections[i];

                        if (!selection.Contains(obj))
                        {
                            selection.Add(obj);
                        }
                    }
                }
            }

            Workspace.ClearSelection();
            Workspace.AddSelection(selection);

            s_startID = a_id;
        }

        static void CreateMenuItem()
        {
            if (GUI.MenuItem("New Scene Object"))
            {
                new NewSceneObjectModal();
            }

            if (GUI.MenuItem("New Scene Object Array"))
            {
                new NewSceneObjectModal(true);
            }

            GUI.Separator();

            ClipBoardItem item;
            if (ClipBoard.GetItem(new ClipBoardItemType[] { ClipBoardItemType.SceneObject }, out item))
            {
                if (GUI.MenuItem("Paste"))
                {
                    EditorScene scene = Workspace.GetScene();

                    scene.AddSceneObject((string)item.Data);
                }
            }
        }

        static void ObjectOptionsMenu(ulong a_id)
        {
            if (GUI.MenuItem("Copy"))
            {
                EditorScene scene = Workspace.GetScene();

                SceneObject sObject = scene.GetSceneObject(a_id);
                ClipBoard.AddItem(ClipBoardItemType.SceneObject, sObject.DefName);
            }

            GUI.Separator();

            if (GUI.MenuItem("Delete"))
            {
                new DeleteSceneObjectModal(a_id);
            }
        }

        static void OnGUI()
        {
            EditorScene scene = Workspace.GetScene();

            if (scene == null)
            {
                return;
            }

            // Probably not the best way of doing this but it works
            List<SelectionObject> selectionList = new List<SelectionObject>();
            ulong selectionID = ulong.MaxValue;

            bool context = false;

            IEnumerable<SceneObjectData> sceneObjects = scene.SceneObjects;
            foreach (SceneObjectData objectData in sceneObjects)
            {
                SceneObject obj = objectData.Object;

                string name = obj.DefName;

                GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(name);
                if (def == null)
                {
                    continue;
                }

                ulong id = objectData.ID;
                string idStr = $"##[{id}]{obj.DefName}";

                selectionList.Add(new SelectionObject()
                {
                    ID = id,
                    SelectionMode = SelectionObjectMode.SceneObject,
                    SceneObject = obj
                });

                bool visible = objectData.Visible;
                if (GUI.ToggleButton($"Visible{idStr}", "Textures/Icons/Hierarchy_Visible.png", "Textures/Icons/Hierarchy_Hidden.png", ref visible, new Vector2(12.0f), false))
                {
                    scene.SetVisible(id, visible);

                    // Have to break as C# does not allow modifying the value while iterating cause C# is a "good" language
                    // Fuck C# and this fucky hack
                    // Why the fuck C# iterates by value instead of by reference who tf knows
                    // Newer versions have ref loops but incompatible with the editor specifically
                    // Should do properly but it works
                    break;
                }
                GUI.Tooltip("Visibility", "Toggles the visibility of the Scene Object");

                GUI.SameLine();

                GUI.Texture("Textures/Icons/Hierarchy_SceneGameObject.png", new Vector2(16.0f));

                GUI.SameLine();

                if (GUI.Selectable($"{name}{idStr}"))
                {
                    selectionID = id;
                }

                if (!context && GUI.BeginContextPopup())
                {
                    context = true;

                    CreateMenuItem();
                    ObjectOptionsMenu(id);

                    GUI.EndPopup();
                }
            }

            GUI.Separator();

            IEnumerable<SceneObjectArrayData> sceneObjectArrays = scene.SceneObjectArrays;
            foreach (SceneObjectArrayData arrayData in sceneObjectArrays)
            {
                SceneObjectArray arr = arrayData.Array;

                string name = arr.DefName;
                GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(name);
                if (def == null)
                {
                    continue;
                }

                ulong id = arrayData.ID;
                string idStr = $"##A[{id}]{arr.DefName}";

                selectionList.Add(new SelectionObject()
                {
                    ID = id,
                    SelectionMode = SelectionObjectMode.SceneObjectArray,
                    SceneObjectArray = arr
                });

                bool visible = arrayData.Visible;
                if (GUI.ToggleButton($"Visible{idStr}", "Textures/Icons/Hierarchy_Visible.png", "Textures/Icons/Hierarchy_Hidden.png", ref visible, new Vector2(12.0f), false))
                {
                    scene.SetVisible(id, visible);

                    break;
                }

                GUI.SameLine();

                if (GUI.Selectable($"{name}{idStr}"))
                {
                    selectionID = id;
                }

                if (!context && GUI.BeginContextPopup())
                {
                    context = true;

                    CreateMenuItem();

                    GUI.EndPopup();
                }
            }

            if (selectionID != ulong.MaxValue)
            {
                Select(selectionID, selectionList);
            }

            if (!context && GUI.BeginContextPopupWindow())
            {
                context = true;
                
                CreateMenuItem();

                GUI.EndPopup();
            }
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