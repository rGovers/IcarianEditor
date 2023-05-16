using IcarianEngine.Maths;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEditor
{
    public enum ManipulationMode : ushort
    {
        Translate = 0,
        Rotate = 1,
        Scale = 2
    }

    public static class Gizmos
    {
        [StructLayout(LayoutKind.Sequential, Pack = 0)]
        struct TransformValue
        {
            public Vector3 Translation;
            public Quaternion Rotation;
            public Vector3 Scale;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetManipulating();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static TransformValue GetManipulation(uint a_mode, Vector3 a_translation, Quaternion a_rotation, Vector3 a_scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawLine(Vector3 a_start, Vector3 a_end, float a_width, Vector4 a_color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawIcoSphere(Vector3 a_pos, float a_radius, uint a_subDivisions, float a_width, Vector4 a_color);

        public static bool IsManipulating
        {
            get
            {
                return GetManipulating() != 0;
            }
        }

        public static void DrawLine(Vector3 a_start, Vector3 a_end, float a_width, Color a_color)
        {
            DrawLine(a_start, a_end, a_width, a_color.ToVector4());
        }
        public static void DrawIcoSphere(Vector3 a_pos, float a_radius, uint a_subDivisions, float a_width, Color a_color)
        {
            DrawIcoSphere(a_pos, a_radius, a_subDivisions, a_width, a_color.ToVector4());
        }

        public static bool Manipulation(ManipulationMode a_mode, ref Vector3 a_translation, ref Quaternion a_rotation, ref Vector3 a_scale)
        {
            TransformValue val = GetManipulation((uint)a_mode, a_translation, a_rotation, a_scale);

            bool ret = false;
            if (val.Translation != a_translation)
            {
                ret = true;
                a_translation = val.Translation;
            }

            if (val.Rotation != a_rotation)
            {
                ret = true;
                a_rotation = val.Rotation;
            }

            if (val.Scale != a_scale)
            {
                ret = true;
                a_scale = val.Scale;
            }

            return ret;
        }
    }
}