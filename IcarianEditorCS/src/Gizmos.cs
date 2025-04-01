// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EditorGizmosInteropStructures.h"

namespace IcarianEditor
{
    public static class Gizmos
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetManipulating();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static TransformValue GetManipulation(uint a_mode, Vector3 a_translation, Quaternion a_rotation, Vector3 a_scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawLine(Vector3 a_start, Vector3 a_end, float a_width, Vector4 a_color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void MultiDrawLine(Vector3 a_start, Vector3 a_end, float a_width, Vector4 a_color, Vector3 a_dir, float a_delta, uint a_count);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawUVSphere(Vector3 a_pos, float a_radius, uint a_subDivisions, float a_width, Vector4 a_color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawIcoSphere(Vector3 a_pos, float a_radius, uint a_subDivisions, float a_width, Vector4 a_color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawCylinder(Vector3 a_pos, float a_height, float a_radius, uint a_subDivisions, float a_width, Vector4 a_color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawCapsule(Vector3 a_pos, float a_height, float a_radius, uint a_subDivisions, float a_width, Vector4 a_color);

        public static bool IsManipulating
        {
            get
            {
                return GetManipulating() != 0;
            }
        }

        public static void DrawLine(Vector3 a_start, Vector3 a_end, float a_width, Color a_color)
        {
            DrawLine(a_start, a_end, a_width, a_color.ToVector4());
        }
        public static void MultiDrawLine(Vector3 a_start, Vector3 a_end, float a_width, Color a_color, Vector3 a_dir, float a_delta, uint a_count)
        {
            MultiDrawLine(a_start, a_end, a_width, a_color.ToVector4(), a_dir, a_delta, a_count);
        }

        public static void DrawUVSphere(Vector3 a_pos, float a_radius, uint a_subDivisions, float a_width, Color a_color)
        {
            DrawUVSphere(a_pos, a_radius, a_subDivisions, a_width, a_color.ToVector4());
        }
        public static void DrawIcoSphere(Vector3 a_pos, float a_radius, uint a_subDivisions, float a_width, Color a_color)
        {
            DrawIcoSphere(a_pos, a_radius, a_subDivisions, a_width, a_color.ToVector4());
        }

        public static void DrawCylinder(Vector3 a_pos, float a_height, float a_radius, uint a_subDivisions, float a_width, Color a_color)
        {
            DrawCylinder(a_pos, a_height, a_radius, a_subDivisions, a_width, a_color.ToVector4());
        }
        public static void DrawCapsule(Vector3 a_pos, float a_height, float a_radius, uint a_subDivisions, float a_width, Color a_color)
        {
            DrawCapsule(a_pos, a_height, a_radius, a_subDivisions, a_width, a_color.ToVector4());
        }

        public static bool Manipulation(ManipulationMode a_mode, ref Vector3 a_translation, ref Quaternion a_rotation, ref Vector3 a_scale)
        {
            TransformValue val = GetManipulation((uint)a_mode, a_translation, a_rotation, a_scale);

            bool ret = false;
            if (val.Translation != a_translation)
            {
                ret = true;
                a_translation = val.Translation;
            }

            if (val.Rotation != a_rotation)
            {
                ret = true;
                a_rotation = val.Rotation;
            }

            if (val.Scale != a_scale)
            {
                ret = true;
                a_scale = val.Scale;
            }

            return ret;
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