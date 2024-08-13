// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(CameraDef))]
    public class CameraPropertiesWindow : PropertiesEditorWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            CameraDef def = a_object as CameraDef;
            if (def == null)
            {
                return;
            }

            if (EditorConfig.UseDegrees)
            {
                float fov = def.FOV * Mathf.RadToDeg;
                if (GUI.FloatSliderField("FOV", ref fov, 0.1f * Mathf.RadToDeg, Mathf.PI * Mathf.RadToDeg))
                {
                    def.FOV = fov * Mathf.DegToRad;
                }
            }
            else
            {
                GUI.FloatSliderField("FOV", ref def.FOV, 0.1f, Mathf.PI);
            }
            GUI.Tooltip("FOV", "Field of View for the camera.");
            
            GUI.RFloatField("Near", ref def.Near, 0.1f);
            GUI.Tooltip("Near", "Near clipping plane for the camera.");
            GUI.RFloatField("Far", ref def.Far, 100.0f);
            GUI.Tooltip("Far", "Far clipping plane for the camera.");
            GUI.BitField("Render Layer", ref def.RenderLayer, 32);

            if (GUI.StructView("Viewport"))
            {
                GUI.Tooltip("Viewport", "Viewport to determine the portion of the screen rendered to.");
                GUI.Indent();

                GUI.RVec2Field("Position", ref def.Viewport.Position, Vector2.Zero);
                GUI.RVec2Field("Size", ref def.Viewport.Size, Vector2.One);

                GUI.Unindent();
            }

            if (GUI.StructView("Render Texture"))
            {
                GUI.Tooltip("Render Texture", "Render Texture to render to instead of the screen.");
                GUI.Indent();

                GUI.RUIntField("Count", ref def.RenderTexture.Count, 0);
                if (def.RenderTexture.Count > 0)
                {   
                    GUI.RUIntField("Width", ref def.RenderTexture.Width, uint.MaxValue);
                    GUI.RUIntField("Height", ref def.RenderTexture.Height, uint.MaxValue);
                    GUI.Checkbox("HDR", ref def.RenderTexture.HDR);
                }

                GUI.Unindent();
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