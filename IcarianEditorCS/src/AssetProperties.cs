using IcarianEditor.Properties;
using IcarianEngine;
using IcarianEngine.Definitions;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;
using System.Xml;

namespace IcarianEditor
{
    public static class AssetProperties
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void WriteDef(string a_path, byte[] a_data);

        static PropertiesWindow                   s_defaultWindow;

        static Dictionary<Type, PropertiesWindow> s_windows;

        static string                             s_defName;

        const string DefintionNamespace = "IcarianEngine.Definitions.";

        internal static void Init()
        {
            s_defaultWindow = new PropertiesWindow();
            s_windows = new Dictionary<Type, PropertiesWindow>();

            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();

            foreach (Assembly asm in assemblies)
            {
                Type[] types = asm.GetTypes();
                foreach (Type type in types)
                {
                    PWindowAttribute att = type.GetCustomAttribute<PWindowAttribute>();
                    if (att != null)
                    {
                        if (type.IsSubclassOf(typeof(PropertiesWindow)))
                        {
                            PropertiesWindow window = Activator.CreateInstance(type) as PropertiesWindow;
                            if (window != null)
                            {
                                s_windows.Add(att.OverrideType, window);
                            }
                            else
                            {
                                Logger.Error($"IcarianEditorCS: Unabled to create PropertiesWindow of type {type}");
                            }
                        }
                        else
                        {
                            Logger.Error($"IcarianEditorCS: {type} has Attribute PWindow and is not inherited from PropertiesWindow");
                        }
                    }
                }
            }
        }

