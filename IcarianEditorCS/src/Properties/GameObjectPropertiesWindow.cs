// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEditor.Modals;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(GameObjectDef))]
    public class GameObjectPropertiesWindow : PropertiesEditorWindow
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

        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            GameObjectDef def = a_object as GameObjectDef;
            if (def == null)
            {
                return;
            }

            GUI.StringField("Name", ref def.Name);

            GUI.RVec3Field("Translation", ref def.Translation, Vector3.Zero);
            
            GUI.EnumField("Rotation Mode", ref m_mode);

            if (m_lastDef != def)
            {
                Quaternion rotation = def.Rotation;
                if (rotation == Quaternion.Identity)
                {
                    m_euler = Vector3.Zero;
                    m_axisAngle = Vector4.UnitY;
                }
                else
                {
                    m_euler = rotation.ToEuler();
                    m_axisAngle = rotation.ToAxisAngle();
                }

                m_lastDef = def;
            }

            switch (m_mode)
            {
            case RotationMode.Euler:
            {
                if (EditorConfig.UseDegrees)
                {
                    Vector3 euler = m_euler * Mathf.RadToDeg;
                    if (GUI.RVec3Field("Euler", ref euler, Vector3.Zero))
                    {
                        m_euler = euler * Mathf.DegToRad;
                        def.Rotation = Quaternion.FromEuler(m_euler);

                        m_axisAngle = def.Rotation.ToAxisAngle();
                    }
                }
                else
                {
                    if (GUI.RVec3Field("Euler", ref m_euler, Vector3.Zero))
                    {
                        def.Rotation = Quaternion.FromEuler(m_euler);

                        m_axisAngle = def.Rotation.ToAxisAngle();
                    }
                }

                break;
            }
            case RotationMode.AxisAngle:
            {
                if (EditorConfig.UseDegrees)
                {
                    Vector4 axisAngle = new Vector4(m_axisAngle.XYZ, m_axisAngle.W * Mathf.RadToDeg);
                    if (GUI.RVec4Field("Axis Angle", ref axisAngle, Vector4.UnitY))
                    {
                        m_axisAngle = new Vector4(axisAngle.XYZ, axisAngle.W * Mathf.DegToRad);

                        float mag = m_axisAngle.XYZ.Magnitude;
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
                }
                else
                {
                    if (GUI.RVec4Field("Axis Angle", ref m_axisAngle, Vector4.UnitY))
                    {
                        float mag = m_axisAngle.XYZ.Magnitude;
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

                new AddComponentModal(def);
            }

            if (show && def.Components != null)
            {
                GUI.Indent();
                GUI.PushID("ObjectComponents");

                int count = def.Components.Count;
                for (int i = 0; i < count; ++i)
                {
                    ComponentDef comp = def.Components[i];

                    GUI.PushID($"[{i}]");

                    if (GUI.Button("-"))
                    {
                        def.Components.RemoveAt(i);
                        --i;
                        --count;

                        GUI.PopID();

                        continue;
                    }

                    GUI.SameLine();

                    if (GUI.DefField<ComponentDef>($"[{i}]", ref comp))
                    {
                        def.Components[i] = comp;
                    }

                    GUI.PopID();
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