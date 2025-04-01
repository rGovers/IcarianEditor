// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Properties
{
    [AttributeUsage(AttributeTargets.Class)]
    public class PWindowAttribute : Attribute
    {
        Type m_overrideType;

        public Type OverrideType
        {
            get
            {
                return m_overrideType;
            }
        }

        public PWindowAttribute(Type a_overrideType)
        {
            m_overrideType = a_overrideType;
        }
    }

    public class PropertiesEditorWindow
    {
        static string FormattedName(string a_name)
        {
            if (string.IsNullOrWhiteSpace(a_name))
            {
                return "[BADWHITESPACENAME]";
            }

            int startIndex = 0;
            if (a_name.StartsWith("m_"))
            {
                startIndex = 2;
            }

            int len = a_name.Length;
            if (len <= startIndex)
            {
                return "[BADFORMATNAME]";
            }

            string outStr = string.Empty;
            outStr += char.ToUpper(a_name[startIndex]);

            for (int i = startIndex + 1; i < len; ++i)
            {
                char c = a_name[i];
                if (char.IsUpper(c))
                {
                    if (char.IsLower(a_name[i - 1]))
                    {
                        outStr += $" {c}";

                        continue;
                    }
                }

                outStr += c;
            }

            return outStr;
        }

        static void ShowFields(string a_name, bool a_sceneObject, ref object a_obj, object a_normVal, Type a_type, IEnumerable<Attribute> a_attributes)
        {
            switch (a_obj)
            {
            case Type _:
            {
                // Ignoring for now as I cannot think of a good way of handling it
                return;
            }
            case IntPtr _:
            {
                // Ignore pointers
                return;
            }
            case bool val:
            {   
                if (GUI.RCheckbox(a_name, ref val, (bool)a_normVal))
                {
                    a_obj = (bool)val;
                }

                break;
            }
            case sbyte val:
            {
                sbyte min = 0;
                sbyte max = 0;

                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorRangeAttribute range)
                    {
                        min = (sbyte)range.Min;
                        max = (sbyte)range.Max;
                    }
                }

                sbyte nVal = (sbyte)a_normVal;
                int sVal = (int)val;
                if (GUI.RIntField(a_name, ref sVal, (int)nVal))
                {
                    a_obj = (sbyte)sVal;
                }

                break;
            }
            case byte val:
            {
                byte min = 0;
                byte max = 0;

                uint sVal = (uint)val;
                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorBitfieldAttribute)
                    {
                        if (GUI.BitField(a_name, ref sVal, 8))
                        {
                            a_obj = (byte)sVal;
                        }

                        return;
                    }
                    else if (a is EditorRangeAttribute range)
                    {
                        min = (byte)range.Min;
                        max = (byte)range.Max;
                    }
                }

                byte nVal = (byte)a_normVal;
                if (min != max)
                {
                    if (GUI.RUIntSliderField(a_name, ref sVal, min, max, (uint)nVal))
                    {
                        a_obj = (byte)sVal;
                    }
                }
                else
                {
                    if (GUI.RUIntField(a_name, ref sVal, (uint)nVal))
                    {
                        a_obj = (byte)sVal;
                    }
                }

                break;
            }
            case short val:
            {
                short min = 0;
                short max = 0;

                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorRangeAttribute range)
                    {
                        min = (short)range.Min;
                        max = (short)range.Max;
                    }
                }

                short nVal = (short)a_normVal;
                int sVal = (int)val;
                if (min != max)
                {
                    if (GUI.RIntSliderField(a_name, ref sVal, min, max, (int)nVal))
                    {
                        a_obj = (short)sVal;
                    }
                }
                else
                {
                    if (GUI.RIntField(a_name, ref sVal, (int)nVal))
                    {
                        a_obj = (short)sVal;
                    }
                }

                break;
            }
            case ushort val:
            {
                ushort min = 0;
                ushort max = 0;

                uint sVal = (uint)val;
                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorBitfieldAttribute)
                    {
                        if (GUI.BitField(a_name, ref sVal, 16))
                        {
                            a_obj = (ushort)sVal;
                        }

                        return;
                    }
                    else if (a is EditorRangeAttribute range)
                    {
                        min = (ushort)range.Min;
                        max = (ushort)range.Max;
                    }
                }

                ushort nVal = (ushort)a_normVal;
                if (min != max)
                {
                    if (GUI.RUIntSliderField(a_name, ref sVal, min, max, (uint)nVal))
                    {
                        a_obj = (ushort)sVal;
                    }
                }
                else
                {
                    if (GUI.RUIntField(a_name, ref sVal, (uint)nVal))
                    {
                        a_obj = (ushort)sVal;
                    }
                }

                break;
            }
            case int val:
            {
                int min = 0;
                int max = 0;

                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorRangeAttribute range)
                    {
                        min = (int)range.Min;
                        max = (int)range.Max;
                    }
                }

                if (min != max)
                {
                    if (GUI.RIntSliderField(a_name, ref val, min, max, (int)a_normVal))
                    {
                        a_obj = val;
                    }
                }
                else
                {
                    if (GUI.RIntField(a_name, ref val, (int)a_normVal))
                    {
                        a_obj = val;
                    }
                }

                break;
            }
            case uint val:
            {
                uint min = 0;
                uint max = 0;

                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorBitfieldAttribute)
                    {
                        if (GUI.BitField(a_name, ref val, 32))
                        {
                            a_obj = val;
                        }

                        return;
                    }
                    else if (a is EditorRangeAttribute range)
                    {
                        min = (uint)range.Min;
                        max = (uint)range.Max;
                    }
                }

                if (min != max)
                {
                    if (GUI.RUIntSliderField(a_name, ref val, min, max, (uint)a_normVal))
                    {
                        a_obj = val;
                    }
                }
                else
                {
                    if (GUI.RUIntField(a_name, ref val, (uint)a_normVal))
                    {
                        a_obj = val;
                    }
                }

                break;
            }
            case float val:
            {   
                bool isAngle = false;
                float min = 0.0f;
                float max = 0.0f;

                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorAngleAttribute)
                    {
                        isAngle = true;
                    }
                    else if (a is EditorRangeAttribute range)
                    {
                        min = (float)range.Min;
                        max = (float)range.Max;
                    }
                }

                if (isAngle && EditorConfig.UseDegrees)
                {
                    float angle = val * Mathf.RadToDeg;
                    if (min != max)
                    {
                        float maxAngle = max * Mathf.RadToDeg;
                        float minAngle = min * Mathf.RadToDeg;

                        if (GUI.RFloatSliderField(a_name, ref angle, minAngle, maxAngle, (float)a_normVal))
                        {
                            a_obj = angle * Mathf.DegToRad;
                        }
                    }
                    else
                    {
                        if (GUI.RFloatField(a_name, ref val, (float)a_normVal))
                        {
                            a_obj = angle * Mathf.DegToRad;
                        }
                    }

                    break;
                }

                if (min != max)
                {
                    if (GUI.RFloatSliderField(a_name, ref val, min, max, (float)a_normVal))
                    {
                        a_obj = val;
                    }
                }
                else
                {
                    if (GUI.RFloatField(a_name, ref val, (float)a_normVal))
                    {
                        a_obj = val;
                    }
                }

                break;
            }
            case Vector2 val:
            {
                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorAngleAttribute)
                    {
                        if (EditorConfig.UseDegrees)
                        {
                            Vector2 v = val * Mathf.RadToDeg;
                            if (GUI.RVec2Field(a_name, ref v, (Vector2)a_normVal))
                            {
                                a_obj = v * Mathf.DegToRad;
                            }

                            return;
                        }

                        break;
                    }
                }

                if (GUI.RVec2Field(a_name, ref val, (Vector2)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case Vector3 val:
            {
                foreach (Attribute a in a_attributes)
                {
                    if (a is EditorAngleAttribute)
                    {
                        if (EditorConfig.UseDegrees)
                        {
                            Vector3 v = val * Mathf.RadToDeg;
                            if (GUI.RVec3Field(a_name, ref v, (Vector3)a_normVal))
                            {
                                a_obj = v * Mathf.DegToRad;
                            }

                            return;
                        }

                        break;
                    }
                }

                if (GUI.RVec3Field(a_name, ref val, (Vector3)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case Vector4 val: 
            {   
                if (GUI.RVec4Field(a_name, ref val, (Vector4)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case Quaternion val:
            {
                // TODO: Improve this as could be a little smarter and use a rotation mode
                if (GUI.RQuaternionField(a_name, ref val, (Quaternion)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case Color val:
            {
                if (GUI.RColorField(a_name, ref val, (Color)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            default:
            {
                if (a_type == typeof(Def) || a_type.IsSubclassOf(typeof(Def)))
                {
                    Def def = (Def)a_obj;
                    Def normDef = (Def)a_normVal; 
                    
                    // Cant pass types to generics so time for the song and dance
                    Type guiType = typeof(GUI);

                    // I would get the method by name but throws an exception if multiple methods are found
                    MethodInfo[] methods = guiType.GetMethods(BindingFlags.Static | BindingFlags.Public);

                    foreach (MethodInfo method in methods)
                    {
                        if (method.Name != "RDefField")
                        {
                            continue;
                        }

                        if (method.IsGenericMethod)
                        {
                            MethodInfo genericInfo = method.MakeGenericMethod(a_type);

                            object[] args = new object[] { a_name, def, normDef, a_sceneObject };
                            if ((bool)genericInfo.Invoke(null, args))
                            {
                                a_obj = args[1] as Def;
                            }

                            break;
                        }
                    }
                }
                else if (a_type == typeof(string))
                {
                    // C# cannot figure out strings again so cannot use pattern matching has to be Type
                    string val = (string)a_obj;

                    foreach (Attribute a in a_attributes)
                    {
                        if (a is EditorPathStringAttribute path)
                        {
                            if (GUI.RPathStringField(a_name, ref val, path.Extensions, (string)a_normVal))
                            {
                                a_obj = val;
                            }

                            return;
                        }
                    }

                    if (GUI.RStringField(a_name, ref val, (string)a_normVal))
                    {
                        a_obj = val;
                    }
                }
                else if (a_type.IsSubclassOf(typeof(Enum)))
                {
                    GUI.REnumField(a_name, ref a_obj, a_normVal);
                }
                else if (a_type.IsArray)
                {
                    Type eType = a_type.GetElementType();

                    Array a = (Array)a_obj;

                    bool add;
                    bool show = GUI.ArrayView(a_name, out add);

                    if (add)
                    {
                        if (a == null)
                        {
                            a = Array.CreateInstance(eType, 1);
                        }
                        else
                        {
                            Array temp = Array.CreateInstance(eType, a.Length + 1);
                            Array.Copy(a, temp, a.Length);
                            a = temp;
                        }
                    }   

                    if (show && a != null)
                    {
                        GUI.Indent();
                        GUI.PushID(a_name);

                        object eNVal = Activator.CreateInstance(eType);

                        int len = a.Length;
                        for (int i = 0; i < len; ++i)
                        {
                            object o = a.GetValue(i);
                            ShowFields($"[{i}]", a_sceneObject, ref o, eNVal, eType, a_attributes);
                            a.SetValue(o, i);
                        }

                        GUI.PopID();
                        GUI.Unindent();
                    }

                    a_obj = a;
                }
                else if (a_type.IsGenericType && a_type.GetGenericTypeDefinition() == typeof(List<>))
                {
                    Type gType = a_type.GetGenericArguments()[0];
                    MethodInfo method = a_type.GetMethod("Add");
                
                    bool add;
                    bool show = GUI.ArrayView(a_name, out add);
                    
                    object gNVal = Activator.CreateInstance(gType);

                    if (add)
                    {
                        if (a_obj == null)
                        {
                            a_obj = Activator.CreateInstance(a_type);
                        }

                        method.Invoke(a_obj, new object[] { gNVal });
                    }

                    if (show && a_obj != null)
                    {
                        GUI.Indent();
                        GUI.PushID(a_name);

                        object nObj = Activator.CreateInstance(a_type);

                        uint index = 0;
                        IEnumerable enumerable = (IEnumerable)a_obj;
                        foreach (object val in enumerable)
                        {
                            object oVal = val;

                            GUI.PushID($"[{index}]");

                            if (GUI.Button("-"))
                            {
                                GUI.PopID();

                                continue;
                            }

                            GUI.SameLine();

                            ShowFields($"[{index++}]", a_sceneObject, ref oVal, gNVal, gType, a_attributes);

                            method.Invoke(nObj, new object[] { oVal });

                            GUI.PopID();
                        }

                        a_obj = nObj;

                        GUI.PopID();
                        GUI.Unindent();
                    }
                }
                else
                {
                    if (a_obj == null)
                    {
                        a_obj = Activator.CreateInstance(a_type);
                    }

                    if (GUI.StructView(a_name))
                    {
                        GUI.Indent();
                        GUI.PushID(a_name);

                        FieldInfo[] fields = a_type.GetFields();
                        foreach (FieldInfo field in fields)
                        {
                            if (field.IsStatic || (field.IsPrivate && field.GetCustomAttributes<SerializableAttribute>() == null) || field.GetCustomAttribute<HideInEditorAttribute>() != null)
                            {
                                continue;
                            }

                            Type fieldType = field.FieldType;

                            object val = field.GetValue(a_obj);
                            object normObj = null;

                            // Fucking strings cant use Activator
                            // UPDATE: Turns out I need both cause C# is annoying like that
                            ConstructorInfo constructor = fieldType.GetConstructor(Type.EmptyTypes);
                            if (constructor != null)
                            {
                                normObj = constructor.Invoke(null);
                            }
                            else
                            {
                                normObj = Activator.CreateInstance(fieldType);
                            }

                            List<Attribute> atts = new List<Attribute>(a_attributes);
                            atts.AddRange(field.GetCustomAttributes());

                            ShowFields($"{a_name}.{field.Name}", a_sceneObject, ref val, normObj, fieldType, atts);

                            field.SetValue(a_obj, val);
                        }

                        GUI.PopID();
                        GUI.Unindent();
                    }
                }

                break;
            }
            }
        }

        public static void BaseGUI(object a_object, bool a_sceneObject)
        {
            Type objType = a_object.GetType();

            if (objType.IsPrimitive || objType == typeof(decimal) || objType == typeof(float) || objType == typeof(double))
            {
                return;
            }

            object normObj = Activator.CreateInstance(objType);

            FieldInfo[] fields = objType.GetFields();
            foreach (FieldInfo field in fields)
            {
                if (field.IsStatic || field.IsLiteral)
                {
                    continue;
                }

                if ((field.IsPrivate && field.GetCustomAttributes<SerializableAttribute>() == null) || field.GetCustomAttribute<HideInEditorAttribute>() != null)
                {
                    continue;
                }

                string fName = FormattedName(field.Name);

                object val = field.GetValue(a_object);

                IEnumerable<Attribute> attributes = field.GetCustomAttributes();

                ShowFields(fName, a_sceneObject, ref val, field.GetValue(normObj), field.FieldType, attributes);
 
                field.SetValue(a_object, val);

                foreach (Attribute a in attributes)
                {
                    if (a is EditorTooltipAttribute tooltip)
                    {
                        GUI.Tooltip(fName, tooltip.Tooltip);
                    }
                }
            }
        }

        public virtual void OnGUI(object a_object, bool a_sceneObject)
        {
            BaseGUI(a_object, a_sceneObject);
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