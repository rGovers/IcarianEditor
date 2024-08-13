// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;
using System.Xml;

#include "InteropBinding.h"
#include "EditorSceneInterop.h"

EDITORSCENE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEditor
{
    public struct SceneObjectData
    {
        public bool Visible;
        public ulong ID;
        public SceneObject Object;
    }
    public struct SceneObjectArrayData
    {
        public bool Visible;
        public ulong ID;
        public SceneObjectArray Array;
    }

    public class EditorScene
    {
        static Dictionary<string, EditorScene> s_scenes = new Dictionary<string, EditorScene>();

        string                     m_path;
        List<SceneObjectData>      m_sceneObjects;
        List<SceneObjectArrayData> m_sceneObjectArrays;
        List<string>               m_defNames;

        public IEnumerable<SceneObjectData> SceneObjects
        {
            get
            {
                return m_sceneObjects;
            }
        }
        public IEnumerable<SceneObjectArrayData> SceneObjectArrays
        {
            get
            {
                return m_sceneObjectArrays;
            }
        }

        public IEnumerable<string> DefNames
        {
            get
            {
                return m_defNames;
            }
        }

        internal EditorScene(string a_path)
        {
            m_path = a_path;
            m_sceneObjects = new List<SceneObjectData>();
            m_sceneObjectArrays = new List<SceneObjectArrayData>();
            m_defNames = new List<string>();
        }

        public ulong GetID(SceneObject a_sceneObject)
        {
            foreach (SceneObjectData dat in m_sceneObjects)
            {
                if (dat.Object == a_sceneObject)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }
        public ulong GetID(SceneObjectArray a_sceneObjectArray)
        {
            foreach (SceneObjectArrayData dat in m_sceneObjectArrays)
            {
                if (dat.Array == a_sceneObjectArray)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }

        public SceneObject GetSceneObject(ulong a_id)
        {
            foreach (SceneObjectData dat in m_sceneObjects)
            {
                if (dat.ID == a_id)
                {
                    return dat.Object;
                }
            }

            return null;
        }
        public SceneObjectArray GetSceneObjectArray(ulong a_id)
        {
            foreach (SceneObjectArrayData dat in m_sceneObjectArrays)
            {
                if (dat.ID == a_id)
                {
                    return dat.Array;
                }
            }

            return null;
        }

        public void AddDef(Def a_def)
        {
            if (EditorDefLibrary.AddSceneDef(a_def))
            {
                m_defNames.Add(a_def.DefName);
            }
        }
        public void RemoveDef(string a_defName)
        {
            if (EditorDefLibrary.RemoveSceneDef(a_defName))
            {
                m_defNames.Remove(a_defName);
            }
        }

        public void AddSceneObject(string a_defName)
        {
            AddSceneObject(a_defName, Vector3.Zero, Quaternion.Identity, Vector3.One);
        }
        public void AddSceneObject(string a_defName, Vector3 a_translation, Quaternion a_rotation, Vector3 a_scale)
        {
            GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(a_defName);
            if (def != null)
            {
                SceneObject obj = new SceneObject()
                {
                    Translation = a_translation,
                    Rotation = a_rotation,
                    Scale = a_scale,
                    DefName = a_defName
                };
                SceneObjectData dat = new SceneObjectData()
                {
                    Visible = true,
                    ID = Workspace.NewID(),
                    Object = obj
                };

                m_sceneObjects.Add(dat);
            }
        }
        public void RemoveSceneObject(ulong a_id)
        {
            foreach (SceneObjectData dat in m_sceneObjects)
            {
                if (dat.ID == a_id)
                {
                    m_sceneObjects.Remove(dat);

                    break;
                }
            }
        }

        public void AddSceneObjectArray(string a_defName)
        {
            AddSceneObjectArray(a_defName, Vector3.Zero, Quaternion.Identity);
        }
        public void AddSceneObjectArray(string a_defName, Vector3 a_translation, Quaternion a_rotation)
        {
            GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(a_defName);
            if (def != null)
            {
                SceneObjectArray array = new SceneObjectArray()
                {
                    Translation = a_translation,
                    Rotation = a_rotation,
                    Count = IVector3.One,
                    Spacing = Vector3.One,
                    DefName = a_defName 
                };
                SceneObjectArrayData dat = new SceneObjectArrayData()
                {
                    Visible = true,
                    ID = Workspace.NewID(),
                    Array = array
                };

                m_sceneObjectArrays.Add(dat);
            }
        }
        public void RemoveSceneObjectArray(ulong a_id)
        {
            foreach (SceneObjectArrayData dat in m_sceneObjectArrays)
            {
                if (dat.ID == a_id)
                {
                    m_sceneObjectArrays.Remove(dat);

                    break;
                }
            }
        }

        public void SetVisible(ulong a_id, bool a_state)
        {
            int objectCount = m_sceneObjects.Count;
            for (int i = 0; i < objectCount; ++i)
            {
                SceneObjectData dat = m_sceneObjects[i];
                if (dat.ID == a_id)
                {
                    dat.Visible = a_state;

                    m_sceneObjects[i] = dat;

                    return;
                }
            }

            int arrayCount = m_sceneObjectArrays.Count;
            for (int i = 0; i < arrayCount; ++i)
            {
                SceneObjectArrayData dat = m_sceneObjectArrays[i];
                if (dat.ID == a_id)
                {
                    dat.Visible = a_state;

                    m_sceneObjectArrays[i] = dat;

                    return;
                }
            }
        }

        void LoadSceneObject(XmlElement a_element)
        {
            SceneObject obj = new SceneObject()
            {
                Translation = Vector3.Zero,
                Rotation = Quaternion.Identity,
                Scale = Vector3.One
            };

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "Translation":
                    {
                        obj.Translation = element.ToVector3();

                        break;
                    }
                    case "Rotation":
                    {
                        obj.Rotation = element.ToQuaternion();

                        break;
                    }
                    case "AxisAngle":
                    {
                        Vector4 rot = element.ToVector4(Vector4.Zero);

                        obj.Rotation = Quaternion.FromAxisAngle(Vector3.Normalized(rot.XYZ), rot.W);

                        break;
                    }
                    case "Scale":
                    {
                        obj.Scale = element.ToVector3(Vector3.One);

                        break;
                    }
                    case "DefName":
                    {
                        obj.DefName = element.InnerText;

                        break;
                    }
                    default:
                    {
                        Logger.Error($"IcarianEditorCS: Invalid SceneObject element: {element.Name}");

                        break;
                    }
                    }
                }
            }

            if (!string.IsNullOrWhiteSpace(obj.DefName))
            {
                m_sceneObjects.Add(new SceneObjectData()
                {
                    Visible = true,
                    ID = Workspace.NewID(),
                    Object = obj
                });
            }
            else
            {
                Logger.Error($"IcarianEditorCS: SceneObject has no Def name");
            }
        }
        void LoadSceneObjectArray(XmlElement a_element)
        {
            SceneObjectArray arr = new SceneObjectArray()
            {
                Translation = Vector3.Zero,
                Rotation = Quaternion.Identity,
                Count = IVector3.One,
                Spacing = Vector3.One
            };

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "Translation":
                    {
                        arr.Translation = element.ToVector3();

                        break;
                    }
                    case "Rotation":
                    {
                        arr.Rotation = element.ToQuaternion();

                        break;
                    }
                    case "AxisAngle":
                    {
                        Vector4 rot = element.ToVector4(Vector4.Zero);

                        arr.Rotation = Quaternion.FromAxisAngle(Vector3.Normalized(rot.XYZ), rot.W);

                        break;
                    }
                    case "Count":
                    {
                        arr.Count = element.ToIVector3(IVector3.One);

                        break;
                    }
                    case "Spacing":
                    {
                        arr.Spacing = element.ToVector3(Vector3.One);

                        break;
                    }
                    case "DefName":
                    {
                        arr.DefName = element.InnerText;

                        break;
                    }
                    default:
                    {
                        Logger.Error($"IcarianEditorCS: Invalid SceneObjectArray element: {element.Name}");

                        break;
                    }
                    }
                }
            }

            if (!string.IsNullOrWhiteSpace(arr.DefName))
            {
                m_sceneObjectArrays.Add(new SceneObjectArrayData()
                {
                    Visible = true,
                    ID = Workspace.NewID(),
                    Array = arr
                });
            }
            else
            {
                Logger.Error($"IcarianEditorCS: SceneObjectArray has no Def name");
            }
        }

        void LoadObjects(XmlElement a_element)
        {
            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "GameObject":
                    {
                        LoadSceneObject(element);

                        break;
                    }
                    case "GameObjectArray":
                    {   
                        LoadSceneObjectArray(element);

                        break;
                    }
                    default:
                    {
                        Logger.Error($"IcarianEditorCS: Invalid object element {element.Name}");

                        break;
                    }
                    }
                }
            }
        }

        void LoadDefs(XmlElement a_element)
        {
            IEnumerable<string> defNames = EditorDefLibrary.LoadSceneDefs(a_element);

            foreach (string defName in defNames)
            {
                m_defNames.Add(defName);
            }
        }

        internal static EditorScene GetScene(string a_path)
        {
            if (s_scenes.TryGetValue(a_path, out EditorScene scene))
            {
                return scene;
            }

            return null;
        }

        internal static EditorScene Load(XmlDocument a_doc, string a_path)
        {
            EditorScene scene = new EditorScene(a_path);

            if (a_doc.DocumentElement is XmlElement root)
            {
                foreach (XmlNode node in root.ChildNodes)
                {
                    if (node is XmlElement element)
                    {
                        switch (element.Name)
                        {
                        case "Objects":
                        {
                            scene.LoadObjects(element);

                            break;
                        }
                        case "Defs":
                        {
                            scene.LoadDefs(element);

                            break;
                        }
                        default:
                        {
                            Logger.Error($"IcarianEditorCS: Invalid scene element {element.Name}");

                            break;
                        }
                        }
                    }
                }
            }

            s_scenes.Add(a_path, scene);

            return scene;
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

                Load(doc, path);
            }
        }
        static void Serialize()
        {
            EditorScene scene = Workspace.GetScene();
            if (scene == null)
            {
                return;
            }

            XmlDocument doc = new XmlDocument();
            doc.AppendChild(doc.CreateXmlDeclaration("1.0", "UTF-8", null));

            XmlElement root = doc.CreateElement("Scene");
            doc.AppendChild(root);

            XmlElement objects = doc.CreateElement("Objects");
            root.AppendChild(objects);

            foreach (SceneObjectData dat in scene.m_sceneObjects)
            {
                SceneObject obj = dat.Object;

                XmlElement element = doc.CreateElement("GameObject");
                objects.AppendChild(element);

                XmlElement translation = obj.Translation.ToXml(doc, "Translation");
                if (translation != null)
                {
                    element.AppendChild(translation);
                }
                XmlElement rotation = obj.Rotation.ToXml(doc, "Rotation");
                if (rotation != null)
                {
                    element.AppendChild(rotation);
                }
                XmlElement scale = obj.Scale.ToXml(doc, "Scale", Vector3.One);
                if (scale != null)
                {
                    element.AppendChild(scale);
                }

                XmlElement defName = doc.CreateElement("DefName");
                defName.InnerText = obj.DefName;
                element.AppendChild(defName);
            }

            foreach (SceneObjectArrayData dat in scene.m_sceneObjectArrays)
            {
                SceneObjectArray arr = dat.Array;

                XmlElement element = doc.CreateElement("GameObjectArray");
                objects.AppendChild(element);

                XmlElement translation = arr.Translation.ToXml(doc, "Translation");
                if (translation != null)
                {
                    element.AppendChild(translation);
                }
                XmlElement rotation = arr.Rotation.ToXml(doc, "Rotation");
                if (rotation != null)
                {
                    element.AppendChild(rotation);
                }
                XmlElement count = arr.Count.ToXml(doc, "Count", IVector3.One);
                if (count != null)
                {
                    element.AppendChild(count);
                }
                XmlElement spacing = arr.Spacing.ToXml(doc, "Spacing", Vector3.One);
                if (spacing != null)
                {
                    element.AppendChild(spacing);
                }

                XmlElement defName = doc.CreateElement("DefName");
                defName.InnerText = arr.DefName;
                element.AppendChild(defName);
            }

            XmlElement defs = doc.CreateElement("Defs");
            root.AppendChild(defs);

            foreach (string defName in scene.m_defNames)
            {
                XmlElement def = EditorDefLibrary.CreateDef(doc, defName, true);
                if (def != null)
                {
                    defs.AppendChild(def);
                }
            }

            MemoryStream stream = new MemoryStream();
            XmlTextWriter writer = new XmlTextWriter(stream, System.Text.Encoding.UTF8);
            writer.Formatting = Formatting.Indented;

            doc.WriteContentTo(writer);

            writer.Flush();
            stream.Flush();

            stream.Position = 0;

            StreamReader reader = new StreamReader(stream);
            byte[] data = Encoding.UTF8.GetBytes(reader.ReadToEnd());

            EditorSceneInterop.WriteScene(scene.m_path, data);
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