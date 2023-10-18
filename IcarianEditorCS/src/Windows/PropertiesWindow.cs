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

        const string DefintionNamespace = "IcarianEngine.Definitions.";

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
            // TODO: Implement multi selection at some point
            if (!Workspace.IsSelectionEmpty)
            {
                DisplayGUI(Workspace.Selection.First());
            }
            else if (!Workspace.IsDefSelectionEmpty)
            {
                string defName = Workspace.SelectedDefs.First();
                if (!string.IsNullOrWhiteSpace(defName))
                {
                    Def def = EditorDefLibrary.GenerateDef(defName);

                    if (def != null)
                    {
                        DisplayGUI(def);

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