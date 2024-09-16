// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(ParticleSystem2DDef))]
    public class ParticleSystem2DWindow : PropertiesEditorWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            ParticleSystem2DDef def = a_object as ParticleSystem2DDef;
            if (def == null)
            {
                return;
            }

            ParticleEmitterType type = def.EmitterType;
            if (GUI.EnumField("Emitter Type", ref type))
            {
                def.EmitterType = type;
            }

            switch (type)
            {
            case ParticleEmitterType.Point:
            {
                break;
            }
            default:
            {
                break;
            }
            }

            GUI.UIntField("Max Particles", ref def.MaxParticles);

            GUI.BitField("Render Layer", ref def.RenderLayer);

            GUI.Checkbox("Auto Play", ref def.AutoPlay);
            GUI.Checkbox("Burst", ref def.Burst);
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