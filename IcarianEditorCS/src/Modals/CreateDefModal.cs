using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.IO;
using System.Text;
using System.Xml;

#include "EditorCreateDefModalInterop.h"
#include "InteropBinding.h"

EDITOR_CREATEDEFMODAL_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEditor.Modals
{
    public class CreateDefModal : Modal
    {
        struct DefData
        {
            public string Name;
            public Type Type;
        };

        string        m_path;
        string        m_name;
        int           m_selectedItem;
        List<DefData> m_defData;

        CreateDefModal(string a_path) : base("Create Def", new Vector2(250.0f, 110.0f))
        {
            m_path = a_path;

            m_selectedItem = 0;
            m_defData = new List<DefData>();

            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            foreach (Assembly asm in assemblies)
            {
                Type[] types = asm.GetTypes();
                foreach (Type t in types)
                {
                    if (t.IsAbstract)
                    {
                        continue;
                    }

                    if (t.IsSubclassOf(typeof(Def)))
                    {
                        string name = t.FullName;
                        if (t.Namespace == EditorDefLibrary.DefintionNamespace)
                        {
                            name = t.Name;
                        }

                        DefData dat = new DefData()
                        {
                            Name = name,
                            Type = t
                        };

                        m_defData.Add(dat);
                    }
                }
            }
        }

        public override bool Update()
        {
            GUI.StringField("Def Name", ref m_name);

            int count = m_defData.Count;
            string[] defTypes = new string[count];
            for (int i = 0; i < count; ++i)
            {
                defTypes[i] = m_defData[i].Name;
            }

            GUI.StringSelector("Def Type", defTypes, ref m_selectedItem);

            if (GUI.Button("Ok"))
            {
                if (string.IsNullOrWhiteSpace(m_name))
                {
                    Logger.Error("Invalid Def name");

                    return true;
                }

                if (m_selectedItem < 0 || m_selectedItem >= count)
                {   
                    Logger.Error("Selected item out of bounds");

                    return true;
                }

                Def def = Activator.CreateInstance(m_defData[m_selectedItem].Type) as Def;
                if (def == null)
                {
                    Logger.Error("Failed to create Def");

                    return true;
                }

                string finalPath = Path.Combine(m_path, $"{m_name}.def");

                def.DefName = m_name;
                def.DefPath = finalPath;

                EditorDefLibrary.AddDef(def);

                XmlDocument doc = new XmlDocument();
                doc.AppendChild(doc.CreateXmlDeclaration("1.0", "UTF-8", null));

                XmlElement root = EditorDefLibrary.CreateDef(doc, m_name);
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

                CreateDefModalInterop.CreateDef(finalPath, data);

                return false;
            }

            GUI.SameLine();

            if (GUI.Button("Cancel"))
            {
                return false;
            }

            return true;
        }

        static void Create(string a_path)
        {
            new CreateDefModal(a_path);
        }
    }
}