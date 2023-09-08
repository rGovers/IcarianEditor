using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace IcarianEditor
{
    public struct GameObjectData
    {
        public ulong ID;
        public GameObjectDef Def;
    }
    public struct SceneObjectData
    {
        public ulong ID;
        public SceneObject Object;
    }

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
        public GameObjectDef GameObject;

        public Vector3 Translation
        {
            get
            {
                switch (SelectionMode)
                {
                case SelectionObjectMode.GameObjectDef:
                {
                    return GameObject.Translation;
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
                    GameObject.Translation = value;

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
                    return GameObject.Rotation;
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
                    GameObject.Rotation = value;

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
                    return GameObject.Scale;
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
                    GameObject.Scale = value;

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
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetCurrentScene();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetCurrentScene(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetManipulationMode();

        static ulong                 s_ID = 0;

        static Scene                 s_LastScene = null;

        public static List<Def> SceneDefs = new List<Def>();
        static List<SelectionObject> s_Selection = new List<SelectionObject>();
        static List<Def> s_SelectedDefs = new List<Def>();
        public static List<SceneObjectData> SceneObjectList = new List<SceneObjectData>();
        public static List<GameObjectData>  SceneGameObjectList = new List<GameObjectData>();

        public static IEnumerable<SelectionObject> Selection
        {
            get
            {
                return s_Selection;
            }
        }
        public static IEnumerable<Def> SelectedDefs
        {
            get
            {
                return s_SelectedDefs;
            }
        }

        public static bool IsSelectionEmpty
        {
            get
            {
                return s_Selection.Count == 0;
            }
        }
        public static bool IsDefSelectionEmpty
        {
            get
            {
                return s_SelectedDefs.Count == 0;
            }
        }

        public static void AddSelection(SelectionObject a_object)
        {
            s_Selection.Add(a_object);
        }
        public static void AddSelection(IEnumerable<SelectionObject> a_objects)
        {
            s_Selection.AddRange(a_objects);
        }
        public static void AddDefSelection(Def a_object)
        {
            s_SelectedDefs.Add(a_object);
        }
        public static void AddDefSelection(IEnumerable<Def> a_objects)
        {
            s_SelectedDefs.AddRange(a_objects);
        }

        public static void ClearSelection()
        {
            s_Selection.Clear();
            s_SelectedDefs.Clear();
        }

        public static ulong NewID()
        {
            return s_ID++;
        }

        public static string CurrentScenePath
        {
            get
            {
                return GetCurrentScene();
            }
            set
            {
                SetCurrentScene(value);
            }
        }

        public static ManipulationMode ManipulationMode
        {
            get
            {
                return (ManipulationMode)GetManipulationMode();
            }
        }

        public static ulong GetID(GameObjectDef a_object)
        {
            foreach (GameObjectData dat in SceneGameObjectList)
            {
                if (dat.Def == a_object)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }
        public static ulong GetID(SceneObject a_object)
        {
            foreach (SceneObjectData dat in SceneObjectList)
            {
                if (dat.Object == a_object)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }

        static void GetObjects(GameObjectDef a_def)
        {
            if (a_def == null)
            {
                return;
            }

            SceneGameObjectList.Add(new GameObjectData()
            {
                ID = NewID(),
                Def = a_def
            });

            foreach (GameObjectDef child in a_def.Children)
            {
                GetObjects(child);
            }
        }

        public static Scene GetScene()
        {
            string curScene = GetCurrentScene();
            if (string.IsNullOrEmpty(curScene))
            {
                return null;
            }

            Scene s = SceneData.GetScene(curScene);
            if (s == null)
            {
                SetCurrentScene(string.Empty);

                return null;
            }

            if (s != s_LastScene)
            {
                s_Selection.Clear();
                s_SelectedDefs.Clear();

                SceneDefs.Clear();
                SceneObjectList.Clear();
                SceneGameObjectList.Clear();

                foreach (Def def in s.Defs)
                {
                    SceneDefs.Add(def);
                }

                foreach (SceneObject obj in s.SceneObjects)
                {
                    SceneObjectList.Add(new SceneObjectData()
                    {
                        ID = NewID(),
                        Object = obj
                    });

                    GetObjects(DefLibrary.GetDef<GameObjectDef>(obj.DefName));
                }

                s_LastScene = s;
            }

            return s;
        }
    }
}