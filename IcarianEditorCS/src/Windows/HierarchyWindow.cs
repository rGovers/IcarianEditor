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

                GUI.PushID(idStr);

                selectionList.Add(new SelectionObject()
                {
                    ID = id,
                    SelectionMode = SelectionObjectMode.SceneObject,
                    SceneObject = obj
                });

                if (GUI.Selectable(name))
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

                GUI.PopID();
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