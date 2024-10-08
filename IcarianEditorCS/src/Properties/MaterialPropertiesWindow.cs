// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(MaterialDef))]
    public class MaterialPropertiesWindow : PropertiesEditorWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            MaterialDef def = a_object as MaterialDef;
            if (def == null)
            {
                return;
            }

            GUI.PathStringField("Vertex Shader Path", ref def.VertexShaderPath, Extensions.VertexShaderExtensions);
            GUI.Tooltip("Vertex Shader Path", "Path relative to the project for the vertex shader file to be used.");
            GUI.PathStringField("Pixel Shader Path", ref def.PixelShaderPath, Extensions.PixelShaderExtensions);
            GUI.Tooltip("Pixel Shader Path", "Path relative to the project for the pixel shader file to be used.");

            GUI.EnumField("Culling Mode", ref def.CullingMode);
            GUI.Tooltip("Culling Mode", "Which faces to show when rendering.");

            GUI.EnumField("Primitive Mode", ref def.PrimitiveMode);
            GUI.Tooltip("Primitive Mode", "Which primitive to use when rendering.");

            GUI.EnumField("Color Blend Mode", ref def.ColorBlendMode);
            GUI.Tooltip("Color Blend Mode", "The blending mode of the material");

            GUI.BitField("Render Layer", ref def.RenderLayer, 32);
            GUI.Tooltip("Render Layer", "Used to determine if it will be rendered by a camera in a matching layer. Binary bit based.");

            bool addValue;
            bool show = GUI.ArrayView("Texture Inputs", out addValue);
            GUI.Tooltip("Texture Input", "Used to determine input values for shaders.");

            if (addValue)
            {
                if (def.TextureInputs == null)
                {
                    def.TextureInputs = new List<TextureInput>();
                }

                def.TextureInputs.Add(new TextureInput());
            }

            if (show && def.TextureInputs != null)
            {
                GUI.Indent();
                GUI.PushID("TextureInputs");

                for (int i = 0; i < def.TextureInputs.Count; ++i)
                {
                    TextureInput input = def.TextureInputs[i];
                    GUI.PushID($"[{i}]");

                    if (GUI.Button("-"))
                    {
                        def.TextureInputs.RemoveAt(i);
                        --i;

                        GUI.PopID();

                        continue;
                    }

                    GUI.SameLine();

                    if (GUI.StructView($"Texture Input[{i}]"))
                    {   
                        GUI.Indent();

                        GUI.RUIntField("Slot", ref input.Slot, uint.MaxValue);
                        GUI.PathStringField("Path", ref input.Path, Extensions.TextureExtensions);
                        GUI.EnumField("Address Mode", ref input.AddressMode);
                        GUI.EnumField("Filter Mode", ref input.FilterMode);

                        def.TextureInputs[i] = input;

                        GUI.Unindent();
                    }

                    GUI.PopID();
                }

                GUI.PopID();
                GUI.Unindent();
            }

            show = GUI.ArrayView("Vertex Attributes", out addValue);

            if (addValue)
            {
                if (def.VertexAttributes == null)
                {
                    def.VertexAttributes = new List<VertexInputAttribute>();
                }

                def.VertexAttributes.Add(new VertexInputAttribute());
            }

            if (show && def.VertexAttributes != null)
            {
                GUI.Indent();
                GUI.PushID("VertexAttributes");

                for (int i = 0; i < def.VertexAttributes.Count; ++i)
                {
                    VertexInputAttribute input = def.VertexAttributes[i];
                    GUI.PushID($"[{i}]");

                    if (GUI.Button("-"))
                    {
                        def.VertexAttributes.RemoveAt(i);
                        --i;

                        GUI.PopID();

                        continue;
                    }

                    GUI.SameLine();

                    if (GUI.StructView($"Vertex Attribute[{i}]"))
                    {
                        GUI.Indent();
    
                        uint location = input.Location;
                        if (GUI.RUIntField("Location", ref location, uint.MaxValue))
                        {
                            input.Location = (ushort)location;
                        }

                        GUI.EnumField("Type", ref input.Type);

                        uint count = input.Count;
                        if (GUI.UIntField("Count", ref count))
                        {
                            input.Count = (ushort)count;
                        }

                        uint offset = input.Offset;
                        if (GUI.UIntField("Offset", ref offset))
                        {
                            input.Offset = (ushort)offset;
                        }

                        def.VertexAttributes[i] = input;

                        GUI.Unindent();
                    }

                    GUI.PopID();
                }

                GUI.PopID();
                GUI.Unindent();
            }

            GUI.PathStringField("Shadow Vertex Shader Path", ref def.ShadowVertexShaderPath, Extensions.VertexShaderExtensions);
            GUI.Tooltip("Shadow Vertex Shader Path", "Path relative to the project for the vertex shader file to be used for shadows.");

            Type uboType = def.UniformBufferType;
            if (uboType != null)
            {
                object obj = Activator.CreateInstance(def.UniformBufferType);
                object baseVal = Activator.CreateInstance(def.UniformBufferType);

                if (def.UniformBufferFields != null)
                {
                    for (int i = 0; i < def.UniformBufferFields.Count; ++i)
                    {
                        UBOField field = def.UniformBufferFields[i];

                        FieldInfo fieldInfo = uboType.GetField(field.Name);
                        if (fieldInfo == null)
                        {
                            continue;
                        }

                        Type type = fieldInfo.FieldType;

                        fieldInfo.SetValue(obj, MaterialDef.UBOValueToObject(type, field.Value));
                    }
                }

                if (GUI.StructView("Uniform Buffer"))
                {
                    List<UBOField> fields = new List<UBOField>();

                    FieldInfo[] fieldInfos = uboType.GetFields(BindingFlags.Public | BindingFlags.Instance);
                    foreach (FieldInfo f in fieldInfos)
                    {
                        string name = f.Name;
                        object value = f.GetValue(obj);
                        object defValue = f.GetValue(baseVal);

                        Type type = f.FieldType;

                        if (def.UniformBufferFields != null)
                        {
                            foreach (UBOField field in def.UniformBufferFields)
                            {
                                if (field.Name == name)
                                {
                                    value = MaterialDef.UBOValueToObject(type, field.Value);

                                    break;
                                }
                            }
                        }
                        
                        if (value == null)
                        {
                            continue;
                        }

                        if (type == typeof(float))
                        {
                            float val = (float)value;
                            float d = (float)defValue;

                            GUI.RFloatField(name, ref val, d);

                            value = val;
                        }
                        else if (type == typeof(Vector2))
                        {
                            Vector2 val = (Vector2)value;
                            Vector2 d = (Vector2)defValue;

                            GUI.RVec2Field(name, ref val, d);

                            value = val;
                        }
                        else if (type == typeof(Vector3))
                        {
                            Vector3 val = (Vector3)value;
                            Vector3 d = (Vector3)defValue;

                            GUI.RVec3Field(name, ref val, d);

                            value = val;
                        }
                        else if (type == typeof(Vector4))
                        {
                            Vector4 val = (Vector4)value;
                            Vector4 d = (Vector4)defValue;

                            GUI.RVec4Field(name, ref val, d);

                            value = val;
                        }

                        if (object.Equals(value, defValue))
                        {
                            continue;
                        }

                        UBOField uF = new UBOField()
                        {
                            Name = name,
                            Value = MaterialDef.UBOFieldValueToString(value)
                        };

                        fields.Add(uF);
                    }

                    def.UniformBufferFields = fields;
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