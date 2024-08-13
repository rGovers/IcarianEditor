// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.IO;
using System.Text;
using System.Xml;

#include "InteropBinding.h"
#include "EditorDefLibraryInterop.h"

EDITORDEFLIBRARY_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEditor
{
    // Need to keep data in an intermediate format so that parent def data propergates to child def data
    // This means that def retrieval mean building the def data from the intermediate format
    // All of this is very slow, but it is only used in the editor
    // I was thinking of direct field access but could not think of a good mapping so end up turning defs back into the intermediate format
    // Luckily I was able to reuse the intermediate format for loading defs from xml
    public static class EditorDefLibrary
    {
        delegate DefData DefDataLoader(string a_path, XmlElement a_root);
        delegate Def DefGenerator(DefData a_data, IEnumerable<DefData> a_defList);

        static List<DefData>           s_defs = new List<DefData>();
        static Dictionary<string, int> s_defPathLookup = new Dictionary<string, int>();
        static Dictionary<string, int> s_defNameLookup = new Dictionary<string, int>();

        static Dictionary<string, int> s_sceneDefLookup = new Dictionary<string, int>();

        static DefDataLoader           s_defDataLoader = null;
        static DefGenerator            s_createDef = null;

        public const string DefintionNamespace = "IcarianEngine.Definitions";

        internal static void Init()
        {   
            Type defLibraryType = typeof(DefLibrary);

            MethodInfo loadMethod = defLibraryType.GetMethod("GetDefData", BindingFlags.Static | BindingFlags.NonPublic);
            s_defDataLoader = (DefDataLoader)Delegate.CreateDelegate(typeof(DefDataLoader), loadMethod);

            MethodInfo generateMethod = defLibraryType.GetMethod("CreateDef", BindingFlags.Static | BindingFlags.NonPublic);
            s_createDef = (DefGenerator)Delegate.CreateDelegate(typeof(DefGenerator), generateMethod);

            MethodInfo defLibraryInitMethod = defLibraryType.GetMethod("Init", BindingFlags.Static | BindingFlags.NonPublic);
            defLibraryInitMethod.Invoke(null, new object[] { });
        }

        internal static void Clear()
        {
            s_defs.Clear();
            s_defPathLookup.Clear();
            s_defNameLookup.Clear();

            DefLibrary.Clear();
        }

        static void Load(byte[][] a_data, string[] a_paths)
        {
            uint defCount = (uint)a_data.Length;

            s_defs.Clear();

            int index = 0;

            for (uint i = 0; i < defCount; ++i)
            {
                string path = a_paths[i];

                MemoryStream stream = new MemoryStream(a_data[i]);

                XmlDocument doc = new XmlDocument();
                doc.Load(stream);

                if (doc.DocumentElement is XmlElement root)
                {
                    DefData data = s_defDataLoader(path, root);

                    if (!string.IsNullOrWhiteSpace(data.Name))
                    {
                        s_defs.Add(data);

                        s_defPathLookup.Add(path, index);
                        s_defNameLookup.Add(data.Name, index);

                        ++index;
                    }
                }
            }
        }

        internal static IEnumerable<string> LoadSceneDefs(XmlElement a_defs)
        {
            if (a_defs == null)
            {
                return null;
            }

            List<string> sceneDefNames = new List<string>();

            int index = s_defs.Count;

            foreach (XmlNode node in a_defs.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    DefData dat = s_defDataLoader(Def.SceneDefPath, element);

                    if (!string.IsNullOrWhiteSpace(dat.Name))
                    {
                        s_defs.Add(dat);
                        s_sceneDefLookup.Add(dat.Name, index++);
                        sceneDefNames.Add(dat.Name);
                    }
                }
            }

            return sceneDefNames;
        }

        static void WriteDataObject(XmlDocument a_doc, XmlElement a_parent, DefDataObject a_object)
        {
            XmlElement element = a_doc.CreateElement(a_object.Name);
            a_parent.AppendChild(element);

            if (a_object.Children != null && a_object.Children.Count > 0)
            {
                foreach (DefDataObject child in a_object.Children)
                {
                    WriteDataObject(a_doc, element, child);
                }
            }
            else
            {
                element.InnerText = a_object.Text;
            }
        }

        internal static XmlElement CreateDef(XmlDocument a_doc, DefData a_data)
        {
            XmlElement root = a_doc.CreateElement(a_data.Type);
            
            root.SetAttribute("Name", a_data.Name);
            if (a_data.Abstract)
            {
                root.SetAttribute("Abstract", "true");
            }
            if (!string.IsNullOrWhiteSpace(a_data.Parent))
            {
                root.SetAttribute("Parent", a_data.Parent);
            }

            foreach (DefDataObject dataObject in a_data.DefDataObjects)
            {
                WriteDataObject(a_doc, root, dataObject);
            }

            return root;
        }
        internal static XmlElement CreateDef(XmlDocument a_doc, string a_defName, bool a_sceneDef = false)
        {
            int index;
            if (a_sceneDef)
            {
                if (s_sceneDefLookup.TryGetValue(a_defName, out index))
                {
                    return CreateDef(a_doc, s_defs[index]);
                }
            }
            else
            {
                if (s_defNameLookup.TryGetValue(a_defName, out index))
                {
                    return CreateDef(a_doc, s_defs[index]);
                }
            }   

            return null;
        }

        static void SerializeDefs()
        {
            foreach (DefData defData in s_defs)
            {
                if (defData.Path == Def.SceneDefPath)
                {
                    continue;
                }

                XmlDocument doc = new XmlDocument();
                doc.AppendChild(doc.CreateXmlDeclaration("1.0", "UTF-8", null));

                XmlElement root = CreateDef(doc, defData);
                doc.AppendChild(root);

                MemoryStream stream = new MemoryStream();
                XmlTextWriter writer = new XmlTextWriter(stream, Encoding.UTF8);
                writer.Formatting = Formatting.Indented;

                doc.WriteContentTo(writer);
                writer.Flush();
                stream.Flush();

                stream.Position = 0;

                StreamReader reader = new StreamReader(stream);
                byte[] data = Encoding.UTF8.GetBytes(reader.ReadToEnd());

                EditorDefLibraryInterop.WriteDef(defData.Path, data);
            }
        }

        public static bool IsAbstract(string a_name)
        {
            int index;
            if (s_defNameLookup.TryGetValue(a_name, out index))
            {
                return s_defs[index].Abstract;
            }

            return false;
        }
        public static bool IsSceneDef(string a_name)
        {            
            return s_sceneDefLookup.ContainsKey(a_name);
        }

        public static bool Exists(string a_name)
        {
            if (s_defNameLookup.ContainsKey(a_name))
            {
                return true;
            }

            if (s_sceneDefLookup.ContainsKey(a_name))
            {
                return true;
            }

            return false;
        }

        public static string GetDefName(string a_path)
        {
            int index;
            if (s_defPathLookup.TryGetValue(a_path, out index))
            {
                return s_defs[index].Name;
            }

            return string.Empty;
        }

        static void FindDefs(object a_object)
        {
            if (a_object == null)
            {
                return;
            }

            // NOTE: Only going one field deep may change this in the future

            Type type = a_object.GetType();
            FieldInfo[] fields = type.GetFields(BindingFlags.Instance | BindingFlags.Public);

            foreach (FieldInfo f in fields)
            {
                Type fieldType = f.FieldType;

                if (fieldType == typeof(Def) || fieldType.IsSubclassOf(typeof(Def)))
                {
                    Def def = f.GetValue(a_object) as Def;
                    if (def != null)
                    {
                        f.SetValue(a_object, GenerateDef(def.DefName, true));
                    }
                }
            }
        }

        public static IEnumerable<T> GenerateDefs<T>() where T : Def
        {
            Type type = typeof(T);

            foreach (DefData defData in s_defs)
            {
                if (string.IsNullOrEmpty(defData.Name))
                {
                    continue;
                }

                Type defType = ModControl.GetTypeValue(defData.Type, true);
                // To ensure stuff still works if there is an error in end user code
                if (defType == null)
                {
                    continue;
                }

                if (type == defType || defType.IsSubclassOf(type))
                {
                    yield return GenerateDef<T>(defData.Name);
                }
            }
        }

        public static Def GenerateDef(string a_name, bool a_recursive = false)
        {
            Def def = null;

            int index;
            if (s_defNameLookup.TryGetValue(a_name, out index))
            {
                def = s_createDef(s_defs[index], s_defs);
            }

            if (def == null)
            {
                if (s_sceneDefLookup.TryGetValue(a_name, out index))
                {
                    def = s_createDef(s_defs[index], s_defs);
                }
            }
            
            if (def != null && a_recursive)
            {
                FindDefs(def);
            }

            return def;
        }
        public static T GenerateDef<T>(string a_name, bool a_recursive = false) where T : Def
        {
            return GenerateDef(a_name, a_recursive) as T;
        }

        public static Def GeneratePathDef(string a_path)
        {
            int index;
            if (s_defPathLookup.TryGetValue(a_path, out index))
            {
                return s_createDef(s_defs[index], s_defs);
            }

            return null;
        }
        public static T GeneratePathDef<T>(string a_path) where T : Def
        {
            return GeneratePathDef(a_path) as T;
        }

        static void SetData(ref DefDataObject a_dataObject, object a_value, object a_default, Type a_type)
        {
            a_dataObject.Text = string.Empty;
            
            if (object.Equals(a_value, a_default))
            {
                return;    
            }

            if (a_type == typeof(Def) || a_type.IsSubclassOf(typeof(Def)))
            {
                Def def = a_value as Def;
                a_dataObject.Text = def.DefName;

                return;
            }
            else if (a_type == typeof(string))
            {
                string val = a_value as string;
                string normVal = a_default as string;

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
                    a_dataObject.Text = val;
                }

                return;
            }
            else if (a_type.IsGenericType && a_type.GetGenericTypeDefinition() == typeof(List<>) && a_value != null)
            {
                Type gType = a_type.GetGenericArguments()[0];

                PropertyInfo countProp = a_type.GetProperty("Count");

                int count = (int)countProp.GetValue(a_value);
                int pCount = 0;
                if (a_default != null)
                {
                    pCount = (int)countProp.GetValue(a_default);
                }

                if (count > pCount)
                {
                    IList list = a_value as IList;

                    for (int i = pCount; i < count; ++i)
                    {
                        DefDataObject child = new DefDataObject();
                        child.Name = "lv";
                        child.Children = new List<DefDataObject>();

                        SetData(ref child, list[i], null, gType);

                        if (!string.IsNullOrWhiteSpace(child.Text) || child.Children.Count > 0)
                        {
                            a_dataObject.Children.Add(child);
                        }
                    }
                }

                return;
            }
            else if (a_type.IsArray)
            {
                Type gType = a_type.GetElementType();

                Array array = a_value as Array;
                if (array != null)
                {
                    int len = array.Length;
                    for (int i = 0; i < len; ++i)
                    {
                        DefDataObject child = new DefDataObject();
                        child.Name = "lv";
                        child.Children = new List<DefDataObject>();

                        SetData(ref child, array.GetValue(i), null, gType);

                        if (!string.IsNullOrWhiteSpace(child.Text) || child.Children.Count > 0)
                        {
                            a_dataObject.Children.Add(child);
                        }
                    }
                }

                return;
            }
            else if (a_type.IsSubclassOf(typeof(Enum)))
            {
                a_dataObject.Text = a_value.ToString();

                return;
            }

            FieldInfo[] fields = a_type.GetFields(BindingFlags.Instance | BindingFlags.Public);
            if (fields.Length == 0)
            {
                a_dataObject.Text = a_value.ToString();
            }
            else
            {
                if (a_value != null)
                {
                    foreach (FieldInfo field in fields)
                    {
                        if (field.IsStatic || field.IsLiteral)
                        {
                            continue;
                        }

                        if (field.GetCustomAttribute<NonSerializedAttribute>() != null)
                        {
                            continue;
                        }

                        object value = field.GetValue(a_value);
                        object defaultValue = null;
                        if (a_default != null)
                        {
                            defaultValue = field.GetValue(a_default);
                        }

                        DefDataObject dataObject = new DefDataObject();
                        dataObject.Name = field.Name;
                        dataObject.Children = new List<DefDataObject>();

                        SetData(ref dataObject, value, defaultValue, field.FieldType);

                        if (!string.IsNullOrWhiteSpace(dataObject.Text) || dataObject.Children.Count > 0)
                        {
                            a_dataObject.Children.Add(dataObject);
                        }
                    }
                }
            }
        }

        static DefData GenerateData(Def a_def)
        {
            Type defType = a_def.GetType();

            object baseDef = Activator.CreateInstance(defType);
            if (!string.IsNullOrWhiteSpace(a_def.DefParentName))
            {
                baseDef = GenerateDef(a_def.DefParentName);
            }

            string typeName = defType.ToString();
            if (typeName.StartsWith(DefintionNamespace))
            {
                typeName = typeName.Substring(DefintionNamespace.Length + 1);
            }

            DefData data = new DefData()
            {
                Type = typeName,
                Name = a_def.DefName,
                Parent = a_def.DefParentName,
                Path = a_def.DefPath,
                Abstract = false,
            };
            data.DefDataObjects = new List<DefDataObject>();

            Type baseType = baseDef.GetType();

            FieldInfo[] fields = defType.GetFields(BindingFlags.Instance | BindingFlags.Public);
            foreach (FieldInfo field in fields)
            {
                if (field.IsStatic || field.IsLiteral)
                {
                    continue;
                }

                if (field.GetCustomAttribute<NonSerializedAttribute>() != null)
                {
                    continue;
                }

                object value = field.GetValue(a_def);
                object defaultValue = null;
                if (baseType.GetField(field.Name) != null)
                {
                    defaultValue = field.GetValue(baseDef);
                }

                DefDataObject dataObject = new DefDataObject();
                dataObject.Name = field.Name;
                dataObject.Children = new List<DefDataObject>();

                SetData(ref dataObject, value, defaultValue, field.FieldType);

                if (!string.IsNullOrWhiteSpace(dataObject.Text) || dataObject.Children.Count > 0)
                {                    
                    data.DefDataObjects.Add(dataObject);
                }
            }

            return data;
        }

        internal static bool AddDef(Def a_def)
        {
            if (a_def == null)
            {
                Logger.Warning("Cannot add null def");

                return false;
            }

            DefData data = GenerateData(a_def);

            int count = s_defs.Count;
            for (int i = 0; i < count; ++i)
            {
                if (string.IsNullOrEmpty(s_defs[i].Name))
                {
                    s_defs[i] = data;
                    s_defPathLookup.Add(data.Path, i);
                    s_defNameLookup.Add(data.Name, i);

                    return true;
                }
            }

            s_defs.Add(data);
            s_defPathLookup.Add(data.Path, count);
            s_defNameLookup.Add(data.Name, count);

            return true;
        }

        internal static bool AddSceneDef(Def a_def)
        {
            if (a_def == null)
            {
                Logger.Warning("Cannot add null def");

                return false;
            }

            DefData data = GenerateData(a_def);

            int count = s_defs.Count;
            for (int i = 0; i < count; ++i)
            {
                if (string.IsNullOrEmpty(s_defs[i].Name))
                {
                    s_defs[i] = data;
                    s_sceneDefLookup.Add(data.Name, i);

                    return true;
                }
            }

            s_defs.Add(data);
            s_sceneDefLookup.Add(data.Name, count);

            return true;
        }
        internal static bool RemoveSceneDef(string a_defName)
        {
            int index;
            if (s_sceneDefLookup.TryGetValue(a_defName, out index))
            {
                s_defs[index] = new DefData();
                s_sceneDefLookup.Remove(a_defName);

                return true;
            }

            return false;
        }

        public static void RebuildDefData(Def a_def)
        {
            if (a_def == null)
            {
                Logger.Warning("Cannot rebuild null def");

                return;
            }        

            int index;
            if (s_defNameLookup.TryGetValue(a_def.DefName, out index))
            {
                s_defs[index] = GenerateData(a_def);
            }
            else if (s_sceneDefLookup.TryGetValue(a_def.DefName, out index))
            {
                s_defs[index] = GenerateData(a_def);
            }
            else
            {
                Logger.Error("Cannot find def to rebuild");
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