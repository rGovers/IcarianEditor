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
        public SceneObject Object;
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

        public static List<Def> SceneDefs = new List<Def>();

        static ulong                 s_id = 0;

        static Scene                 s_lastScene = null;

        static List<SelectionObject> s_selection = new List<SelectionObject>();
        static List<Def>             s_selectedDefs = new List<Def>();

        static List<SceneObjectData> s_sceneObjectList = new List<SceneObjectData>();
        static List<GameObjectData>  s_sceneGameObjectList = new List<GameObjectData>();
        
        public static IEnumerable<SelectionObject> Selection
        {
            get
            {
                return s_selection;
            }
        }
        public static IEnumerable<Def> SelectedDefs
        {
            get
            {
                return s_selectedDefs;
            }
        }

        public static IEnumerable<SceneObjectData> SceneObjectList
        {
            get
            {
                return s_sceneObjectList;
            }
        }
        public static IEnumerable<GameObjectData> SceneGameObjectList
        {
            get
            {
                return s_sceneGameObjectList;
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

        public static void AddSelection(SelectionObject a_object)
        {
            s_selection.Add(a_object);
        }
        public static void AddSelection(IEnumerable<SelectionObject> a_objects)
        {
            s_selection.AddRange(a_objects);
        }
        public static void AddDefSelection(Def a_object)
        {
            s_selectedDefs.Add(a_object);
        }
        public static void AddDefSelection(IEnumerable<Def> a_objects)
        {
            s_selectedDefs.AddRange(a_objects);
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
        public static bool SelectionContains(SceneObject a_obj, GameObjectDef a_def)
        {
            foreach (SelectionObject obj in s_selection)
            {
                if (obj.SceneObject == a_obj && obj.GameObject == a_def)
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

        public static ulong GetID(SceneObject a_sceneObject, GameObjectDef a_object)
        {
            foreach (GameObjectData dat in s_sceneGameObjectList)
            {
                if (dat.Object == a_sceneObject && dat.Def == a_object)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }
        public static ulong GetID(SceneObject a_object)
        {
            foreach (SceneObjectData dat in s_sceneObjectList)
            {
                if (dat.Object == a_object)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }

        static void GetObjects(SceneObject a_object, GameObjectDef a_def)
        {
            if (a_def == null)
            {
                return;
            }

            s_sceneGameObjectList.Add(new GameObjectData()
            {
                ID = NewID(),
                Object = a_object,
                Def = a_def
            });

            foreach (GameObjectDef child in a_def.Children)
            {
                GetObjects(a_object, child);
            }
        }

        public static void CreateSceneObject(GameObjectDef a_def)
        {
            SceneObject obj = new SceneObject()
            {
                DefName = a_def.DefName,
                Translation = Vector3.Zero,
                Rotation = Quaternion.Identity,
                Scale = Vector3.One
            };
            SceneObjectData data = new SceneObjectData()
            {
                ID = NewID(),
                Object = obj
            };

            s_sceneObjectList.Add(data);

            GetObjects(obj, a_def);
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

            if (s != s_lastScene)
            {
                s_selection.Clear();
                s_selectedDefs.Clear();

                SceneDefs.Clear();
                s_sceneObjectList.Clear();
                s_sceneGameObjectList.Clear();

                foreach (Def def in s.Defs)
                {
                    SceneDefs.Add(def);
                }

                foreach (SceneObject obj in s.SceneObjects)
                {
                    s_sceneObjectList.Add(new SceneObjectData()
                    {
                        ID = NewID(),
                        Object = obj
                    });

                    GameObjectDef def = DefLibrary.GetDef<GameObjectDef>(obj.DefName);

                    GetObjects(obj, def);
                }

                s_lastScene = s;
            }

            return s;
        }
    }
}