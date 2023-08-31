using IcarianEngine.Definitions;
using IcarianEngine.Rendering;
using System.Collections.Generic;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(MaterialDef))]
    public class MaterialPropertiesWindow : PropertiesWindow
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

                    if (GUI.StructView($"Shader Buffer[{i}]"))
                    {
                        GUI.Indent();

                        GUI.EnumField("Buffer Type", ref input.BufferType);
                        switch (input.BufferType)
                        {
                        case ShaderBufferType.ModelBuffer:
                        case ShaderBufferType.UIBuffer:
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
        }
    }
}