// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEditor.Modals
{
    public class AddComponentModal : Modal
    {
        struct ComponentData
        {
            public string Name;
            public ComponentDef Def;
        }

        int                  m_selection;
        List<ComponentData>  m_components;
        GameObjectDef        m_def;

        public AddComponentModal(GameObjectDef a_def) : base("Add Component", new Vector2(250, 80))
        {
            m_def = a_def;

            m_selection = 0;
            m_components = new List<ComponentData>();

            IEnumerable<ComponentDef> defs = EditorDefLibrary.GenerateDefs<ComponentDef>();
            foreach (ComponentDef def in defs)
            {
                m_components.Add(new ComponentData()
                {
                    Name = def.DefName,
                    Def = def
                });
            }
        }

        public override bool Update()
        {
            int count = m_components.Count;
            string[] names = new string[count];
            for (int i = 0; i < count; ++i)
            {
                names[i] = m_components[i].Name;
            }
            
            GUI.StringSelector("ComponentDef", names, ref m_selection);

            if (GUI.Button("Ok"))
            {
                if (m_selection < 0 || m_selection >= count)
                {
                    Logger.Error("Invalid component selection");

                    return true;
                }

                ComponentData data = m_components[m_selection];

                m_def.Components.Add(data.Def);

                EditorDefLibrary.RebuildDefData(m_def);

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