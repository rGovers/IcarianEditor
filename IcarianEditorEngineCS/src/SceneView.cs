// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Lighting;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

#include "EditorLightInteropStructures.h"
#include "EditorDrawInteropStructures.h"

namespace IcarianEditor.Engine
{
    public struct MeshTable
    {
        public uint MaterialID;
        public uint MeshID;
        public uint IndexCount;
        public List<GameObject> GameObjects;
    }
    public struct ModelTable
    {
        public uint MaterialID;
        public uint ModelID;
        public List<GameObject> GameObjects;
    }

    public struct DrawMeshData
    {
        public uint MaterialID;
        public uint MeshID;
        public uint IndexCount;
        public List<Matrix4> Transforms;
    }
    public struct DrawModelData
    {
        public uint MaterialID;
        public uint ModelID;
        public List<Matrix4> Transforms;
    }

    public static class SceneView
    {
        static GameObject             s_camObject;

        static EditorLightMode        s_lightMode;

        static AmbientLight           s_staticLight;
        static DirectionalLight       s_viewportLight;

        static List<AmbientLight>     s_ambientLights;
        static List<DirectionalLight> s_directionalLights;
        static List<PointLight>       s_pointLights;
        static List<SpotLight>        s_spotLights;

        static List<MeshTable>         s_meshObjects;
        static List<ModelTable>        s_modelObjects;

        static NativeLock              s_ambientLightLock;
        static NativeLock              s_directionalLightLock;
        static NativeLock              s_pointLightLock;
        static NativeLock              s_spotLightLock;

        static NativeLock              s_meshLock;
        static NativeLock              s_modelLock;

        static void LightMode(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:LightMode")
            {
                return;
            }

            uint val = BitConverter.ToUInt32(a_data, 0);

            EditorLightMode newLightMode = (EditorLightMode)val;
            if (newLightMode == s_lightMode)
            {
                return;
            }

            s_lightMode = newLightMode;

            switch (s_lightMode)
            {
            case EditorLightMode.Ambient:
            {
                s_staticLight.Intensity = 1.0f;
                s_staticLight.RenderLayer = 0b1;

                s_viewportLight.Intensity = 0.0f;
                s_viewportLight.RenderLayer = 0;

                break;
            }
            case EditorLightMode.Viewport:
            {
                s_staticLight.Intensity = 0.25f;
                s_staticLight.RenderLayer = 0b1;

                s_viewportLight.Intensity = 1.0f;
                s_viewportLight.RenderLayer = 0b1;

                break;
            }
            case EditorLightMode.Scene:
            {
                s_staticLight.Intensity = 0.0f;
                s_staticLight.RenderLayer = 0;

                s_viewportLight.Intensity = 0.0f;
                s_viewportLight.RenderLayer = 0;

                break;
            }
            }
        }

