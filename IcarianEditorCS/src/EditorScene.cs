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
    public struct GameObjectData
    {
        public ulong ID;
        public SceneObject Object;
        public string DefName;
    }
    public struct SceneObjectData
    {
        public bool Visible;
        public ulong ID;
        public SceneObject Object;
    }

    public class EditorScene
    {
        static Dictionary<string, EditorScene> s_scenes = new Dictionary<string, EditorScene>();

        string                m_path;
        List<SceneObjectData> m_sceneObjects;
        List<GameObjectData>  m_gameObjects;
        List<string>          m_defNames;

        public IEnumerable<SceneObjectData> SceneObjects
        {
            get
            {
                return m_sceneObjects;
            }
        }
        public IEnumerable<GameObjectData> GameObjects
        {
            get
            {
                return m_gameObjects;
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
            m_gameObjects = new List<GameObjectData>();
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
        public ulong GetID(SceneObject a_sceneObject, string a_defName)
        {
            foreach (GameObjectData dat in m_gameObjects)
            {
                if (dat.Object == a_sceneObject && dat.DefName == a_defName)
                {
                    return dat.ID;
                }
            }

            return ulong.MaxValue;
        }
        public ulong GetID(SceneObject a_sceneObject, GameObjectDef a_def)
        {
            string defName = a_def.DefName;
            foreach (GameObjectData dat in m_gameObjects)
            {
                if (dat.Object == a_sceneObject && dat.DefName == defName)
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

                GetObjects(obj, def);
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

        public void SetVisible(ulong a_id, bool a_state)
        {
            int count = m_sceneObjects.Count;
            for (int i = 0; i < count; ++i)
            {
                SceneObjectData dat = m_sceneObjects[i];
                if (dat.ID == a_id)
                {
                    dat.Visible = a_state;

                    m_sceneObjects[i] = dat;

                    return;
                }
            }
        }

        void GetObjects(SceneObject a_object, GameObjectDef a_def)
        {
            if (a_def == null)
            {
                return;
            }

            GameObjectData dat = new GameObjectData()
            {
                ID = Workspace.NewID(),
                Object = a_object,
                DefName = a_def.DefName
            };

            m_gameObjects.Add(dat);

            foreach (GameObjectDef child in a_def.Children)
            {
                GetObjects(a_object, child);
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
            SceneObjectData dat = new SceneObjectData()
            {
                Visible = true,
                ID = Workspace.NewID(),
                Object = obj
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
                        Logger.Error($"IcarianEditorCS: Invalid scene object element {element.Name}");

                        break;
                    }
                    }
                }
            }

            if (!string.IsNullOrEmpty(obj.DefName))
            {
                m_sceneObjects.Add(dat);
            }
            else
            {
                Logger.Error($"IcarianEditorCS: Scene object has no def name");
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

            foreach (SceneObjectData dat in scene.m_sceneObjects)
            {
                SceneObject obj = dat.Object;

                GameObjectDef def = EditorDefLibrary.GenerateDef<GameObjectDef>(obj.DefName);
                if (def != null)
                {
                    scene.GetObjects(obj, def);
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