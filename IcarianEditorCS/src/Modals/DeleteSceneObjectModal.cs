// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Modals
{
    public class DeleteSceneObjectModal : Modal
    {
        ulong m_id;
        bool  m_sceneDef;
        bool  m_deleteDef;

        public DeleteSceneObjectModal(ulong a_id) : base("Delete Scene Object", new Vector2(375.0f, 100.0f))
        {
            m_id = a_id;

            m_sceneDef = false;
            m_deleteDef = false;

            EditorScene scene = Workspace.GetScene();

            SceneObject obj = scene.GetSceneObject(m_id);
            m_sceneDef = EditorDefLibrary.IsSceneDef(obj.DefName);
        }

        public override bool Update()
        {
            GUI.Label("Are you sure you want to delete this scene object?");

            GUI.Separator();

            if (m_sceneDef)
            {
                GUI.Checkbox("Delete Def", ref m_deleteDef);
            }

            if (GUI.Button("Ok"))
            {
                EditorScene scene = Workspace.GetScene();
                
                if (m_deleteDef)
                {
                    SceneObject obj = scene.GetSceneObject(m_id);

                    scene.RemoveDef(obj.DefName);
                }

                Workspace.ClearSelection();
                
                scene.RemoveSceneObject(m_id);

                return false;
            }

            GUI.SameLine();

            if (GUI.Button("Cancel"))
            {
                return false;
            }

            return true;
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