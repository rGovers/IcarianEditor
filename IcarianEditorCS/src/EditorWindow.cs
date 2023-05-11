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

            RenderComponents(a_gameObjectDef, true, mat);

            foreach (GameObjectDef c in a_gameObjectDef.Children)
            {
                RenderGameObjects(c, mat);
            }
        }

        static void OnGUI()
        {
            Scene scene = Workspace.GetScene();

            if (scene == null)
            {
                return;
            }

            if (Workspace.Selection != null)
            {
                int selectionSize = Workspace.Selection.Count;
                if (selectionSize > 0)
                {
                    Vector3 mid = Vector3.Zero;

                    foreach (SelectionObject sel in Workspace.Selection)
                    {
                        mid += sel.Translation;
                    }

                    mid /= selectionSize;

                    Quaternion rot = Quaternion.Identity;
                    Vector3 scale = Vector3.One;

                    if (!Gizmos.IsManipulating)
                    {
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
                    }

                    // TODO: Account for world space
                    if (Gizmos.Manipulation(Workspace.ManipulationMode, ref mid, ref rot, ref scale))
                    {
                        Vector3 deltaPos = mid - s_startPos;

                        for (int i = 0; i < selectionSize; ++i)
                        {
                            TransformData dat = s_startData[i];
                            Vector3 objDeltaPos = dat.StartPos - s_startPos;

                            SelectionObject sel = Workspace.Selection[i];

                            sel.Translation = objDeltaPos + deltaPos;
                            sel.Rotation = dat.StartRotation * rot;
                            sel.Scale = dat.StartScale * scale;

                            Workspace.Selection[i] = sel;
                        }
                    }
                }
            }

            foreach (SceneObject obj in scene.SceneObjects)
            {
                GameObjectDef def = DefLibrary.GetDef(obj.DefName) as GameObjectDef;

                Matrix4 mat = Matrix4.FromTransform(obj.Translation, obj.Rotation, obj.Scale);

                if (def != null)
                {
                    RenderGameObjects(def, mat);
                }
            }
        }
    }
}