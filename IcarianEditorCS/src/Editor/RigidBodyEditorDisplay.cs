// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics;

namespace IcarianEditor.Editor
{
    [EDisplay(typeof(RigidBody))]
    public class RigidBodyEditorDisplay : EditorDisplay
    {
        public override bool Render(bool a_selected, Def a_component, Matrix4 a_transform, Matrix4 a_view, Matrix4 a_proj, uint a_screenWidth, uint a_screenHeight)
        {
            if (!a_selected)
            {
                return false;
            }

            RigidBodyDef def = a_component as RigidBodyDef;
            if (def == null || def.CollisionShape == null)
            {
                return false;
            }

            CollisionShapeDef shapeDef = EditorDefLibrary.GenerateDef(def.CollisionShape.DefName) as CollisionShapeDef;
            if (shapeDef == null)
            {
                return false;
            }

            ColliderRenderer.DrawCollider(a_transform, shapeDef, Color.Red);

            return false;
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