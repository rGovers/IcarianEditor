// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using System;

namespace IcarianEditor.Engine
{
    public class CameraController : Component
    {
        Camera m_cam;

        void ReceiveMessage(string a_type, byte[] a_data)
        {
            if (a_type != "Editor:CameraTransform")
            {
                return;
            }

            float[] dat = new float[16];
            for (uint i = 0; i < 16; ++i)
            {
                dat[i] = BitConverter.ToSingle(a_data, (int)i * sizeof(float));
            }

            Transform.SetMatrix(new Matrix4(dat));
        }

        public override void Init()
        {
            PipeMessage.AddCallback("Editor:CameraTransform", ReceiveMessage);

            m_cam = AddComponent<Camera>();
            m_cam.FOV = Mathf.PI * 0.4f;
            m_cam.Far = 1000.0f;
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
