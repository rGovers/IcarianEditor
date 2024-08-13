// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using System.Collections.Generic;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Modals
{
    public class NewSceneObjectModal : Modal
    {
        bool         m_array; 
        int          m_selection;
        List<string> m_defNames;

        public NewSceneObjectModal(bool a_array = false) : base("New Scene Object", new Vector2(300.0f, 80.0f))
        {
            m_array = a_array;
            m_selection = 0;
            m_defNames = new List<string>();

            IEnumerable<GameObjectDef> defs = EditorDefLibrary.GenerateDefs<GameObjectDef>();
            foreach (GameObjectDef def in defs)
            {
                m_defNames.Add(def.DefName);
            }
        }

        public override bool Update()
        {
            string[] names = m_defNames.ToArray();
            GUI.StringSelector("GameObjectDef", names, ref m_selection);

            if (GUI.Button("Ok"))
            {
                if (m_selection < 0 || m_selection >= m_defNames.Count)
                {
                    Logger.Error("Invalid selection index");

                    return true;
                }

                EditorScene scene = Workspace.GetScene();
                if (m_array)
                {
                    scene.AddSceneObjectArray(m_defNames[m_selection]);
                }
                else
                {
                    scene.AddSceneObject(m_defNames[m_selection]);
                }

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