// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEditor.Properties;
using IcarianEngine;
using IcarianEngine.Definitions;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace IcarianEditor.Windows
{
    public static class PropertiesWindow
    {
        static PropertiesEditorWindow                   s_defaultWindow;

        static Dictionary<Type, PropertiesEditorWindow> s_windows;

        internal static void Init()
        {
            s_defaultWindow = new PropertiesEditorWindow();
            s_windows = new Dictionary<Type, PropertiesEditorWindow>();

            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();

            foreach (Assembly asm in assemblies)
            {
                Type[] types = asm.GetTypes();
                foreach (Type type in types)
                {
                    PWindowAttribute att = type.GetCustomAttribute<PWindowAttribute>();
                    if (att != null)
                    {
                        if (type.IsSubclassOf(typeof(PropertiesEditorWindow)))
                        {
                            PropertiesEditorWindow window = Activator.CreateInstance(type) as PropertiesEditorWindow;
                            if (window != null)
                            {
                                s_windows.Add(att.OverrideType, window);
                            }
                            else
                            {
                                Logger.Error($"IcarianEditorCS: Unabled to create PropertiesEditorWindow of type {type}");
                            }
                        }
                        else
                        {
                            Logger.Error($"IcarianEditorCS: {type} has Attribute PWindow and is not inherited from PropertiesEditorWindow");
                        }
                    }
                }
            }
        }

        static void DisplayGUI(object a_object, bool a_sceneObject)
        {
            Type type = a_object.GetType();
            if (s_windows.ContainsKey(type))
            {
                s_windows[type].OnGUI(a_object, a_sceneObject);
            }
            else
            {
                s_defaultWindow.OnGUI(a_object, a_sceneObject);
            }
        }

        static void OnGUI()
        {
            // TODO: Implement multi selection at some point
            if (!Workspace.IsSelectionEmpty)
            {
                DisplayGUI(Workspace.Selection.First(), false);
            }
            else if (!Workspace.IsDefSelectionEmpty)
            {
                string defName = Workspace.SelectedDefs.First();
                if (!string.IsNullOrWhiteSpace(defName))
                {
                    Def def = EditorDefLibrary.GenerateDef(defName);

                    if (def != null)
                    {
                        DisplayGUI(def, def.IsSceneDef);

                        EditorDefLibrary.RebuildDefData(def);
                    }
                    else
                    {
                        Logger.Error($"IcarianEditorCS: Unable to generate def {defName}");
                    }
                }
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