        static void PushAmbientLight(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:PushAmbientLight")
            {
                return;
            }

            if (s_lightMode != EditorLightMode.Scene)
            {
                return;
            }

            if (a_data.Length < Marshal.SizeOf<LightHeader>())
            {
                return;
            }

            uint version = BitConverter.ToUInt32(a_data, 0);
            if (version != 0)
            {
                return;
            }

            LightHeader header = new LightHeader()
            {
                Version = version,
                DataCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataCount")),
                DataOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataOffset")),
            };

            uint ambientLightDataSize = (uint)Marshal.SizeOf<AmbientLightData>();
            uint dataSize = header.DataCount * ambientLightDataSize;
            if (a_data.Length < header.DataOffset + dataSize)
            {
                return;
            }

            AmbientLightData[] data = new AmbientLightData[header.DataCount];

            for (uint i = 0; i < header.DataCount; ++i)
            {
                uint offset = header.DataOffset + (i * ambientLightDataSize);

                uint colorOffset = (uint)Marshal.OffsetOf<AmbientLightData>("Color");

                Vector4 c = Vector4.Zero;
                for (uint j = 0; j < 4; ++j)
                {
                    c[j] = BitConverter.ToSingle(a_data, (int)(offset + colorOffset + (j * sizeof(float))));
                }

                AmbientLightData d = new AmbientLightData()
                {
                    Intensity = BitConverter.ToSingle(a_data, (int)(offset + (uint)Marshal.OffsetOf<AmbientLightData>("Intensity"))),
                    Color = c
                };

                data[i] = d;
            }

            s_ambientLightLock.WriteLock();

            uint currentCount = (uint)s_ambientLights.Count;

            if (header.DataCount > currentCount)
            {
                uint diff = header.DataCount - currentCount;

                GameObject[] objects = GameObject.BatchInstantiate(diff);

                foreach (GameObject obj in objects)
                {
                    s_ambientLights.Add(obj.AddComponent<AmbientLight>());
                }
            }
            else if (header.DataCount < currentCount)
            {
                uint diff = currentCount - header.DataCount;

                for (uint i = header.DataCount; i < currentCount; ++i)
                {
                    s_ambientLights[(int)i].GameObject.Dispose();
                }

                s_ambientLights.RemoveRange((int)header.DataCount, (int)diff);
            }

            for (uint i = 0; i < header.DataCount; ++i)
            {
                AmbientLightData d = data[i];
                AmbientLight light = s_ambientLights[(int)i];

                light.Intensity = d.Intensity;
                light.Color = d.Color.ToColor();
                light.RenderLayer = 0b1;
            }

            s_ambientLightLock.WriteUnlock();
        }
        static void PushDirectionalLight(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:PushDirectionalLight")
            {
                return;
            }

            if (s_lightMode != EditorLightMode.Scene)
            {
                return;
            }

            if (a_data.Length < Marshal.SizeOf<LightHeader>())
            {
                return;
            }

            uint version = BitConverter.ToUInt32(a_data, 0);
            if (version != 0)
            {
                return;
            }

            LightHeader header = new LightHeader()
            {
                Version = version,
                DataCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataCount")),
                DataOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataOffset")),
            };

            uint directionalLightDataSize = (uint)Marshal.SizeOf<DirectionalLightData>();
            uint dataSize = header.DataCount * directionalLightDataSize;
            if (a_data.Length < header.DataOffset + dataSize)
            {
                return;
            }

            DirectionalLightData[] data = new DirectionalLightData[header.DataCount];

            for (uint i = 0; i < header.DataCount; ++i)
            {
                uint offset = header.DataOffset + (i * directionalLightDataSize);

                uint transformOffset = (uint)Marshal.OffsetOf<DirectionalLightData>("Transform");

                float[] trans = new float[16];
                for (uint j = 0; j < 16; ++j)
                {
                    trans[j] = BitConverter.ToSingle(a_data, (int)(offset + transformOffset + (j * sizeof(float))));
                }

                uint colorOffset = (uint)Marshal.OffsetOf<DirectionalLightData>("Color");

                Vector4 c = Vector4.Zero;
                for (uint j = 0; j < 4; ++j)
                {
                    c[j] = BitConverter.ToSingle(a_data, (int)(offset + colorOffset + (j * sizeof(float))));
                }

                DirectionalLightData d = new DirectionalLightData()
                {
                    Intensity = BitConverter.ToSingle(a_data, (int)(offset + (uint)Marshal.OffsetOf<DirectionalLightData>("Intensity"))),
                    Color = c,
                    Transform = new Matrix4(trans)
                };

                data[i] = d;
            }

            s_directionalLightLock.WriteLock();

            uint currentCount = (uint)s_directionalLights.Count;

            if (header.DataCount > currentCount)
            {
                uint diff = header.DataCount - currentCount;

                GameObject[] objects = GameObject.BatchInstantiate(diff);

                foreach (GameObject obj in objects)
                {
                    s_directionalLights.Add(obj.AddComponent<DirectionalLight>());
                }
            }
            else if (header.DataCount < currentCount)
            {
                uint diff = currentCount - header.DataCount;

                for (uint i = header.DataCount; i < currentCount; ++i)
                {
                    s_directionalLights[(int)i].GameObject.Dispose();
                }

                s_directionalLights.RemoveRange((int)header.DataCount, (int)diff);
            }

            for (uint i = 0; i < header.DataCount; ++i)
            {
                DirectionalLightData d = data[i];
                DirectionalLight light = s_directionalLights[(int)i];

                light.Transform.SetMatrix(d.Transform);
                light.Intensity = d.Intensity;
                light.Color = d.Color.ToColor();
                light.RenderLayer = 0b1;
            }

            s_directionalLightLock.WriteUnlock();
        }
        static void PushPointLight(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:PushPointLight")
            {
                return;
            }

            if (s_lightMode != EditorLightMode.Scene)
            {
                return;
            }

            if (a_data.Length < Marshal.SizeOf<LightHeader>())
            {
                return;
            }

            uint version = BitConverter.ToUInt32(a_data, 0);
            if (version != 0)
            {
                return;
            }

            LightHeader header = new LightHeader()
            {
                Version = version,
                DataCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataCount")),
                DataOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataOffset")),
            };

            uint pointLightDataSize = (uint)Marshal.SizeOf<PointLightData>();
            uint dataSize = header.DataCount * pointLightDataSize;
            if (a_data.Length < header.DataOffset + dataSize)
            {
                return;
            }

            PointLightData[] data = new PointLightData[header.DataCount];

            for (uint i = 0; i < header.DataCount; ++i)
            {
                uint offset = header.DataOffset + (i * pointLightDataSize);

                uint transformOffset = (uint)Marshal.OffsetOf<PointLightData>("Transform");

                float[] trans = new float[16];
                for (uint j = 0; j < 16; ++j)
                {
                    trans[j] = BitConverter.ToSingle(a_data, (int)(offset + transformOffset + (j * sizeof(float))));
                }

                uint colorOffset = (uint)Marshal.OffsetOf<PointLightData>("Color");

                Vector4 c = Vector4.Zero;
                for (uint j = 0; j < 4; ++j)
                {
                    c[j] = BitConverter.ToSingle(a_data, (int)(offset + colorOffset + (j * sizeof(float))));
                }

                uint dataOffset = (uint)Marshal.OffsetOf<PointLightData>("Data");

                Vector2 pD = Vector2.Zero;
                for (uint j = 0; j < 2; ++j)
                {
                    pD[j] = BitConverter.ToSingle(a_data, (int)(offset + dataOffset + (j * sizeof(float))));
                }

                PointLightData d = new PointLightData()
                {
                    Data = pD,
                    Color = c,
                    Transform = new Matrix4(trans)
                };

                data[i] = d;
            }

            s_pointLightLock.WriteLock();

            uint currentCount = (uint)s_pointLights.Count;

            if (header.DataCount > currentCount)
            {
                uint diff = header.DataCount - currentCount;

                GameObject[] objects = GameObject.BatchInstantiate(diff);

                foreach (GameObject obj in objects)
                {
                    s_pointLights.Add(obj.AddComponent<PointLight>());
                }
            }
            else if (header.DataCount < currentCount)
            {
                uint diff = currentCount - header.DataCount;

                for (uint i = header.DataCount; i < currentCount; ++i)
                {
                    s_pointLights[(int)i].GameObject.Dispose();
                }

                s_pointLights.RemoveRange((int)header.DataCount, (int)diff);
            }

            for (uint i = 0; i < header.DataCount; ++i)
            {
                PointLightData d = data[i];
                PointLight light = s_pointLights[(int)i];

                light.Transform.SetMatrix(d.Transform);
                light.Intensity = d.Data.X;
                light.Radius = d.Data.Y;
                light.Color = d.Color.ToColor();
                light.RenderLayer = 0b1;
            }

            s_pointLightLock.WriteUnlock();
        }
        static void PushSpotLight(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:PushSpotLight")
            {
                return;
            }

            if (s_lightMode != EditorLightMode.Scene)
            {
                return;
            }

            if (a_data.Length < Marshal.SizeOf<LightHeader>())
            {
                return;
            }

            uint version = BitConverter.ToUInt32(a_data, 0);
            if (version != 0)
            {
                return;
            }

            LightHeader header = new LightHeader()
            {
                Version = version,
                DataCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataCount")),
                DataOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<LightHeader>("DataOffset")),
            };

