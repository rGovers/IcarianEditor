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

            if (GUI.FloatField("Emitter Velocity Scale", ref def.EmitterVelocityScale))
            {
                def.EmitterVelocityScale = Mathf.Max(0.0f, def.EmitterVelocityScale);
            }

            GUI.UIntField("Max Particles", ref def.MaxParticles);
            GUI.FloatSliderField("Emitter Ratio", ref def.EmitterRatio, 0.0f, 1.0f);

            GUI.BitField("Render Layer", ref def.RenderLayer);

            if (GUI.FloatField("Lifetime", ref def.Lifetime))
            {
                def.Lifetime = Mathf.Max(0.0f, def.Lifetime);
            }

            if (GUI.FloatField("Start Size", ref def.StartSize))
            {
                def.StartSize = Mathf.Max(0.0f, def.StartSize);
            }
            if (GUI.FloatField("End Size", ref def.EndSize))
            {
                def.EndSize = Mathf.Max(0.0f, def.EndSize);
            }

            GUI.ColorField("Start Color", ref def.StartColor);
            GUI.ColorField("End Color", ref def.EndColor);

            GUI.Vec3Field("Initial Velocity", ref def.InitialVelocity);
            GUI.Vec3Field("Gravity", ref def.Gravity);

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