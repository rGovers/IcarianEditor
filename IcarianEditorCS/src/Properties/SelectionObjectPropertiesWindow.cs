using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(SelectionObject))]
    public class SelectionObjectPropertiesWindow : PropertiesWindow
    {
        enum RotationMode
        {
            Euler,
            AxisAngle,
            Quaternion
        };

        object       m_lastObject = null;

        Vector3      m_euler;
        Vector4      m_axisAngle;

        RotationMode m_mode = RotationMode.AxisAngle;

        static GameObjectPropertiesWindow s_gameObjectWindow = new GameObjectPropertiesWindow();

        public override void OnGUI(object a_object)
        {
            SelectionObject selectionObject = (SelectionObject)a_object;

            switch (selectionObject.SelectionMode)
            {
            case SelectionObjectMode.SceneObject:
            {
                GUI.EnumField("Rotation Mode", ref m_mode);

                if (m_lastObject != selectionObject.SceneObject)
                {
                    Quaternion rotation = selectionObject.SceneObject.Rotation;
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
                    
                    m_lastObject = selectionObject.SceneObject;
                }

                GUI.RVec3Field("Translation", ref selectionObject.SceneObject.Translation, Vector3.Zero);
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
                            selectionObject.SceneObject.Rotation = Quaternion.FromEuler(m_euler);

                            m_axisAngle = selectionObject.SceneObject.Rotation.ToAxisAngle();
                        }
                    }
                    else
                    {
                        if (GUI.RVec3Field("Euler", ref m_euler, Vector3.Zero))
                        {
                            selectionObject.SceneObject.Rotation = Quaternion.FromEuler(m_euler);

                            m_axisAngle = selectionObject.SceneObject.Rotation.ToAxisAngle();
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

                            float mag = m_axisAngle.Magnitude;
                            if (mag > 0)
                            {
                                selectionObject.SceneObject.Rotation = Quaternion.FromAxisAngle(m_axisAngle.XYZ / mag, m_axisAngle.W);
                            }
                            else
                            {
                                selectionObject.SceneObject.Rotation = Quaternion.Identity;
                            }

                            m_euler = selectionObject.SceneObject.Rotation.ToEuler();
                        }
                    }
                    else
                    {
                        if (GUI.RVec4Field("Axis Angle", ref m_axisAngle, Vector4.UnitY))
                        {
                            float mag = m_axisAngle.Magnitude;
                            if (mag > 0)
                            {
                                selectionObject.SceneObject.Rotation = Quaternion.FromAxisAngle(m_axisAngle.XYZ / mag, m_axisAngle.W);
                            }
                            else
                            {
                                selectionObject.SceneObject.Rotation = Quaternion.Identity;
                            }

                            m_euler = selectionObject.SceneObject.Rotation.ToEuler();
                        }
                    }

                    break;
                }
                case RotationMode.Quaternion:
                {
                    if (GUI.RQuaternionField("Quaternion", ref selectionObject.SceneObject.Rotation, Quaternion.Identity))
                    {
                        m_euler = selectionObject.SceneObject.Rotation.ToEuler();
                        m_axisAngle = selectionObject.SceneObject.Rotation.ToAxisAngle();
                    }

                    break;
                }
                }

                GUI.RVec3Field("Scale", ref selectionObject.SceneObject.Scale, Vector3.One);

                // GUI.StringField("DefName", ref selectionObject.SceneObject.DefName);

                break;
            }
            case SelectionObjectMode.GameObjectDef:
            {
                GameObjectDef def = selectionObject.GameObject;
                // Allow inline editing for scene objects
                if (def != null && def.IsSceneDef)
                {
                    s_gameObjectWindow.OnGUI(def);  
                }

                break;
            }
            }

            a_object = selectionObject;
        }
    }
}