            uint spotLightDataSize = (uint)Marshal.SizeOf<SpotLightData>();
            uint dataSize = header.DataCount * spotLightDataSize;
            if (a_data.Length < header.DataOffset + dataSize)
            {
                return;
            }

            SpotLightData[] data = new SpotLightData[header.DataCount];

            for (uint i = 0; i < header.DataCount; ++i)
            {
                uint offset = header.DataOffset + (i * spotLightDataSize);

                uint transformOffset = (uint)Marshal.OffsetOf<SpotLightData>("Transform");

                float[] trans = new float[16];
                for (uint j = 0; j < 16; ++j)
                {
                    trans[j] = BitConverter.ToSingle(a_data, (int)(offset + transformOffset + (j * sizeof(float))));
                }

                uint colorOffset = (uint)Marshal.OffsetOf<SpotLightData>("Color");

                Vector4 c = Vector4.Zero;
                for (uint j = 0; j < 4; ++j)
                {
                    c[j] = BitConverter.ToSingle(a_data, (int)(offset + colorOffset + (j * sizeof(float))));
                }

                uint dataOffset = (uint)Marshal.OffsetOf<SpotLightData>("Data");

                Vector4 sD = Vector4.Zero;
                for (uint j = 0; j < 4; ++j)
                {
                    sD[j] = BitConverter.ToSingle(a_data, (int)(offset + dataOffset + (j * sizeof(float))));
                }

                SpotLightData d = new SpotLightData()
                {
                    Data = sD,
                    Color = c,
                    Transform = new Matrix4(trans)
                };

                data[i] = d;
            }

