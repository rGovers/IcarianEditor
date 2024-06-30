using IcarianEditor.Editor;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Windows
{
    struct TransformData
    {
        public Vector3 StartPos;
        public Quaternion StartRotation;
        public Vector3 StartScale;
    }

    public static class EditorWindow
    {
        static Dictionary<Type, EditorDisplay> s_componentLookup;

        static List<TransformData>             s_startData;
        static Vector3                         s_startPos;
        static Vector3                         s_mid;
        static Quaternion                      s_rotation;
        static Vector3                         s_scale;

        internal static void Init()
        {
            s_componentLookup = new Dictionary<Type, EditorDisplay>();
            s_startData = new List<TransformData>();

            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (Assembly asm in assemblies)
            {
                Type[] types = asm.GetTypes();
                foreach (Type t in types)
                {
                    EDisplayAttribute att = null;

                    foreach (CustomAttributeData a in t.CustomAttributes)
                    {
                        if (a.AttributeType == typeof(EDisplayAttribute))
                        {
                            att = a.Constructor.Invoke(new object[] { a.ConstructorArguments[0].Value }) as EDisplayAttribute;

                            break;
                        }
                    }
                    // Crashes on Windows so we'll use the above method
                    // C# is portable btw
                    // EDisplayAttribute att = t.GetCustomAttribute<EDisplayAttribute>();
                    
                    if (att != null)
                    {
                        if (s_componentLookup.ContainsKey(att.OverrideType))
                        {
                            Logger.Warning($"EditorDisplay already exists: {t}, {att.OverrideType}");

                            continue;
                        }

                        s_componentLookup.Add(att.OverrideType, Activator.CreateInstance(t) as EditorDisplay);
                    }
                }
            }
        }

        static void RenderComponents(GameObjectDef a_def, bool a_selected, Matrix4 a_transform)
        {
            foreach (ComponentDef c in a_def.Components)
            {
                ComponentDef component = EditorDefLibrary.GenerateDef<ComponentDef>(c.DefName);

                Type t = component.ComponentType;
                if (s_componentLookup.ContainsKey(t))
                {
                    EditorDisplay disp = s_componentLookup[t];
                    if (disp != null)
                    {
                        disp.Render(a_selected, component, a_transform);
                    }
                    else
                    {
                        Logger.Error($"IcarianEditorCS: Null editor display {t}");
                    }
                }
            }
        }

        static void RenderGameObjects(bool a_selected, GameObjectDef a_gameObjectDef, Matrix4 a_parentTransform)
        {
            Matrix4 mat = Matrix4.FromTransform(a_gameObjectDef.Translation, a_gameObjectDef.Rotation, a_gameObjectDef.Scale) * a_parentTransform;

            RenderComponents(a_gameObjectDef, a_selected, mat);

            foreach (GameObjectDef c in a_gameObjectDef.Children)
            {
                GameObjectDef gameObject = EditorDefLibrary.GenerateDef<GameObjectDef>(c.DefName);
                if (gameObject == null)
                {
                    continue;
                }

                RenderGameObjects(a_selected, gameObject, mat);
            }
        }
        static void RenderArray(SceneObjectArray a_array)
        {
            GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(a_array.DefName);
            if (def == null)
            {
                return;
            }

            bool selected = Workspace.SelectionContains(a_array);
            Matrix4 rotMat = a_array.Rotation.ToMatrix();

            for (int x = 0; x < a_array.Count.X; ++x)
            {
                for (int y = 0; y < a_array.Count.Y; ++y)
                {
                    for (int z = 0; z < a_array.Count.Z; ++z)
                    {
                        Vector3 pos = new Vector3((float)x, (float)y, (float)z) * a_array.Spacing;
                        Matrix4 transMat = new Matrix4
                        (
                            Vector4.UnitX,
                            Vector4.UnitY,
                            Vector4.UnitZ,
                            new Vector4(a_array.Translation + a_array.Rotation * pos, 1.0f)
                        );

                        Matrix4 mat = rotMat * transMat;

                        RenderGameObjects(selected, def, mat);
                    }
                }
            }
        }
        static void RenderScene(EditorScene a_scene)
        {
            IEnumerable<SceneObjectData> sceneObjects = a_scene.SceneObjects;
            foreach (SceneObjectData objData in sceneObjects)
            {
                if (!objData.Visible)
                {
                    continue;
                }

                SceneObject obj = objData.Object;
                GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(obj.DefName);
                if (def != null)
                {
                    bool selected = Workspace.SelectionContains(obj);
                    Matrix4 mat = Matrix4.FromTransform(obj.Translation, obj.Rotation, obj.Scale);

                    RenderGameObjects(selected, def, mat);
                }
            }

            IEnumerable<SceneObjectArrayData> arrayObjects = a_scene.SceneObjectArrays;
            foreach (SceneObjectArrayData arrData in arrayObjects)
            {
                if (!arrData.Visible)
                {
                    continue;
                }

                RenderArray(arrData.Array);
            }
        }

        static void PeekDefPath(string a_path, Vector3 a_editorPos)
        {
            GameObjectDef def = EditorDefLibrary.GeneratePathDef<GameObjectDef>(a_path);
            if (def != null)
            {
                Matrix4 transform = new Matrix4(Vector4.UnitX, Vector4.UnitY, Vector4.UnitZ, new Vector4(a_editorPos, 1.0f));
                RenderComponents(def, true, transform);
            }

            Gizmos.DrawIcoSphere(a_editorPos, 0.025f, 1, 0.001f, Color.White);
        }
        static void AcceptDefPath(string a_path, Vector3 a_editorPos)
        {
            EditorScene scene = Workspace.GetScene();
            if (scene == null)
            {
                return;
            }

            GameObjectDef def = EditorDefLibrary.GeneratePathDef<GameObjectDef>(a_path);
            if (def != null)
            {
                scene.AddSceneObject(def.DefName, a_editorPos, Quaternion.Identity, Vector3.One);
            }
        }

        static void OnGUI()
        {
            EditorScene scene = Workspace.GetScene();
            if (scene == null)
            {
                return;
            }

            if (!Workspace.IsSelectionEmpty)
            {
                if (!Gizmos.IsManipulating)
                {
                    Vector3 mid = Vector3.Zero;
                    uint count = 0;

                    IEnumerable<SelectionObject> selection = Workspace.Selection;

                    IEnumerable<SceneObjectData> sceneObjects = scene.SceneObjects;
                    foreach (SceneObjectData objData in sceneObjects)
                    {
                        if (!objData.Visible)
                        {
                            continue;
                        }

                        SceneObject obj = objData.Object;
                        GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(obj.DefName);
                        if (def == null)
                        {
                            continue;
                        }

                        bool selected = false;
                        foreach (SelectionObject selectionObj in selection)
                        {
                            if (selectionObj.SelectionMode == SelectionObjectMode.SceneObject && selectionObj.SceneObject == obj)
                            {
                                selected = true;
                                mid += obj.Translation;
                                ++count;

                                break;
                            }
                        }

                        Matrix4 mat = Matrix4.FromTransform(obj.Translation, obj.Rotation, obj.Scale);

                        RenderGameObjects(selected, def, mat);
                    }

                    IEnumerable<SceneObjectArrayData> sceneArrays = scene.SceneObjectArrays;
                    foreach (SceneObjectArrayData arrData in sceneArrays)
                    {
                        if (!arrData.Visible)
                        {
                            continue;
                        }

                        SceneObjectArray arr = arrData.Array;
                        GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(arr.DefName);
                        if (def == null)
                        {
                            continue;
                        }

                        foreach (SelectionObject selectionObj in selection)
                        {
                            if (selectionObj.SelectionMode == SelectionObjectMode.SceneObjectArray && selectionObj.SceneObjectArray == arr)
                            {
                                mid += arr.Translation;
                                ++count;

                                break;
                            }
                        }

                        RenderArray(arr);
                    }

                    if (count <= 0)
                    {
                        return;
                    }

                    if (mid != Vector3.Zero)
                    {
                        mid /= count;
                    }

                    s_startData.Clear();

                    s_startPos = mid;

                    foreach (SelectionObject sel in selection)
                    {
                        s_startData.Add(new TransformData()
                        {
                            StartPos = sel.Translation,
                            StartRotation = sel.Rotation,
                            StartScale = sel.Scale
                        });
                    }

                    s_mid = mid;
                    s_rotation = Quaternion.Identity;
                    s_scale = Vector3.One;
                }
                else
                {
                    RenderScene(scene);
                }

                if (Gizmos.Manipulation(Workspace.ManipulationMode, ref s_mid, ref s_rotation, ref s_scale))
                {
                    Vector3 deltaPos = s_mid - s_startPos;
                    Vector3 deltaScale = s_scale - Vector3.One;

                    List<SelectionObject> selection = new List<SelectionObject>();
                    selection.AddRange(Workspace.Selection);

                    int count = selection.Count;
                    for (int i = 0; i < count; ++i)
                    {
                        SelectionObject sel = selection[i];
                        TransformData dat = s_startData[i];

                        sel.Translation = dat.StartPos + deltaPos;
                        sel.Rotation = dat.StartRotation * s_rotation;
                        sel.Scale = dat.StartScale + deltaScale;
                    }
                }
            }
            else
            {
                RenderScene(scene);    
            }
        }
    }
}