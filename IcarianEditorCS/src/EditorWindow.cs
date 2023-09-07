using IcarianEditor.Editor;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor
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
                    EDisplayAttribute att = t.GetCustomAttribute<EDisplayAttribute>();
                    if (att != null)
                    {
                        if (s_componentLookup.ContainsKey(att.OverrideType))
                        {
                            Logger.Error($"EditorDisplay already exists: {t}, {att.OverrideType}");

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
                Type t = c.ComponentType;
                if (s_componentLookup.ContainsKey(t))
                {
                    EditorDisplay disp = s_componentLookup[t];
                    if (disp != null)
                    {
                        disp.Render(a_selected, c, a_transform);
                    }
                    else
                    {
                        Logger.Error($"IcarianEditorCS: Null editor display {t}");
                    }
                }
            }
        }

        static void RenderGameObjects(GameObjectDef a_gameObjectDef, Matrix4 a_parentTransform)
        {
            Matrix4 mat = Matrix4.FromTransform(a_gameObjectDef.Translation, a_gameObjectDef.Rotation, a_gameObjectDef.Scale) * a_parentTransform;

            bool selected = false;
            if (Workspace.Selection != null)
            {
                foreach (SelectionObject obj in Workspace.Selection)
                {
                    if (obj.GameObject == null)
                    {
                        continue;
                    }

                    if (obj.GameObject.DefName == a_gameObjectDef.DefName)
                    {
                        selected = true;

                        break;
                    }
                }
            }
            
            RenderComponents(a_gameObjectDef, selected, mat);

            foreach (GameObjectDef c in a_gameObjectDef.Children)
            {
                RenderGameObjects(c, mat);
            }
        }

        static void BeginSelectGameObjects(GameObjectDef a_gameObjectDef, Matrix4 a_parentTransform, ref Vector3 a_mid, ref uint a_count)
        {
            Matrix4 mat = Matrix4.FromTransform(a_gameObjectDef.Translation, a_gameObjectDef.Rotation, a_gameObjectDef.Scale) * a_parentTransform;

            bool selected = false;
            foreach (SelectionObject obj in Workspace.Selection)
            {
                if (obj.GameObject == null)
                {
                    continue;
                }

                if (obj.GameObject.DefName == a_gameObjectDef.DefName)
                {
                    selected = true;

                    break;
                }
            }

            if (selected)
            {
                a_mid += mat[3].XYZ;
                a_count++;
            }

            foreach (GameObjectDef c in a_gameObjectDef.Children)
            {
                BeginSelectGameObjects(c, mat, ref a_mid, ref a_count);
            }
        }

        static void RenderScene(Scene a_scene)
        {
            foreach (SceneObject obj in a_scene.SceneObjects)
            {
                GameObjectDef def = DefLibrary.GetDef<GameObjectDef>(obj.DefName);

                if (def != null)
                {
                    Matrix4 mat = Matrix4.FromTransform(obj.Translation, obj.Rotation, obj.Scale);

                    RenderGameObjects(def, mat);
                }
            }
        }

        static void OnGUI()
        {
            Scene scene = Workspace.GetScene();

            if (scene == null)
            {
                return;
            }

            if (Workspace.Selection != null && Workspace.Selection.Count > 0)
            {
                bool isManipulating = Gizmos.IsManipulating;

                if (!isManipulating)
                {
                    Vector3 mid = Vector3.Zero;
                    uint count = 0;

                    foreach (SceneObject obj in scene.SceneObjects)
                    {
                        GameObjectDef def = DefLibrary.GetDef<GameObjectDef>(obj.DefName);

                        foreach (SelectionObject selectionObj in Workspace.Selection)
                        {
                            if (selectionObj.SceneObject == obj)
                            {
                                mid += obj.Translation;
                                count++;

                                break;
                            }
                        }

                        if (def != null)
                        {
                            Matrix4 mat = Matrix4.FromTransform(obj.Translation, obj.Rotation, obj.Scale);

                            BeginSelectGameObjects(def, mat, ref mid, ref count);
                            RenderGameObjects(def, mat);
                        }
                    }

                    mid /= count;

                    s_startData.Clear();

                    s_startPos = mid;

                    foreach (SelectionObject sel in Workspace.Selection)
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

                    for (int i = 0; i < Workspace.Selection.Count; ++i)
                    {
                        TransformData dat = s_startData[i];
                        // Vector3 objDeltaPos = dat.StartPos - s_startPos;

                        SelectionObject sel = Workspace.Selection[i];

                        sel.Translation = dat.StartPos + deltaPos;
                        sel.Rotation = dat.StartRotation * s_rotation;
                        sel.Scale = dat.StartScale + deltaScale;

                        Workspace.Selection[i] = sel;
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