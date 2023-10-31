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
        public override void OnGUI(object a_object)
        {
            MaterialDef def = a_object as MaterialDef;
            if (def == null)
            {
                return;
            }

            GUI.StringField("Vertex Shader Path", ref def.VertexShaderPath);
            GUI.Tooltip("Vertex Shader Path", "Path relative to the project for the vertex shader file to be used.");
            GUI.StringField("Pixel Shader Path", ref def.PixelShaderPath);
            GUI.Tooltip("Pixel Shader Path", "Path relative to the project for the pixel shader file to be used.");

            GUI.EnumField("Culling Mode", ref def.CullingMode);
            GUI.Tooltip("Culling Mode", "Which faces to show when rendering.");

            GUI.EnumField("Primitive Mode", ref def.PrimitiveMode);
            GUI.Tooltip("Primitive Mode", "Which primitive to use when rendering.");

            GUI.Checkbox("Enable Color Blending", ref def.EnableColorBlending);
            GUI.Tooltip("Enable Color Blending", "Enables color blending.");

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
                        GUI.StringField("Path", ref input.Path);
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

            show = GUI.ArrayView("Shader Buffers", out addValue);
            GUI.Tooltip("Shader Buffer", "Used to determine input values for shaders.");

            if (addValue)
            {
                if (def.ShaderBuffers == null)
                {
                    def.ShaderBuffers = new List<ShaderBufferInput>();
                }

                def.ShaderBuffers.Add(new ShaderBufferInput());
            }

            if (show && def.ShaderBuffers != null)
            {
                GUI.Indent();
                GUI.PushID("ShaderBuffers");

                for (int i = 0; i < def.ShaderBuffers.Count; ++i)
                {
                    ShaderBufferInput input = def.ShaderBuffers[i];
                    GUI.PushID($"[{i}]");

                    if (GUI.Button("-"))
                    {
                        def.ShaderBuffers.RemoveAt(i);
                        --i;

                        GUI.PopID();

                        continue;
                    }

                    GUI.SameLine();

                    if (GUI.StructView($"Shader Buffer[{i}]"))
                    {
                        GUI.Indent();

                        GUI.EnumField("Buffer Type", ref input.BufferType);
                        switch (input.BufferType)
                        {
                        case ShaderBufferType.PModelBuffer:
                        case ShaderBufferType.PUIBuffer:
                        case ShaderBufferType.PShadowLightBuffer:
                        {
                            break;
                        }
                        default:
                        {
                            uint slot = input.Slot;
                            if (GUI.RUIntField("Slot", ref slot, 0))
                            {
                                input.Slot = (ushort)slot;
                            }
                            uint set = input.Set;
                            if (GUI.RUIntField("Set", ref set, 0))
                            {
                                input.Set = (ushort)set;
                            }

                            break;
                        }
                        }
                        
                        GUI.EnumField("Shader Slot", ref input.ShaderSlot);

                        def.ShaderBuffers[i] = input;

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

            GUI.StringField("Shadow Vertex Shader Path", ref def.ShadowVertexShaderPath);
            GUI.Tooltip("Shadow Vertex Shader Path", "Path relative to the project for the vertex shader file to be used for shadows.");

            if (!string.IsNullOrWhiteSpace(def.ShadowVertexShaderPath))
            {
                show = GUI.ArrayView("Shadow Vertex Attributes", out addValue);
                GUI.Tooltip("Shadow Vertex Attribute", "Used to determine input values for shaders.");

                if (addValue)
                {
                    if (def.ShadowShaderBuffers == null)
                    {
                        def.ShadowShaderBuffers = new List<ShaderBufferInput>();
                    }

                    def.ShadowShaderBuffers.Add(new ShaderBufferInput());
                }

                if (show && def.ShadowShaderBuffers != null)
                {
                    GUI.Indent();
                    GUI.PushID("ShadowShaderBuffers");

                    for (int i = 0; i < def.ShadowShaderBuffers.Count; ++i)
                    {
                        ShaderBufferInput input = def.ShadowShaderBuffers[i];
                        GUI.PushID($"[{i}]");

                        if (GUI.Button("-"))
                        {
                            def.ShadowShaderBuffers.RemoveAt(i);
                            --i;

                            GUI.PopID();

                            continue;
                        }

                        GUI.SameLine();

                        if (GUI.StructView($"Shadow Shader Buffer[{i}]"))
                        {
                            GUI.Indent();

                            GUI.EnumField("Buffer Type", ref input.BufferType);
                            switch (input.BufferType)
                            {
                            case ShaderBufferType.PModelBuffer:
                            case ShaderBufferType.PUIBuffer:
                            case ShaderBufferType.PShadowLightBuffer:
                            {
                                break;
                            }
                            default:
                            {
                                uint slot = input.Slot;
                                if (GUI.RUIntField("Slot", ref slot, 0))
                                {
                                    input.Slot = (ushort)slot;
                                }
                                uint set = input.Set;
                                if (GUI.RUIntField("Set", ref set, 0))
                                {
                                    input.Set = (ushort)set;
                                }

                                break;
                            }
                            }

                            GUI.EnumField("Shader Slot", ref input.ShaderSlot);

                            def.ShadowShaderBuffers[i] = input;

                            GUI.Unindent();
                        }

                        GUI.PopID();
                    }

                    GUI.PopID();
                    GUI.Unindent();
                }
            }

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