            s_spotLightLock.WriteLock();

            uint currentCount = (uint)s_spotLights.Count;

            if (header.DataCount > currentCount)
            {
                uint diff = header.DataCount - currentCount;

                GameObject[] objects = GameObject.BatchInstantiate(diff);

                foreach (GameObject obj in objects)
                {
                    s_spotLights.Add(obj.AddComponent<SpotLight>());
                }
            }
            else if (header.DataCount < currentCount)
            {
                uint diff = currentCount - header.DataCount;

                for (uint i = header.DataCount; i < currentCount; ++i)
                {
                    s_spotLights[(int)i].GameObject.Dispose();
                }

                s_spotLights.RemoveRange((int)header.DataCount, (int)diff);
            }

            for (uint i = 0; i < header.DataCount; ++i)
            {
                SpotLightData d = data[i];
                SpotLight light = s_spotLights[(int)i];

                light.Transform.SetMatrix(d.Transform);
                light.Intensity = d.Data.X;
                light.Radius = d.Data.Y;
                light.InnerCutoffAngle = d.Data.Z;
                light.OuterCutoffAngle = d.Data.W;
                light.Color = d.Color.ToColor();
                light.RenderLayer = 0b1;
            }

            s_spotLightLock.WriteUnlock();
        }

        static List<DrawMeshData> GetMeshData(byte[] a_data)
        {
            if (a_data.Length < Marshal.SizeOf<DrawDataHeader>())
            {
                return null;
            }

            uint version = BitConverter.ToUInt32(a_data, 0);
            if (version != 0)
            {
                return null;
            }

            DrawDataHeader dataHeader = new DrawDataHeader()
            {
                Version = version,
                HeaderOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<DrawDataHeader>("HeaderOffset")),
                HeaderCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<DrawDataHeader>("HeaderCount")),
            };

            uint bufferHeaderSize = (uint)Marshal.SizeOf<DrawMeshBufferHeader>();
            uint headerDataSize = dataHeader.HeaderCount * bufferHeaderSize;
            if (a_data.Length < dataHeader.HeaderOffset + headerDataSize)
            {
                return null;
            }

            List<DrawMeshData> meshData = new List<DrawMeshData>();
            for (uint i = 0; i < dataHeader.HeaderCount; ++i)
            {
                uint headerOffset = dataHeader.HeaderOffset + (i * bufferHeaderSize);

                DrawMeshBufferHeader bufferHeader = new DrawMeshBufferHeader()
                {
                    MaterialID = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawMeshBufferHeader>("MaterialID"))),
                    MeshID = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawMeshBufferHeader>("MeshID"))),
                    IndexCount = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawMeshBufferHeader>("IndexCount"))),
                    TransformOffset = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawMeshBufferHeader>("TransformOffset"))),
                    TransformCount = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawMeshBufferHeader>("TransformCount"))),
                };

                if (bufferHeader.TransformCount <= 0)
                {
                    continue;
                }

                uint mat4Size = (uint)Marshal.SizeOf<Matrix4>();
                uint transformSize = bufferHeader.TransformCount * mat4Size;
                if (a_data.Length < bufferHeader.TransformOffset + transformSize)
                {
                    return null;
                }

                DrawMeshData data = new DrawMeshData()
                {
                    MaterialID = bufferHeader.MaterialID,
                    MeshID = bufferHeader.MeshID,
                    IndexCount = bufferHeader.IndexCount,
                    Transforms = new List<Matrix4>((int)bufferHeader.TransformCount)
                };

                for (uint j = 0; j < bufferHeader.TransformCount; ++j)
                {
                    uint transformOffset = bufferHeader.TransformOffset + (j * mat4Size);

                    float[] matData = new float[16];

                    for (uint k = 0; k < 16; ++k)
                    {
                        uint indexOffset = transformOffset + (k * sizeof(float));

                        matData[k] = BitConverter.ToSingle(a_data, (int)indexOffset);
                    }

                    data.Transforms.Add(new Matrix4(matData));
                }

                meshData.Add(data);
            }

            return meshData;
        }

        static void DrawMesh(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:DrawMesh")
            {
                return;
            }

            List<DrawMeshData> meshData = GetMeshData(a_data);
            if (meshData == null)
            {
                return;
            }

            s_meshLock.WriteLock();

            List<MeshTable> newTables = new List<MeshTable>();

            foreach (DrawMeshData dat in meshData)
            {
                Material mat = null;
                Mesh mesh = null;

                uint tableCount = (uint)s_meshObjects.Count;
                for (uint i = 0; i < tableCount; ++i)
                {
                    MeshTable t = s_meshObjects[(int)i];

                    if (t.MaterialID != dat.MaterialID)
                    {
                        continue;
                    }

                    if (t.MeshID != dat.MeshID)
                    {
                        continue;
                    }

                    if (t.IndexCount != dat.IndexCount)
                    {
                        continue;
                    }

                    uint tarObjCount = (uint)dat.Transforms.Count;
                    uint curObjCount = (uint)t.GameObjects.Count;

                    if (tarObjCount > curObjCount)
                    {
                        uint diff = tarObjCount - curObjCount;

                        mat = AssetStore.GetMaterial(dat.MaterialID);
                        if (mat == null)
                        {
                            goto NextMesh;
                        }

                        if (dat.MeshID != uint.MaxValue)
                        {
                            mesh = AssetStore.GetMesh(dat.MeshID);
                            if (mesh == null)
                            {
                                goto NextMesh;
                            }
                        }

                        GameObject[] objects = GameObject.BatchInstantiate(diff);
                        foreach (GameObject obj in objects)
                        {
                            MeshRenderer renderer = obj.AddComponent<MeshRenderer>();
                            renderer.Mesh = mesh;
                            renderer.IndexCount = dat.IndexCount;
                            renderer.Material = mat;
                        }

                        t.GameObjects.AddRange(objects);
                    }
                    else if (tarObjCount < curObjCount)
                    {
                        uint diff = curObjCount - tarObjCount;

                        for (uint j = tarObjCount; j < curObjCount; ++j)
                        {
                            t.GameObjects[(int)j].Dispose();
                        }

                        t.GameObjects.RemoveRange((int)tarObjCount, (int)diff);
                    }

                    for (uint j = 0; j < tarObjCount; ++j)
                    {
                        t.GameObjects[(int)j].Transform.SetMatrix(dat.Transforms[(int)j]);
                    }

                    newTables.Add(t);
                    s_meshObjects.RemoveAt((int)i);

                    goto NextMesh;
                }

                uint transformCount = (uint)dat.Transforms.Count;
                if (transformCount <= 0)
                {
                    continue;
                }

                mat = AssetStore.GetMaterial(dat.MaterialID);
                if (mat == null)
                {
                    continue;
                }

                if (dat.MeshID != uint.MaxValue)
                {
                    mesh = AssetStore.GetMesh(dat.MeshID);
                    if (mesh == null)
                    {
                        continue;
                    }
                }

                GameObject[] objs = GameObject.BatchInstantiate(transformCount);
                for (uint i = 0; i < transformCount; ++i)
                {
                    GameObject obj = objs[(int)i];

                    MeshRenderer renderer = obj.AddComponent<MeshRenderer>();
                    renderer.Mesh = mesh;
                    renderer.IndexCount = dat.IndexCount;
                    renderer.Material = mat;

                    obj.Transform.SetMatrix(dat.Transforms[(int)i]);
                }

                MeshTable table = new MeshTable()
                {
                    MaterialID = dat.MaterialID,
                    MeshID = dat.MeshID,
                    IndexCount = dat.IndexCount,
                    GameObjects = new List<GameObject>(objs),
                };

                newTables.Add(table);
NextMesh:;
            }

            foreach (MeshTable table in s_meshObjects)
            {
                foreach (GameObject obj in table.GameObjects)
                {
                    obj.Dispose();
                }
            }

            s_meshObjects = newTables;

            s_meshLock.WriteUnlock();
        }

        static List<DrawModelData> GetModelData(byte[] a_data)
        {
            if (a_data.Length < Marshal.SizeOf<DrawDataHeader>())
            {
                return null;
            }

            uint version = BitConverter.ToUInt32(a_data, 0);
            if (version != 0)
            {
                return null;
            }

            DrawDataHeader dataHeader = new DrawDataHeader()
            {
                Version = version,
                HeaderOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<DrawDataHeader>("HeaderOffset")),
                HeaderCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<DrawDataHeader>("HeaderCount")),
            };

            uint bufferHeaderSize = (uint)Marshal.SizeOf<DrawModelBufferHeader>();
            uint headerDataSize = dataHeader.HeaderCount * bufferHeaderSize;
            if (a_data.Length < dataHeader.HeaderOffset + headerDataSize)
            {
                return null;
            }

            List<DrawModelData> modelData = new List<DrawModelData>();
            for (uint i = 0; i < dataHeader.HeaderCount; ++i)
            {
                uint headerOffset = dataHeader.HeaderOffset + (i * bufferHeaderSize);

                DrawModelBufferHeader bufferHeader = new DrawModelBufferHeader()
                {
                    MaterialID = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawModelBufferHeader>("MaterialID"))),
                    ModelID = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawModelBufferHeader>("ModelID"))),
                    TransformOffset = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawModelBufferHeader>("TransformOffset"))),
                    TransformCount = BitConverter.ToUInt32(a_data, (int)(headerOffset + (uint)Marshal.OffsetOf<DrawModelBufferHeader>("TransformCount"))),
                };

                if (bufferHeader.TransformCount <= 0)
                {
                    continue;
                }

                uint mat4Size = (uint)Marshal.SizeOf<Matrix4>();
                uint transformSize = bufferHeader.TransformCount * mat4Size;
                if (a_data.Length < bufferHeader.TransformOffset + transformSize)
                {
                    return null;
                }

                DrawModelData data = new DrawModelData()
                {
                    MaterialID = bufferHeader.MaterialID,
                    ModelID = bufferHeader.ModelID,
                    Transforms = new List<Matrix4>((int)bufferHeader.TransformCount)
                };

                for (uint j = 0; j < bufferHeader.TransformCount; ++j)
                {
                    uint transformOffset = bufferHeader.TransformOffset + (j * mat4Size);

                    float[] matData = new float[16];

                    for (uint k = 0; k < 16; ++k)
                    {
                        uint indexOffset = transformOffset + (k * sizeof(float));

                        matData[k] = BitConverter.ToSingle(a_data, (int)indexOffset);
                    }

                    data.Transforms.Add(new Matrix4(matData));
                }

                modelData.Add(data);
            }

            return modelData;
        }

        static void DrawModel(string a_string, byte[] a_data)
        {
            if (a_string != "Editor:SceneView:DrawModel")
            {
                return;
            }

            List<DrawModelData> modelData = GetModelData(a_data);
            if (modelData == null)
            {
                return;
            }

            s_modelLock.WriteLock();

            List<ModelTable> newTables = new List<ModelTable>();

            foreach (DrawModelData dat in modelData)
            {
                Material mat = null;
                Model model = null;

                uint tableCount = (uint)s_modelObjects.Count;
                for (uint i = 0; i < tableCount; ++i)
                {
                    ModelTable t = s_modelObjects[(int)i];

                    if (t.MaterialID != dat.MaterialID)
                    {
                        continue;
                    }

                    if (t.ModelID != dat.ModelID)
                    {
                        continue;
                    }

                    uint tarObjCount = (uint)dat.Transforms.Count;
                    uint curObjCount = (uint)t.GameObjects.Count;

                    if (tarObjCount > curObjCount)
                    {
                        uint diff = tarObjCount - curObjCount;

                        mat = AssetStore.GetMaterial(dat.MaterialID);
                        if (mat == null)
                        {
                            goto NextModel;
                        }

                        model = AssetStore.GetModel(dat.ModelID);
                        if (model == null)
                        {
                            goto NextModel;
                        }

                        GameObject[] objects = GameObject.BatchInstantiate(diff);
                        foreach (GameObject obj in objects)
                        {
                            ModelRenderer renderer = obj.AddComponent<ModelRenderer>();
                            renderer.Model = model;
                            renderer.Material = mat;
                        }

                        t.GameObjects.AddRange(objects);
                    }
                    else if (tarObjCount < curObjCount)
                    {
                        uint diff = curObjCount - tarObjCount;

                        for (uint j = tarObjCount; j < curObjCount; ++j)
                        {
                            t.GameObjects[(int)j].Dispose();
                        }

                        t.GameObjects.RemoveRange((int)tarObjCount, (int)diff);
                    }

                    for (uint j = 0; j < tarObjCount; ++j)
                    {
                        t.GameObjects[(int)j].Transform.SetMatrix(dat.Transforms[(int)j]);
                    }

                    newTables.Add(t);
                    s_modelObjects.RemoveAt((int)i);

                    goto NextModel;
                }

                uint transformCount = (uint)dat.Transforms.Count;
                if (transformCount <= 0)
                {
                    continue;
                }

                mat = AssetStore.GetMaterial(dat.MaterialID);
                if (mat == null)
                {
                    continue;
                }

                model = AssetStore.GetModel(dat.ModelID);
                if (model == null)
                {
                    continue;
                }

                GameObject[] objs = GameObject.BatchInstantiate(transformCount);
                for (uint i = 0; i < transformCount; ++i)
                {
                    GameObject obj = objs[(int)i];

                    ModelRenderer renderer = obj.AddComponent<ModelRenderer>();
                    renderer.Model = model;
                    renderer.Material = mat;

                    obj.Transform.SetMatrix(dat.Transforms[(int)i]);
                }

                ModelTable table = new ModelTable()
                {
                    MaterialID = dat.MaterialID,
                    ModelID = dat.ModelID,
                    GameObjects = new List<GameObject>(objs),
                };

                newTables.Add(table);
NextModel:;
            }

            foreach (ModelTable table in s_modelObjects)
            {
                foreach (GameObject obj in table.GameObjects)
                {
                    obj.Dispose();
                }
            }

            s_modelObjects = newTables;

            s_modelLock.WriteUnlock();
        }

        public static void Update()
        {
            Quaternion camRotation = s_camObject.Transform.Rotation;
            Quaternion verOffset = Quaternion.FromAxisAngle(Vector3.UnitX, 0.4f);
            Quaternion horOffset = Quaternion.FromAxisAngle(Vector3.UnitY, 0.4f);

            s_viewportLight.Transform.Rotation = verOffset * horOffset * camRotation;

            // Things are async so another update may come through after we update the state leading to wonky visuals
            // Doing the lazy route and forcing it off every update if we are not in the scene state
            if (s_lightMode != EditorLightMode.Scene)
            {
                s_ambientLightLock.WriteLock();

                foreach (AmbientLight aLight in s_ambientLights)
                {
                    aLight.RenderLayer = 0;
                    aLight.Intensity = 0.0f;
                }

                s_ambientLightLock.WriteUnlock();

                s_directionalLightLock.WriteLock();

                foreach (DirectionalLight dLight in s_directionalLights)
                {
                    dLight.RenderLayer = 0;
                    dLight.Intensity = 0.0f;
                }

                s_directionalLightLock.WriteUnlock();

                s_pointLightLock.WriteLock();

                foreach (PointLight pLight in s_pointLights)
                {
                    pLight.RenderLayer = 0;
                    pLight.Intensity = 0.0f;
                }

                s_pointLightLock.WriteUnlock();

                s_spotLightLock.WriteLock();

                foreach (SpotLight sLight in s_spotLights)
                {
                    sLight.RenderLayer = 0;
                    sLight.Intensity = 0.0f;
                }

                s_spotLightLock.WriteUnlock();
            }
        }

        public static void Init()
        {
            s_lightMode = EditorLightMode.Ambient;

            s_ambientLights = new List<AmbientLight>();
            s_directionalLights = new List<DirectionalLight>();
            s_pointLights = new List<PointLight>();
            s_spotLights = new List<SpotLight>();

            s_meshObjects = new List<MeshTable>();
            s_modelObjects = new List<ModelTable>();

            s_ambientLightLock = new NativeLock();
            s_directionalLightLock = new NativeLock();
            s_pointLightLock = new NativeLock();
            s_spotLightLock = new NativeLock();

            s_meshLock = new NativeLock();
            s_modelLock = new NativeLock();

            GameObject staticLightObject = GameObject.Instantiate();
            s_staticLight = staticLightObject.AddComponent<AmbientLight>();

            GameObject viewportLightObject = GameObject.Instantiate();
            s_viewportLight = viewportLightObject.AddComponent<DirectionalLight>();
            s_viewportLight.Intensity = 0.0f;
            s_viewportLight.RenderLayer = 0;

            s_camObject = GameObject.Instantiate();
            s_camObject.AddComponent<CameraController>();

            PipeMessage.AddCallback("Editor:SceneView:LightMode", LightMode);

            PipeMessage.AddCallback("Editor:SceneView:PushAmbientLight", PushAmbientLight);
            PipeMessage.AddCallback("Editor:SceneView:PushDirectionalLight", PushDirectionalLight);
            PipeMessage.AddCallback("Editor:SceneView:PushPointLight", PushPointLight);
            PipeMessage.AddCallback("Editor:SceneView:PushSpotLight", PushSpotLight);

            PipeMessage.AddCallback("Editor:SceneView:DrawMesh", DrawMesh);
            PipeMessage.AddCallback("Editor:SceneView:DrawModel", DrawModel);
        }
        public static void Destroy()
        {
            s_ambientLightLock.WriteLock();

            foreach (AmbientLight aLight in s_ambientLights)
            {
                aLight.GameObject.Dispose();
            }

            s_ambientLightLock.WriteUnlock();

            s_directionalLightLock.WriteLock();

            foreach (DirectionalLight dLight in s_directionalLights)
            {
                dLight.GameObject.Dispose();
            }

            s_directionalLights.Clear();

            s_directionalLightLock.WriteUnlock();

            s_pointLightLock.WriteLock();

            foreach (PointLight pLight in s_pointLights)
            {
                pLight.GameObject.Dispose();
            }

            s_pointLights.Clear();

            s_pointLightLock.WriteUnlock();

            s_spotLightLock.WriteLock();

            foreach (SpotLight sLight in s_spotLights)
            {
                sLight.GameObject.Dispose();
            }

            s_spotLights.Clear();

            s_spotLightLock.WriteUnlock();

            s_meshLock.WriteLock();

            foreach (MeshTable mshT in s_meshObjects)
            {
                foreach (GameObject obj in mshT.GameObjects)
                {
                    obj.Dispose();
                }
            }

            s_meshObjects.Clear();

            s_meshLock.WriteUnlock();

            s_modelLock.WriteLock();

            foreach (ModelTable mdlT in s_modelObjects)
            {
                foreach (GameObject obj in mdlT.GameObjects)
                {
                    obj.Dispose();
                }
            }

            s_modelObjects.Clear();

            s_modelLock.WriteUnlock();

            s_ambientLightLock.Dispose();
            s_directionalLightLock.Dispose();
            s_pointLightLock.Dispose();
            s_spotLightLock.Dispose();

            s_meshLock.Dispose();
            s_modelLock.Dispose();

            s_staticLight.GameObject.Dispose();
            s_viewportLight.GameObject.Dispose();

            s_camObject.Dispose();
        }
    }
}

// MIT License
// 
// Copyright (c) 2026 River Govers
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
