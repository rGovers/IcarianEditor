using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(GameObjectDef))]
    public class GameObjectPropertiesWindow : PropertiesWindow
    {
        enum RotationMode
        {
            Euler,
            AxisAngle,
            Quaternion
        };

        GameObjectDef m_lastDef = null;

        Vector3       m_euler;
        Vector4       m_axisAngle;

        RotationMode  m_mode = RotationMode.AxisAngle;

        public override void OnGUI(object a_object)
        {
            GameObjectDef def = a_object as GameObjectDef;
            if (def == null)
            {
                return;
            }

            GUI.EnumField("Rotation Mode", ref m_mode);

            GUI.RVec3Field("Translation", ref def.Translation, Vector3.Zero);

            if (m_lastDef != def)
            {
                m_euler = def.Rotation.ToEuler();
                m_axisAngle = def.Rotation.ToAxisAngle();

                m_lastDef = def;
            }

            switch (m_mode)
            {
            case RotationMode.Euler:
            {
                if (GUI.RVec3Field("Euler", ref m_euler, Vector3.Zero))
                {
                    def.Rotation = Quaternion.FromEuler(m_euler);

                    m_axisAngle = def.Rotation.ToAxisAngle();
                }

                break;
            }
            case RotationMode.AxisAngle:
            {
                if (GUI.RVec4Field("Axis Angle", ref m_axisAngle, Vector4.UnitY))
                {
                    float mag = m_axisAngle.Magnitude;
                    if (mag > 0)
                    {
                        def.Rotation = Quaternion.FromAxisAngle(m_axisAngle.XYZ / mag, m_axisAngle.W);
                    }
                    else
                    {
                        def.Rotation = Quaternion.Identity;
                    }

                    m_euler = def.Rotation.ToEuler();
                }

                break;
            }
            case RotationMode.Quaternion:
            {
                Vector4 rot = def.Rotation.ToVector4();

                if (GUI.RVec4Field("Quaternion", ref rot, Vector4.UnitW))
                {
                    def.Rotation = rot.ToQuaternion();

                    m_euler = def.Rotation.ToEuler();
                    m_axisAngle = def.Rotation.ToAxisAngle();
                }

                break;
            }
            }

            GUI.Vec3Field("Scale", ref def.Scale);

            bool addValue;
            bool show = GUI.ArrayView("Components", out addValue);

            if (addValue)
            {
                if (def.Components == null)
                {
                    def.Components = new List<ComponentDef>();
                }

                def.Components.Add(null);
            }

            if (show && def.Components != null)
            {
                GUI.Indent();
                GUI.PushID("ObjectComponents");

                int count = def.Components.Count;
                for (int i = 0; i < count; ++i)
                {
                    ComponentDef comp = def.Components[i];

                    // Allow inline editing for scene defs
                    if (comp.IsSceneDef)
                    {
                        GUI.PushID($"[{i}]");
                    
                        if (GUI.StructView($"[{i}] Scene Component"))
                        {
                            BaseGUI(comp);
                        }

                        GUI.PopID();
                    }
                    else
                    {
                        if (GUI.DefField<ComponentDef>($"[{i}]", ref comp))
                        {
                            def.Components[i] = comp;
                        }
                    }
                }

                GUI.PopID();
                GUI.Unindent();
            }

            if (!def.IsSceneDef)
            {
                show = GUI.ArrayView("Children", out addValue);

                if (addValue)
                {
                    if (def.Children == null)
                    {
                        def.Children = new List<GameObjectDef>();
                    }

                    def.Children.Add(null);
                }

                if (show && def.Children != null)
                {
                    GUI.Indent();
                    GUI.PushID("ObjectChildren");

                    int count = def.Children.Count;
                    for (int i = 0; i < count; ++i)
                    {
                        GameObjectDef child = def.Children[i];

                        if (GUI.DefField<GameObjectDef>($"[{i}]", ref child))
                        {
                            def.Children[i] = child;
                        }
                    }

                    GUI.PopID();
                    GUI.Unindent();
                }
            }
        }
    }
}