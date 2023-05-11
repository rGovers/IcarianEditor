using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;

namespace IcarianEditor.Editor
{
    [AttributeUsage(AttributeTargets.Class)]
    public class EDisplayAttribute : Attribute
    {
        Type m_overrideType;

        public Type OverrideType
        {
            get
            {
                return m_overrideType;
            }
        }

        public EDisplayAttribute(Type a_overrideType)
        {
            m_overrideType = a_overrideType;
        }
    }

    public abstract class EditorDisplay
    {
        public abstract void Render(bool a_selected, Def a_component, Matrix4 a_transform); 
    };
}