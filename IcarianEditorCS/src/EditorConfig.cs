using System.Runtime.CompilerServices;

namespace IcarianEditor
{
    public static class EditorConfig
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetUseDegrees();

        public static bool UseDegrees
        {
            get
            {
                return GetUseDegrees() != 0;
            }
        }
    }
}