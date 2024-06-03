using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(SelectionObject))]
    public class SelectionObjectPropertiesWindow : PropertiesEditorWindow
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

        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            SelectionObject selectionObject = (SelectionObject)a_object;
            object obj = selectionObject.Object;

            GUI.EnumField("Rotation Mode", ref m_mode);

            if (m_lastObject != obj)
            {
                Quaternion rotation = selectionObject.Rotation;
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
                    
                m_lastObject = obj;
            }

            Vector3 translation = selectionObject.Translation;
            if (GUI.RVec3Field("Translation", ref translation))
            {
                selectionObject.Translation = translation;
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

                        Quaternion rot = Quaternion.FromEuler(m_euler);

                        selectionObject.Rotation = rot;
                        m_axisAngle = rot.ToAxisAngle();
                    }

                    break;
                }

                if (GUI.RVec3Field("Euler", ref m_euler, Vector3.Zero))
                {
                    Quaternion rot = Quaternion.FromEuler(m_euler);

                    selectionObject.Rotation = rot;
                    m_axisAngle = rot.ToAxisAngle();
                }

                break;
            }
            case RotationMode.AxisAngle:
            {
                if (EditorConfig.UseDegrees)
                {
                    Vector4 axisAngle = new Vector4(m_axisAngle.XYZ, m_axisAngle.W * Mathf.RadToDeg);
                    if (GUI.RVec4Field("Axis Angle", ref axisAngle))
                    {
                        m_axisAngle = new Vector4(axisAngle.XYZ, axisAngle.W * Mathf.DegToRad);

                        Quaternion rot = Quaternion.Identity;
                        
                        float mag = m_axisAngle.XYZ.Magnitude;
                        if (mag > 0)
                        {
                            rot = Quaternion.FromAxisAngle(m_axisAngle.XYZ / mag, m_axisAngle.W);
                        }

                        selectionObject.Rotation = rot;
                        m_euler = rot.ToEuler();
                    }

                    break;
                }

                if (GUI.RVec4Field("Axis Angle", ref m_axisAngle, Vector4.UnitY))
                {
                    Quaternion rot = Quaternion.Identity;

                    float mag = m_axisAngle.XYZ.Magnitude;
                    if (mag > 0)
                    {
                        rot = Quaternion.FromAxisAngle(m_axisAngle.XYZ / mag, m_axisAngle.W);
                    }

                    selectionObject.Rotation = rot;
                    m_euler = rot.ToEuler();
                }

                break;
            }
            case RotationMode.Quaternion:
            {
                Quaternion quat = selectionObject.Rotation;
                if (GUI.RQuaternionField("Quaternion", ref quat, Quaternion.Identity))
                {
                    m_euler = quat.ToEuler();
                    m_axisAngle = quat.ToAxisAngle();
                    selectionObject.Rotation = quat;
                }

                break;
            }
            }

            switch (selectionObject.SelectionMode)
            {
            case SelectionObjectMode.SceneObject:
            {
                GUI.RVec3Field("Scale", ref selectionObject.SceneObject.Scale, Vector3.One);

                break;
            }
            case SelectionObjectMode.SceneObjectArray:
            {   
                IVector3 count = selectionObject.SceneObjectArray.Count;

                if (GUI.RIntField("X", ref count.X, 1))
                {
                    selectionObject.SceneObjectArray.Count.X = Mathf.Max(1, count.X);
                }
                if (GUI.RIntField("Y", ref count.Y, 1))
                {
                    selectionObject.SceneObjectArray.Count.Y = Mathf.Max(1, count.Y);
                }
                if (GUI.RIntField("Z", ref count.Z, 1))
                {
                    selectionObject.SceneObjectArray.Count.Z = Mathf.Max(1, count.Z);
                }

                GUI.RVec3Field("Spacing", ref selectionObject.SceneObjectArray.Spacing, Vector3.One);

                break;
            }
            }

            a_object = selectionObject;
        }
    }
}