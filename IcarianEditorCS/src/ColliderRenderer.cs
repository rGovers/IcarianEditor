// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor
{
    public static class ColliderRenderer
    {
        public static void DrawCollider(Matrix4 a_mat, CollisionShapeDef a_def, Color a_color)
        {
            if (a_def == null)
            {
                return;
            }

            switch (a_def)
            {
            case SphereCollisionShapeDef def:
            {
                Gizmos.DrawIcoSphere(a_mat[3].XYZ, def.Radius, 1, 0.01f, a_color);

                break;
            }
            case BoxCollisionShapeDef def:
            {
                Vector3 halfExtents = def.Extents * 0.5f;

                Vector3 pos;
                Vector3 right;
                Vector3 up;
                Vector3 forward;
                Vector3 scale;
                Matrix4.Decompose(a_mat, out pos, out right, out up, out forward, out scale);                

                right *= halfExtents.X;
                up *= halfExtents.Y;
                forward *= halfExtents.Z;

                Gizmos.DrawLine(pos + forward + up + right, pos + forward + up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward + up + right, pos - forward + up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward + up + right, pos - forward + up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward + up - right, pos - forward + up - right, 0.01f, a_color);

                Gizmos.DrawLine(pos + forward - up + right, pos + forward - up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward - up + right, pos - forward - up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward - up + right, pos - forward - up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward - up - right, pos - forward - up - right, 0.01f, a_color);

                Gizmos.DrawLine(pos + forward + up + right, pos + forward - up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward + up - right, pos + forward - up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward + up + right, pos - forward - up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward + up - right, pos - forward - up - right, 0.01f, a_color);

                break;
            }
            case CapsuleCollisionShapeDef def:
            {
                Gizmos.DrawCapsule(a_mat[3].XYZ, def.Height, def.Radius, 8, 0.01f, a_color);

                break;
            }
            case CylinderCollisionShapeDef def:
            {
                Gizmos.DrawCylinder(a_mat[3].XYZ, def.Height, def.Radius, 8, 0.01f, a_color);

                break;
            }
            }
        }
    };
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