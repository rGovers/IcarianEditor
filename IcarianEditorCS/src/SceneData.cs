using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;
using System.Xml;

namespace IcarianEditor
{
    public static class SceneData
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void WriteScene(string a_path, byte[] a_data);

        static Dictionary<string, Scene> m_scenes;

        internal static void Init()
        {
            m_scenes = new Dictionary<string, Scene>();
        }

        public static Scene GetScene(string a_path)
        {
            if (m_scenes.ContainsKey(a_path))
            {
                return m_scenes[a_path];
            }

            Logger.Error($"IcarianEditorCS: Could not find scene {a_path}");

            return null;
        }

        static void LoadScenes(byte[][] a_data, string[] a_paths)
        {
            uint count = (uint)a_paths.LongLength;   

            for (uint i = 0; i < count; ++i)
            {
                string path = a_paths[i];

                MemoryStream stream = new MemoryStream(a_data[i]);

                XmlDocument doc = new XmlDocument();
                doc.Load(stream);

                if (m_scenes.ContainsKey(path))
                {
                    Scene s = m_scenes[path];
                    if (s != null && !s.IsDisposed)
                    {
                        s.Dispose();
                    }

                    m_scenes[path] = new Scene(doc);
                }
                else
                {
                    m_scenes.Add(path, new Scene(doc));
                }
            }
        }

        static void SerializeVector(string a_name, XmlDocument a_doc, XmlElement a_parent, Vector3 a_vec, Vector3 a_default)
        {
            if (a_vec != a_default)
            {
                XmlElement vecElement = a_doc.CreateElement(a_name);
                a_parent.AppendChild(vecElement);

                if (a_vec.X != a_default.X)
                {
                    XmlElement xElement = a_doc.CreateElement("X");
                    vecElement.AppendChild(xElement);
                    xElement.InnerText = a_vec.X.ToString();
                }
                if (a_vec.Y != a_default.Y)
                {
                    XmlElement yElement = a_doc.CreateElement("Y");
                    vecElement.AppendChild(yElement);
                    yElement.InnerText = a_vec.Y.ToString();
                }
                if (a_vec.Z != a_default.Z)
                {
                    XmlElement zElement = a_doc.CreateElement("Z");
                    vecElement.AppendChild(zElement);
                    zElement.InnerText = a_vec.Z.ToString();
                }
            }
        }
        static void SerializeQuaternion(string a_name, XmlDocument a_doc, XmlElement a_parent, Quaternion a_quat)
        {
            if (a_quat != Quaternion.Identity)
            {
                XmlElement quatElement = a_doc.CreateElement(a_name);
                a_parent.AppendChild(quatElement);

                if (a_quat.X != 0)
                {
                    XmlElement xElement = a_doc.CreateElement("X");
                    quatElement.AppendChild(xElement);
                    xElement.InnerText = a_quat.X.ToString();
                }
                if (a_quat.Y != 0)
                {
                    XmlElement yElement = a_doc.CreateElement("Y");
                    quatElement.AppendChild(yElement);
                    yElement.InnerText = a_quat.Y.ToString();
                }
                if (a_quat.Z != 0)
                {
                    XmlElement zElement = a_doc.CreateElement("Z");
                    quatElement.AppendChild(zElement);
                    zElement.InnerText = a_quat.Z.ToString();
                }
                if (a_quat.W != 1)
                {
                    XmlElement wElement = a_doc.CreateElement("W");
                    quatElement.AppendChild(wElement);
                    wElement.InnerText = a_quat.W.ToString();
                }
            }
        }

        static void Serialize()
        {
            string path = Workspace.CurrentScenePath;

            Scene scene = GetScene(path);
            if (scene == null)
            {
                return;
            }

            XmlDocument doc = new XmlDocument();
            doc.AppendChild(doc.CreateXmlDeclaration("1.0", "utf-16", null));

            XmlElement rootElement = doc.CreateElement("Scene");
            doc.AppendChild(rootElement);

            XmlElement objectsElement = doc.CreateElement("Objects");
            rootElement.AppendChild(objectsElement);

            foreach (SceneObjectData obj in Workspace.SceneObjectList)
            {
                XmlElement objElement = doc.CreateElement("GameObject");
                objectsElement.AppendChild(objElement);

                SerializeVector("Translation", doc, objElement, obj.Object.Translation, Vector3.Zero);
                SerializeQuaternion("Rotation", doc, objElement, obj.Object.Rotation);
                SerializeVector("Scale", doc, objElement, obj.Object.Scale, Vector3.One);

                XmlElement defNameElement = doc.CreateElement("DefName");
                objElement.AppendChild(defNameElement);
                defNameElement.InnerText = obj.Object.DefName;
            }

            XmlElement defsElement = doc.CreateElement("Defs");
            rootElement.AppendChild(defsElement);

            foreach (Def def in Workspace.SceneDefs)
            {
                defsElement.AppendChild(AssetProperties.SerializeDef(doc, def));
            }

            MemoryStream stream = new MemoryStream();
            XmlTextWriter writer = new XmlTextWriter(stream, Encoding.Unicode);
            writer.Formatting = Formatting.Indented;

            doc.WriteContentTo(writer);

            writer.Flush();
            stream.Flush();

            stream.Position = 0;

            StreamReader reader = new StreamReader(stream);
            byte[] bytes = Encoding.Unicode.GetBytes(reader.ReadToEnd());

            WriteScene(path, bytes);

            // Scene is immutable so generate new one
            m_scenes[path].Dispose();
            m_scenes[path] = new Scene(doc);
        }
    }
}