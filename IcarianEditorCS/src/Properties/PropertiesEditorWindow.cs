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
            case short val:
            {
                short nVal = (short)a_normVal;
                int sVal = (int)val;
                if (GUI.RIntField(a_name, ref sVal, (int)nVal))
                {
                    a_obj = (short)sVal;
                }

                break;
            }
            case ushort val:
            {
                ushort nVal = (ushort)a_normVal;
                uint sVal = (uint)val;
                if (GUI.RUIntField(a_name, ref sVal, (uint)nVal))
                {
                    a_obj = (ushort)sVal;
                }

                break;
            }
            case int val:
            {
                if (GUI.RIntField(a_name, ref val, (int)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case uint val:
            {
                if (GUI.RUIntField(a_name, ref val, (uint)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case float val:
            {   
                if (GUI.RFloatField(a_name, ref val, (float)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case Vector2 val:
            {
                if (GUI.RVec2Field(a_name, ref val, (Vector2)a_normVal))
                {
                    a_obj = val;
                }

                break;
            }
            case Vector3 val:
            {
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

                    // I would get the method by name but throws an exception is multiple methods are found
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
                            ConstructorInfo constructor = fieldType.GetConstructor(Type.EmptyTypes);
                            if (constructor != null)
                            {
                                normObj = constructor.Invoke(null);
                            }

                            List<Attribute> atts = new List<Attribute>(a_attributes);
                            atts.AddRange(field.GetCustomAttributes());

                            ShowFields($"{a_name}.{field.Name}", a_sceneObject, ref val, field.GetValue(normObj), field.FieldType, atts);

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

                string fName = field.Name;

                object val = field.GetValue(a_object);

                IEnumerable<Attribute> attributes = field.GetCustomAttributes();

                ShowFields(field.Name, a_sceneObject, ref val, field.GetValue(normObj), field.FieldType, attributes);
 
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