        static void SerializeField(XmlDocument a_doc, XmlElement a_parent, string a_name, object a_obj, object a_defaultObj, Type a_type)
        {
            switch (a_obj)
            {
            case Type val:
            {
                if (a_defaultObj == null || val != (Type)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case bool val:
            {
                if (a_defaultObj == null || val != (bool)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case byte val:
            {
                if (a_defaultObj == null || val != (byte)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case short val:
            {
                if (a_defaultObj == null || val != (short)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case ushort val:
            {
                if (a_defaultObj == null || val != (ushort)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case int val:
            {
                if (a_defaultObj == null || val != (int)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case uint val:
            {
                if (a_defaultObj == null || val != (uint)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            case float val:
            {
                if (a_defaultObj == null || val != (float)a_defaultObj)
                {
                    XmlElement element = a_doc.CreateElement(a_name);
                    a_parent.AppendChild(element);
                    element.InnerText = val.ToString();
                }

                break;
            }
            default:
            {
                if (a_type == typeof(Def) || a_type.IsSubclassOf(typeof(Def)))
                {
                    Def def = a_obj as Def;
                    Def pDef = a_defaultObj as Def;

                    if (def != null && (pDef == null || def.DefName != pDef.DefName))
                    {
                        XmlElement element = a_doc.CreateElement(a_name);
                        a_parent.AppendChild(element);
                        element.InnerText = def.DefName;
                    }
                }  
                else if (a_type == typeof(string))
                {
                    string val = a_obj as string; 
                    string normVal = a_defaultObj as string;

                    if (val == null)
                    {
                        val = string.Empty;
                    }
                    if (normVal == null)
                    {
                        normVal = string.Empty;
                    }

                    if (val != normVal)
                    {
                        XmlElement element = a_doc.CreateElement(a_name);
                        a_parent.AppendChild(element);
                        element.InnerText = val;
                    }
                }
                else if (a_type.IsGenericType && a_type.GetGenericTypeDefinition() == typeof(List<>) && a_obj != null)
                {
                    Type gType = a_type.GetGenericArguments()[0];
                    
                    PropertyInfo cProperty = a_type.GetProperty("Count");

                    int count = (int)cProperty.GetValue(a_obj);
                    int pCount = 0;
                    if (a_defaultObj != null)
                    {
                        pCount = (int)cProperty.GetValue(a_defaultObj);
                    }

                    if (count > pCount)
                    {
                        XmlElement rootLElement = a_doc.CreateElement(a_name);
                        a_parent.AppendChild(rootLElement);

                        IList l = a_obj as IList;

                        for (int i = pCount; i < count; ++i)
                        {
                            SerializeField(a_doc, rootLElement, "lv", l[i], null, gType);
                        }
                    }
                }
                else if (a_type.IsArray)
                {
                    Type eType = a_type.GetElementType();

                    Array a = a_obj as Array;
                    if (a != null)
                    {   
                        int len = a.Length;
                        if (len > 0)
                        {
                            XmlElement rootAElement = a_doc.CreateElement(a_name);
                            a_parent.AppendChild(rootAElement);

                            for (int i = 0; i < len; ++i)
                            {
                                SerializeField(a_doc, rootAElement, "lv", a.GetValue(i), null, eType);
                            }   
                        }
                    }
                }
                else if (a_type.IsSubclassOf(typeof(Enum)))
                {
                    if (!Enum.Equals(a_obj, a_defaultObj))
                    {
                        XmlElement element = a_doc.CreateElement(a_name);
                        a_parent.AppendChild(element);
                        element.InnerText = a_obj.ToString();
                    }
                }
                else
                {
                    if (a_obj == null || a_obj == a_defaultObj)
                    {
                        return;
                    }

                    XmlElement element = a_doc.CreateElement(a_name);

                    FieldInfo[] fields = a_type.GetFields(BindingFlags.Public | BindingFlags.Instance);
                    if (a_defaultObj != null)
                    {
                        foreach (FieldInfo field in fields)
                        {
                            if (field.GetCustomAttribute<NonSerializedAttribute>() != null)
                            {
                                continue;
                            }

                            SerializeField(a_doc, element, field.Name, field.GetValue(a_obj), field.GetValue(a_defaultObj), field.FieldType);
                        }
                    }
                    else
                    {
                        foreach (FieldInfo field in fields)
                        {
                            if (field.GetCustomAttribute<NonSerializedAttribute>() != null)
                            {
                                continue;
                            }

                            SerializeField(a_doc, element, field.Name, field.GetValue(a_obj), null, field.FieldType);
                        }
                    }

                    if (element.HasChildNodes)
                    {
                        a_parent.AppendChild(element);
                    }
                }

                break;
            }
            }
        }

        internal static XmlElement SerializeDef(XmlDocument a_doc, Def a_def)
        {
            Type type = a_def.GetType();

            string defNameStr = a_def.GetType().ToString();
            if (defNameStr.StartsWith(DefintionNamespace))
            {
                defNameStr = defNameStr.Remove(0, DefintionNamespace.Length);
            }

            XmlElement root = a_doc.CreateElement(defNameStr);
            root.SetAttribute("Name", a_def.DefName);

            Def parentDef = (Def)Activator.CreateInstance(type);
            if (!string.IsNullOrWhiteSpace(a_def.DefParentName))
            {
                parentDef = DefLibrary.GetDef(a_def.DefParentName);
                root.SetAttribute("Parent", a_def.DefParentName);
            }

            Type pType = parentDef.GetType();

            FieldInfo[] fields = type.GetFields();
            foreach (FieldInfo field in fields)
            {
                if (field.GetCustomAttribute<NonSerializedAttribute>() != null)
                {
                    continue;
                }

                Type fType = field.FieldType;

                object normVal = null;
                if (pType.GetField(field.Name) != null)
                {
                    normVal = field.GetValue(parentDef);
                }

                SerializeField(a_doc, root, field.Name, field.GetValue(a_def), normVal, fType);
            }

            return root;
        }

        static void SerializeDefs(string[] a_paths)
        {
            uint defCount = (uint)a_paths.LongLength;
            IEnumerable<Def> defs = DefLibrary.GetDefs();

            for (uint i = 0; i < defCount; ++i)
            {
                string path = a_paths[i];

                if (path == Def.SceneDefPath)
                {
                    continue;
                }

                Def mainDef = null;
                foreach (Def def in defs)
                {
                    if (def.DefPath == path)
                    {
                        mainDef = def;

                        break;
                    }
                }

                if (mainDef == null)
                {
                    continue;
                }

                XmlDocument doc = new XmlDocument();
                doc.AppendChild(doc.CreateXmlDeclaration("1.0", "utf-16", null));
                
                doc.AppendChild(SerializeDef(doc, mainDef));

                MemoryStream stream = new MemoryStream();
                XmlTextWriter writer = new XmlTextWriter(stream, Encoding.Unicode);
                writer.Formatting = Formatting.Indented;

                doc.WriteContentTo(writer);
                writer.Flush();
                stream.Flush();

                stream.Position = 0;

                StreamReader reader = new StreamReader(stream);
                byte[] bytes = Encoding.Unicode.GetBytes(reader.ReadToEnd());

                WriteDef(path, bytes);
            }
        }

        static void PushDef(string a_path)
        {
            Workspace.Selection = new List<SelectionObject>();

            IEnumerable<Def> defs = DefLibrary.GetDefs();

            foreach (Def def in defs)
            {
                if (def.DefPath == a_path)
                {
                    s_defName = def.DefName;

                    return;
                }
            }

            s_defName = null;
        }

        static void DisplayGUI(object a_object)
        {
            Type type = a_object.GetType();
            if (s_windows.ContainsKey(type))
            {
                s_windows[type].OnGUI(a_object);
            }
            else
            {
                s_defaultWindow.OnGUI(a_object);
            }
        }

        static void OnGUI()
        {
            if (Workspace.Selection != null && Workspace.Selection.Count > 0)
            {
                // TODO: Implement multi selection at some point
                DisplayGUI(Workspace.Selection[0]);
            }

            if (!string.IsNullOrWhiteSpace(s_defName))
            {
                Def def = DefLibrary.GetDef(s_defName);
                if (def != null)
                {
                    DisplayGUI(def);
                }
            }
        }
    }
}