using IcarianEditor.Modals;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEditor
{
    public class GUI
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetButton(string a_label);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetCheckbox(string a_label, IntPtr a_bool);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetDef(string a_label, string a_preview, IntPtr a_dispatchModal);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetInt(string a_label, IntPtr a_int);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetUInt(string a_label, IntPtr a_int);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetBitField(string a_label, IntPtr a_int, uint a_bitCount);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetFloat(string a_label, IntPtr a_float);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetVec2(string a_label, IntPtr a_vec);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetVec3(string a_label, IntPtr a_vec);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetVec4(string a_label, IntPtr a_vec);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetColor(string a_label, IntPtr a_vec);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetString(string a_label, string a_str);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetPathString(string a_label, string a_str);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetStringList(string a_label, string[] a_strings, IntPtr a_selected);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint ResetButton(string a_label);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Indent();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void NIndent();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Unindent();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint ShowStructView(string a_title);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint ShowArrayView(string a_title, IntPtr a_addValue);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint ShowTexture(string a_path, Vector2 a_size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Tooltip(string a_title, string a_str);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void PushID(string a_id);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void PopID();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetCurrentID();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Label(string a_str);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetSelectable(string a_str);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetContextPopup();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetContextPopupWindow();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void EndPopup();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMenu(string a_label);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void EndMenu();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMenuItem(string a_label);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint NodeI(string a_str);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void PopNode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void SameLine();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Separator();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetShiftModifier();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetCtrlModifier();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetBeginChild(string a_label, Vector2 a_size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void EndChild();

        struct DefVal
        {
            public string Input;
            public Def Def;
        }

        static List<DefVal> s_defValues = new List<DefVal>();

        public static bool ShiftModifier
        {
            get
            {
                return GetShiftModifier() != 0;
            }
        }
        public static bool CtrlModifier
        {
            get
            {
                return GetCtrlModifier() != 0;
            }
        }

        public static bool BeginChild(string a_label, Vector2 a_size)
        {
            return GetBeginChild(a_label, a_size) != 0;
        }

        public static bool Button(string a_label)
        {
            return GetButton(a_label) != 0;
        }

        public static bool RCheckbox(string a_label, ref bool a_value, bool a_default)
        {
            bool ret = false;
            if (a_value != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_value = a_default;

                    ret = true;
                }
            }

            if (Checkbox(a_label, ref a_value))
            {
                ret = true;
            }

            return ret;
        }
        public static bool Checkbox(string a_label, ref bool a_value)
        {
            uint bInt = 0;
            if (a_value)
            {
                bInt = 1;
            }

            GCHandle handle = GCHandle.Alloc(bInt, GCHandleType.Pinned);

            bool ret = false;
            if (GetCheckbox(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_value = (uint)handle.Target != 0;
            }

            handle.Free();

            return ret;
        }

        public static bool REnumField<T>(string a_label, ref T a_enum, T a_default) where T : Enum
        {
            object e = a_enum;
            bool ret = REnumField(a_label, ref e, a_default);
            if (ret)
            {
                a_enum = (T)e;
            }

            return ret; 
        }
        public static bool REnumField(string a_label, ref object a_enum, object a_default)
        {
            bool ret = false;
            if (!Enum.Equals(a_enum, a_default))
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_enum = a_default;

                    ret = true;
                }
            }

            if (EnumField(a_label, ref a_enum))
            {
                ret = true;
            }

            return ret;
        }
        public static bool EnumField<T>(string a_label, ref T a_enum) where T : Enum
        {
            object e = a_enum;
            bool ret = EnumField(a_label, ref e);
            if (ret)
            {
                a_enum = (T)e;
            }

            return ret;
        }
        public static bool EnumField(string a_label, ref object a_enum)
        {
            Type type = a_enum.GetType();
            string[] list = Enum.GetNames(type);

            string selectedStr = Enum.GetName(type, a_enum);
            int selectedValue = 0;
            for (int i = 0; i < list.Length; ++i)
            {
                if (list[i] == selectedStr)
                {
                    selectedValue = i;

                    break;
                }
            }

            GCHandle handle = GCHandle.Alloc(selectedValue, GCHandleType.Pinned);
            
            bool ret = false;
            if (GetStringList(a_label, list, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_enum = Enum.Parse(type, list[(int)handle.Target]);
            }

            handle.Free();

            return ret;
        }

        public static bool StringSelector(string a_label, string[] a_strings, ref int a_selected)
        {
            GCHandle handle = GCHandle.Alloc(a_selected, GCHandleType.Pinned);

            bool ret = false;
            if (GetStringList(a_label, a_strings, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_selected = (int)handle.Target;
            }

            handle.Free();

            return ret;
        }

        public static bool RIntField(string a_label, ref int a_int, int a_default = default(int))
        {
            bool ret = false;
            if (a_int != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_int = a_default;

                    ret = true;
                }
            }

            if (IntField(a_label, ref a_int))
            {
                ret = true;
            }

            return ret;
        }
        public static bool IntField(string a_label, ref int a_int)
        {
            GCHandle handle = GCHandle.Alloc(a_int, GCHandleType.Pinned);

            bool ret = false;
            if (GetInt(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_int = (int)handle.Target;
            }

            handle.Free();

            return ret;
        }
        public static bool RUIntField(string a_label, ref uint a_int, uint a_default = default(uint))
        {
            bool ret = false;
            if (a_int != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_int = a_default;

                    ret = true;
                }
            }

            if (UIntField(a_label, ref a_int))
            {
                ret = true;
            }

            return ret;
        }
        public static bool UIntField(string a_label, ref uint a_int)
        {
            GCHandle handle = GCHandle.Alloc(a_int, GCHandleType.Pinned);

            bool ret = false;
            if (GetUInt(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_int = (uint)handle.Target;
            }

            handle.Free();

            return ret;
        }

        public static bool BitField(string a_label, ref uint a_value, uint a_bitCount = 8)
        {
            GCHandle handle = GCHandle.Alloc(a_value, GCHandleType.Pinned);

            bool ret = false;
            if (GetBitField(a_label, handle.AddrOfPinnedObject(), a_bitCount) != 0)
            {
                ret = true;
                a_value = (uint)handle.Target;
            }

            handle.Free();

            return ret;
        }

        public static bool RFloatField(string a_label, ref float a_float, float a_default = default(float))
        {
            bool ret = false;
            if (a_float != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_float = a_default;

                    ret = true;
                }
            }

            if (FloatField(a_label, ref a_float))
            {
                ret = true;
            }

            return ret;
        }
        public static bool FloatField(string a_label, ref float a_float)
        {
            GCHandle handle = GCHandle.Alloc(a_float, GCHandleType.Pinned);

            bool ret = false;
            if (GetFloat(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_float = (float)handle.Target;
            }

            handle.Free();

            return ret;
        }

        public static bool RVec2Field(string a_label, ref Vector2 a_vec, Vector2 a_default = default(Vector2))
        {
            bool ret = false;
            if (a_vec != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    ret = true;
                    a_vec = a_default;
                }
            }

            if (Vec2Field(a_label, ref a_vec))
            {
                ret = true;
            }

            return ret;
        }
        public static bool Vec2Field(string a_label, ref Vector2 a_vec)
        {
            GCHandle handle = GCHandle.Alloc(a_vec, GCHandleType.Pinned);

            bool ret = false;
            if (GetVec2(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_vec = (Vector2)handle.Target;
            }

            handle.Free();

            return ret;
        }
        public static bool RVec3Field(string a_label, ref Vector3 a_vec, Vector3 a_default = default(Vector3))
        {
            bool ret = false;
            if (a_vec != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    ret = true;
                    a_vec = a_default;
                }
            }

            if (Vec3Field(a_label, ref a_vec))
            {
                ret = true;
            }

            return ret;
        }
        public static bool Vec3Field(string a_label, ref Vector3 a_vec)
        {
            GCHandle handle = GCHandle.Alloc(a_vec, GCHandleType.Pinned);

            bool ret = false;
            if (GetVec3(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_vec = (Vector3)handle.Target;
            }

            handle.Free();

            return ret;
        }
        public static bool RVec4Field(string a_label, ref Vector4 a_vec, Vector4 a_default = default(Vector4))
        {
            bool ret = false;
            if (a_vec != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    ret = true;
                    a_vec = a_default;
                }
            }

            if (Vec4Field(a_label, ref a_vec))
            {
                ret = true;
            }

            return ret;
        }
        public static bool Vec4Field(string a_label, ref Vector4 a_vec) 
        {
            GCHandle handle = GCHandle.Alloc(a_vec, GCHandleType.Pinned);

            bool ret = false;
            if (GetVec4(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_vec = (Vector4)handle.Target;
            }

            handle.Free();

            return ret;
        }

        public static bool RQuaternionField(string a_label, ref Quaternion a_quat, Quaternion a_default = default(Quaternion))
        {
            Vector4 val = a_quat.ToVector4();
            if (RVec4Field(a_label, ref val, a_default.ToVector4()))
            {
                a_quat = val.ToQuaternion();

                return true;
            }

            return false;
        }
        public static bool QuaternionField(string a_label, ref Quaternion a_quat)
        {
            Vector4 val = a_quat.ToVector4();
            if (Vec4Field(a_label, ref val))
            {
                a_quat = val.ToQuaternion();

                return true;
            }

            return false;
        }
        
        public static bool RColorField(string a_label, ref Color a_color, Color a_default = default(Color))
        {
            bool ret = false;
            if (a_color != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    ret = true;
                    a_color = a_default;
                }
            }

            if (ColorField(a_label, ref a_color))
            {
                ret = true;
            }

            return ret;
        }
        public static bool ColorField(string a_label , ref Color a_color)
        {
            Vector4 vec = a_color.ToVector4();
            GCHandle handle = GCHandle.Alloc(vec, GCHandleType.Pinned);

            bool ret = false;
            if (GetColor(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
                a_color = ((Vector4)handle.Target).ToColor();
            }

            handle.Free();

            return ret;
        }

        public static bool RStringField(string a_label, ref string a_str, string a_default = null)
        {
            if (a_str == null)
            {
                a_str = string.Empty;
            }
            if (a_default == null)
            {
                a_default = string.Empty;
            }

            bool ret = false;
            if (a_str != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_str = a_default;
                    ret = true;
                }
            }

            if (StringField(a_label, ref a_str))
            {
                ret = true;
            }

            return ret;
        }
        public static bool StringField(string a_label, ref string a_str)
        {
            if (a_str == null)
            {
                a_str = string.Empty;
            }

            string ret = GetString(a_label, a_str);
            if (ret != null)
            {
                a_str = ret;
                
                return true;
            }

            return false;
        }

        public static bool RPathStringField(string a_label, ref string a_str, string a_default = null)
        {
            if (a_str == null)
            {
                a_str = string.Empty;
            }
            if (a_default == null)
            {
                a_default = string.Empty;
            }

            bool ret = false;
            if (a_str != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_str = a_default;
                    ret = true;
                }
            }

            if (PathStringField(a_label, ref a_str))
            {
                ret = true;
            }

            return ret;
        }
        public static bool PathStringField(string a_label, ref string a_str)
        {
            if (a_str == null)
            {
                a_str = string.Empty;
            }

            string ret = GetPathString(a_label, a_str);
            if (ret != null)
            {
                a_str = ret;

                return true;
            }

            return false;
        }

        internal static void PushDef(string a_input, Def a_def)
        {
            DefVal val = new DefVal();
            val.Input = a_input;
            val.Def = a_def;

            s_defValues.Add(val);
        }

        public static bool RDefField<T>(string a_label, ref T a_def, T a_default, bool a_useSceneDefs = false) where T : Def
        {
            bool ret = false;
            if (a_def != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_def = a_default;
                    ret = true;
                }
            }

            if (DefField<T>(a_label, ref a_def, a_useSceneDefs))
            {
                ret = true;
            }

            return ret;
        }
        public static bool DefField<T>(string a_label, ref T a_def, bool a_useSceneDefs = false) where T : Def
        {
            string id = $"{GetCurrentID()}: {a_label}";

            foreach (DefVal val in s_defValues)
            {
                if (val.Input == id)
                {
                    a_def = val.Def as T;
                    s_defValues.Remove(val);

                    return true;
                }
            }

            string preview = null;
            if (a_def != null)
            {
                preview = a_def.DefName;
            }

            if (preview == null)
            {
                preview = "Null";
            }

            GCHandle handle = GCHandle.Alloc(0U, GCHandleType.Pinned);

            string ret = GetDef(a_label, preview, handle.AddrOfPinnedObject());
            bool dispatch = (uint)handle.Target != 0;

            handle.Free();

            if (dispatch)
            {
                new GUIGetDefSelectorModal<T>(id, a_def, a_useSceneDefs);
            }

            if (ret != null)
            {
                if (ret == Def.SceneDefPath)
                {   
                    a_def = null;
                }
                else
                {
                    a_def = EditorDefLibrary.GeneratePathDef<T>(ret);
                }

                return true;
            }

            return false;
        }
        public static bool RDefField(string a_label, ref Def a_def, Def a_default, bool a_useSceneDefs = false)
        {
            bool ret = false;
            if (a_def != a_default)
            {
                if (ResetButton(a_label + "_R") != 0)
                {
                    a_def = a_default;
                    ret = true;
                }
            }

            if (DefField(a_label, ref a_def, a_useSceneDefs))
            {
                ret = true;
            }

            return ret;
        }
        public static bool DefField(string a_label, ref Def a_def, bool a_useSceneDefs = false)
        {
            string id = $"{GetCurrentID()}: {a_label}";

            foreach (DefVal val in s_defValues)
            {
                if (val.Input == id)
                {
                    a_def = val.Def;
                    s_defValues.Remove(val);

                    return true;
                }
            }

            string preview = null;
            if (a_def != null)
            {
                preview = a_def.DefName;
            }

            if (preview == null)
            {
                preview = "Null";
            }

            GCHandle handle = GCHandle.Alloc(0U, GCHandleType.Pinned);

            string ret = GetDef(a_label, preview, handle.AddrOfPinnedObject());
            bool dispatch = (uint)handle.Target != 0;
            
            handle.Free();

            if (dispatch)
            {
                new GUIGetDefSelectorModal<Def>(id, a_def, a_useSceneDefs);
            }

            if (ret != null)
            {
                if (ret == Def.SceneDefPath)
                {
                    a_def = null;
                }
                else
                {
                    a_def = EditorDefLibrary.GeneratePathDef(ret);
                }

                return true;
            } 

            return false;
        }

        public static bool BeginContextPopup()
        {
            return GetContextPopup() != 0;
        }
        public static bool BeginContextPopupWindow()
        {
            return GetContextPopupWindow() != 0;
        }

        public static bool BeginMenu(string a_label)
        {
            return GetMenu(a_label) != 0;
        }

        public static bool MenuItem(string a_label)
        {
            return GetMenuItem(a_label) != 0;
        }

        public static bool StructView(string a_label)
        {
            return ShowStructView(a_label) != 0;
        }
        public static bool ArrayView(string a_label, out bool a_addValue)
        {
            bool ret = false;

            GCHandle handle = GCHandle.Alloc(0U, GCHandleType.Pinned);
            if (ShowArrayView(a_label, handle.AddrOfPinnedObject()) != 0)
            {
                ret = true;
            }

            a_addValue = (uint)handle.Target != 0;

            handle.Free();

            return ret;
        }
        
        public static bool Texture(string a_path, Vector2 a_size)
        {
            return ShowTexture(a_path, a_size) != 0;
        }

        public static bool Node(string a_str)
        {
            return NodeI(a_str) != 0;
        }

        public static bool Selectable(string a_str)
        {
            return GetSelectable(a_str) != 0;
        }
    }
}