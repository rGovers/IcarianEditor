// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Shaders;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

#include "EditorRenderProgramHeaderInteropStructures.h"

namespace IcarianEditor.Engine
{
    public static class AssetStore
    {
        struct RenderProgramData
        {
            public RenderProgramHeader Header;
            public byte[] Data;
        }

        static Dictionary<uint, RenderProgramData> s_renderProgramHeaders;
        static Dictionary<uint, Material>          s_materials;

        static Dictionary<uint, MeshShader>        s_meshShaders;
        static Dictionary<uint, VertexShader>      s_vertexShaders;
        static Dictionary<uint, PixelShader>       s_pixelShaders;

        static Dictionary<uint, Mesh>              s_meshes;
        static Dictionary<uint, Model>             s_models;

        static NativeLock                          s_materialLock;

        static NativeLock                          s_meshShaderLock;
        static NativeLock                          s_vertexShaderLock;
        static NativeLock                          s_pixelShaderLock;

        static NativeLock                          s_meshLock;
        static NativeLock                          s_modelLock;

        static Material GenerateMaterial(RenderProgramData a_data)
        {
            RenderProgramHeader header = a_data.Header;

            MaterialBuilder builder = new MaterialBuilder();

            if (header.IsMesh != 0)
            {
                MeshShader meshShader = GetMeshShader(header.VertexShader);
                if (meshShader == null)
                {
                    return null;
                }

                builder.MeshShader = meshShader;
            }
            else
            {
                VertexShader vertexShader = GetVertexSahder(header.VertexShader);
                if (vertexShader == null)
                {
                    return null;
                }

                builder.VertexShader = vertexShader;

                if (header.VertexAttributeCount > 0)
                {
                    VertexInputAttribute[] inputs = new VertexInputAttribute[header.VertexAttributeCount];

                    for (uint i = 0; i < header.VertexAttributeCount; ++i)
                    {
                        uint offset = header.VertexAttributeOffset + (i * (uint)Marshal.SizeOf<VertexInputAttribute>());

                        VertexInputAttribute inp = new VertexInputAttribute()
                        {
                            Location = BitConverter.ToUInt16(a_data.Data, (int)(offset + (uint)Marshal.OffsetOf<VertexInputAttribute>("Location"))),
                            Type = (VertexType)BitConverter.ToUInt16(a_data.Data, (int)(offset + (uint)Marshal.OffsetOf<VertexInputAttribute>("Type"))),
                            Count = BitConverter.ToUInt16(a_data.Data, (int)(offset + (uint)Marshal.OffsetOf<VertexInputAttribute>("Count"))),
                            Offset = BitConverter.ToUInt16(a_data.Data, (int)(offset + (uint)Marshal.OffsetOf<VertexInputAttribute>("Offset"))),
                        };

                        inputs[i] = inp;
                    }

                    builder.Attributes = inputs;
                }
            }

            PixelShader pixelShader = GetPixelShader(header.PixelShader);
            if (pixelShader == null)
            {
                return null;
            }

            builder.PixelShader = pixelShader;

            builder.VertexStride = (ushort)header.VertexStride;
            builder.CullingMode = (CullMode)header.CullMode;
            builder.ColorBlendMode = (MaterialBlendMode)header.ColorBlendMode;
            builder.RenderLayer = header.RenderLayer;

            if (header.UBOSize > 0)
            {
                builder.UBOData = new byte[header.UBOSize];
                Array.Copy(a_data.Data, (long)header.UBOOffset, builder.UBOData, 0, (long)header.UBOSize);
            }

            if (header.UserArrayCount > 0 && header.UserArrayStride > 0)
            {
                ulong size = (ulong)header.UserArrayCount * header.UserArrayStride;

                builder.UserArrayData = new byte[size];
                Array.Copy(a_data.Data, (long)header.UserArrayOffset, builder.UserArrayData, 0, (long)size);
            }

            return Material.CreateMaterial(builder);
        }

        static void GenerateRenderProgram(string a_type, byte[] a_data)
        {
            s_materialLock.WriteLock();

            if (a_type != "Editor:AssetStore:GenerateRenderProgram")
            {
                s_materialLock.WriteUnlock();

                return;
            }

            // It is not the worst if this does not load on the current frame and loads on the next frame
            // Cannot ensure that the shaders will be loaded in time due to being async so defer the loading of the Material to a later time
            RenderProgramHeader header = new RenderProgramHeader()
            {
                ID = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("ID")),
                IsMesh = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("IsMesh")),
                VertexShader = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("VertexShader")),
                PixelShader = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("PixelShader")),
                VertexStride = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("VertexStride")),
                VertexAttributeCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("VertexAttributeCount")),
                VertexAttributeOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("VertexAttributeOffset")),
                CullMode = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("CullMode")),
                ColorBlendMode = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("ColorBlendMode")),
                RenderLayer = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("RenderLayer")),
                UBOSize = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("UBOSize")),
                UBOOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("UBOOffset")),
                UserArrayCount = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("UserArrayCount")),
                UserArrayStride = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("UserArrayStride")),
                UserArrayOffset = BitConverter.ToUInt32(a_data, (int)Marshal.OffsetOf<RenderProgramHeader>("UserArrayOffset")),
            };

            RenderProgramData dat = new RenderProgramData()
            {
                Header = header,
                Data = a_data
            };

            s_renderProgramHeaders.Add(header.ID, dat);

            s_materialLock.WriteUnlock();
        }

        static void GenerateMeshShaderFromFile(string a_type, byte[] a_data)
        {
            s_meshShaderLock.WriteLock();

            if (a_type != "Editor:AssetStore:GenerateMeshShaderFromFile")
            {
                s_meshShaderLock.WriteUnlock();

                return;
            }

            uint id = BitConverter.ToUInt32(a_data, 0);
            if (id == uint.MaxValue)
            {
                Logger.Error("GenerateMeshShaderFromFile invalid id");
                s_meshShaderLock.WriteUnlock();

                return;
            }

            string path = Encoding.UTF8.GetString(a_data, sizeof(uint), a_data.Length - sizeof(uint));
            if (string.IsNullOrWhiteSpace(path))
            {
                Logger.Error("GenerateMeshShaderFromFile null path");
                s_meshShaderLock.WriteUnlock();

                return;
            }

            // We do not have access to the assets so need to request the assets from the editor so use the pipe
            // I should probably do this better rather than just appending pipe://
            string pipePath = "pipe://" + path;

            MeshShader shader = MeshShader.LoadMeshShader(pipePath);
            if (shader == null)
            {
                Logger.Error("GenerateMeshShaderFromFile failed to load MeshShader");
                s_meshShaderLock.WriteUnlock();

                return;
            }

            s_meshShaders.Add(id, shader);

            s_meshShaderLock.WriteUnlock();
        }
        static void GenerateVertexShaderFromFile(string a_type, byte[] a_data)
        {
            s_vertexShaderLock.WriteLock();

            if (a_type != "Editor:AssetStore:GenerateVertexShaderFromFile")
            {
                s_vertexShaderLock.WriteUnlock();

                return;
            }

            uint id = BitConverter.ToUInt32(a_data, 0);
            if (id == uint.MaxValue)
            {
                Logger.Error("GenerateVertexShaderFromFile invalid id");
                s_vertexShaderLock.WriteUnlock();

                return;
            }

            string path = Encoding.UTF8.GetString(a_data, sizeof(uint), a_data.Length - sizeof(uint));
            if (string.IsNullOrWhiteSpace(path))
            {
                Logger.Error("GenerateVertexShaderFromFile null path");
                s_vertexShaderLock.WriteUnlock();

                return;
            }

            string pipePath = "pipe://" + path;

            VertexShader shader = VertexShader.LoadVertexShader(pipePath);
            if (shader == null)
            {
                Logger.Error("GenerateVertexShaderFromFile failed to load VertexShader");
                s_vertexShaderLock.WriteUnlock();

                return;
            }

            s_vertexShaders.Add(id, shader);

            s_vertexShaderLock.WriteUnlock();
        }
        static void GeneratePixelShaderFromFile(string a_type, byte[] a_data)
        {
            s_pixelShaderLock.WriteLock();

            if (a_type != "Editor:AssetStore:GeneratePixelShaderFromFile")
            {
                s_pixelShaderLock.WriteUnlock();

                return;
            }

            uint id = BitConverter.ToUInt32(a_data, 0);
            if (id == uint.MaxValue)
            {
                Logger.Error("GeneratePixelShaderFromFile invalid id");
                s_pixelShaderLock.WriteUnlock();

                return;
            }

            string path = Encoding.UTF8.GetString(a_data, sizeof(uint), a_data.Length - sizeof(uint));
            if (string.IsNullOrWhiteSpace(path))
            {
                Logger.Error("GeneratePixelShaderFromFile null path");
                s_pixelShaderLock.WriteUnlock();

                return;
            }

            string pipePath = "pipe://" + path;

            PixelShader shader = PixelShader.LoadPixelShader(pipePath);
            if (shader == null)
            {
                Logger.Error("GeneratePixelShaderFromFile failed to load PixelShader");
                s_pixelShaderLock.WriteUnlock();

                return;
            }

            s_pixelShaders.Add(id, shader);

            s_pixelShaderLock.WriteUnlock();
        }

        static void GenerateMeshFromFile(string a_type, byte[] a_data)
        {
            if (a_type != "Editor:AssetStore:GenerateMeshFromFile")
            {
                return;
            }

            s_meshLock.WriteLock();

            uint id = BitConverter.ToUInt32(a_data, 0);
            if (id == uint.MaxValue)
            {
                Logger.Error("GenerateMeshFromFile invalid id");
                s_meshLock.WriteUnlock();

                return;
            }

            string path = Encoding.UTF8.GetString(a_data, sizeof(uint), a_data.Length - sizeof(uint));
            if (string.IsNullOrWhiteSpace(path))
            {
                Logger.Error("GenerateMeshFromFile null path");
                s_meshLock.WriteUnlock();

                return;
            }

            string pipePath = "pipe://" + path;

            Mesh mesh = Mesh.LoadMesh(pipePath);
            if (mesh == null)
            {
                Logger.Error("GenerateMeshFromFile failed to load Mesh");
                s_meshLock.WriteUnlock();

                return;
            }

            s_meshes.Add(id, mesh);

            s_meshLock.WriteUnlock();
        }
        static void GenerateModelFromFile(string a_type, byte[] a_data)
        {
            if (a_type != "Editor:AssetStore:GenerateModelFromFile")
            {
                return;
            }

            s_modelLock.WriteLock();

            uint id = BitConverter.ToUInt32(a_data, 0);
            if (id == uint.MaxValue)
            {
                Logger.Error("GenerateModelFromFile invalid id");
                s_modelLock.WriteUnlock();

                return;
            }

            string path = Encoding.UTF8.GetString(a_data, sizeof(uint), a_data.Length - sizeof(uint));
            if (string.IsNullOrWhiteSpace(path))
            {
                Logger.Error("GenerateModelFromFile null path");
                s_modelLock.WriteUnlock();

                return;
            }

            string pipePath = "pipe://" + path;

            Model model = Model.LoadModel(pipePath);
            if (model == null)
            {
                Logger.Error("GenerateModelFromFile failed to load Model");
                s_modelLock.WriteUnlock();

                return;
            }

            s_models.Add(id, model);

            s_modelLock.WriteUnlock();
        }

        public static void Init()
        {
            s_renderProgramHeaders = new Dictionary<uint, RenderProgramData>();
            s_materials = new Dictionary<uint, Material>();

            s_meshShaders = new Dictionary<uint, MeshShader>();
            s_vertexShaders = new Dictionary<uint, VertexShader>();
            s_pixelShaders = new Dictionary<uint, PixelShader>();

            s_meshes = new Dictionary<uint, Mesh>();
            s_models = new Dictionary<uint, Model>();

            s_materialLock = new NativeLock();

            s_meshShaderLock = new NativeLock();
            s_vertexShaderLock = new NativeLock();
            s_pixelShaderLock = new NativeLock();

            s_meshLock = new NativeLock();
            s_modelLock = new NativeLock();

            PipeMessage.AddCallback("Editor:AssetStore:GenerateRenderProgram", GenerateRenderProgram);

            PipeMessage.AddCallback("Editor:AssetStore:GenerateMeshShaderFromFile", GenerateMeshShaderFromFile);
            PipeMessage.AddCallback("Editor:AssetStore:GenerateVertexShaderFromFile", GenerateVertexShaderFromFile);
            PipeMessage.AddCallback("Editor:AssetStore:GeneratePixelShaderFromFile", GeneratePixelShaderFromFile);

            PipeMessage.AddCallback("Editor:AssetStore:GenerateMeshFromFile", GenerateMeshFromFile);
            PipeMessage.AddCallback("Editor:AssetStore:GenerateModelFromFile", GenerateModelFromFile);
        }
        public static void Destroy()
        {
            s_materialLock.WriteLock();

            foreach (Material material in s_materials.Values)
            {
                if (material != null && !material.IsDisposed)
                {
                    material.Dispose();
                }
            }

            s_materialLock.WriteUnlock();

            s_meshShaderLock.WriteLock();

            foreach (MeshShader meshShader in s_meshShaders.Values)
            {
                if (meshShader != null && !meshShader.IsDisposed)
                {
                    meshShader.Dispose();
                }
            }
            s_meshShaders = null;

            s_meshShaderLock.WriteUnlock();

            s_vertexShaderLock.WriteLock();

            foreach (VertexShader vertexShader in s_vertexShaders.Values)
            {
                if (vertexShader != null && !vertexShader.IsDisposed)
                {
                    vertexShader.Dispose();
                }
            }
            s_vertexShaders = null;

            s_vertexShaderLock.WriteUnlock();

            s_pixelShaderLock.WriteLock();

            foreach (PixelShader pixelShader in s_pixelShaders.Values)
            {
                if (pixelShader != null && !pixelShader.IsDisposed)
                {
                    pixelShader.Dispose();
                }
            }
            s_pixelShaders = null;

            s_pixelShaderLock.WriteUnlock();

            s_meshLock.WriteLock();

            foreach (Mesh mesh in s_meshes.Values)
            {
                if (mesh != null && !mesh.IsDisposed)
                {
                    mesh.Dispose();
                }
            }
            s_meshes = null;

            s_meshLock.WriteUnlock();

            s_modelLock.WriteLock();

            foreach (Model model in s_models.Values)
            {
                if (model != null && !model.IsDisposed)
                {
                    model.Dispose();
                }
            }
            s_models = null;

            s_modelLock.WriteUnlock();

            s_materialLock.Dispose();
            s_materialLock = null;

            s_meshShaderLock.Dispose();
            s_meshShaderLock = null;
            s_vertexShaderLock.Dispose();
            s_vertexShaderLock = null;
            s_pixelShaderLock.Dispose();
            s_pixelShaderLock = null;

            s_meshLock.Dispose();
            s_meshLock = null;
            s_modelLock.Dispose();
            s_modelLock = null;
        }

        public static Material GetMaterial(uint a_id)
        {
            Material mat = null;

            s_materialLock.ReadLock();

            if (s_materials.ContainsKey(a_id))
            {
                mat = s_materials[a_id];

                s_materialLock.ReadUnlock();

                return mat;
            }

            if (!s_renderProgramHeaders.ContainsKey(a_id))
            {
                s_materialLock.ReadUnlock();

                return null;
            }

            s_materialLock.ReadUnlock();

            s_materialLock.WriteLock();

            if (s_materials.ContainsKey(a_id))
            {
                mat = s_materials[a_id];

                s_materialLock.WriteUnlock();

                return mat;
            }

            RenderProgramData dat = s_renderProgramHeaders[a_id];

            mat = GenerateMaterial(dat);
            if (mat == null)
            {
                s_materialLock.WriteUnlock();

                return null;
            }

            s_renderProgramHeaders.Remove(a_id);
            s_materials.Add(a_id, mat);

            s_materialLock.WriteUnlock();

            return mat;
        }

        public static MeshShader GetMeshShader(uint a_id)
        {
            s_meshShaderLock.ReadLock();

            if (s_meshShaders.ContainsKey(a_id))
            {
                MeshShader shader = s_meshShaders[a_id];

                s_meshShaderLock.ReadUnlock();

                return shader;
            }

            s_meshShaderLock.ReadUnlock();

            return null;
        }
        public static VertexShader GetVertexSahder(uint a_id)
        {
            s_vertexShaderLock.ReadLock();

            if (s_vertexShaders.ContainsKey(a_id))
            {
                VertexShader shader = s_vertexShaders[a_id];

                s_vertexShaderLock.ReadUnlock();

                return shader;
            }

            s_vertexShaderLock.ReadUnlock();

            return null;
        }
        public static PixelShader GetPixelShader(uint a_id)
        {
            s_pixelShaderLock.ReadLock();

            if (s_pixelShaders.ContainsKey(a_id))
            {
                PixelShader shader = s_pixelShaders[a_id];

                s_pixelShaderLock.ReadUnlock();

                return shader;
            }

            s_pixelShaderLock.ReadUnlock();

            return null;
        }

        public static Mesh GetMesh(uint a_id)
        {
            s_meshLock.ReadLock();

            if (s_meshes.ContainsKey(a_id))
            {
                Mesh mesh = s_meshes[a_id];

                s_meshLock.ReadUnlock();

                return mesh;
            }

            s_meshLock.ReadUnlock();

            return null;
        }
        public static Model GetModel(uint a_id)
        {
            s_modelLock.ReadLock();

            if (s_models.ContainsKey(a_id))
            {
                Model model = s_models[a_id];

                s_modelLock.ReadUnlock();

                return model;
            }

            s_modelLock.ReadUnlock();

            return null;
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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
