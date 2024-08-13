// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using System.Collections.Generic;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Modals
{
    public class GUIGetDefSelectorModal<T> : Modal where T : Def
    {
        string       m_input;
        int          m_seletedIndex;
        List<string> m_defNames;

        public GUIGetDefSelectorModal(string a_input, Def a_currentDef, bool a_useSceneDefs) : base("Def Selector", new Vector2(250, 80))
        {
            m_input = a_input;
            m_seletedIndex = 0;

            m_defNames = new List<string>();

            IEnumerable<T> defs = EditorDefLibrary.GenerateDefs<T>();

            m_defNames.Add("Null");

            if (a_useSceneDefs)
            {
                foreach (T def in defs)
                {
                    if (def == a_currentDef)
                    {
                        m_seletedIndex = m_defNames.Count;
                    }

                    m_defNames.Add(def.DefName);                    
                }
            }
            else
            {
                foreach (T def in defs)
                {
                    if (def.IsSceneDef)
                    {
                        continue;
                    }

                    if (def == a_currentDef)
                    {
                        m_seletedIndex = m_defNames.Count;
                    }

                    m_defNames.Add(def.DefName);
                }
            }
        }

        public override bool Update()
        {
            GUI.StringSelector("Def", m_defNames.ToArray(), ref m_seletedIndex);

            if (GUI.Button("Ok"))
            {
                if (m_seletedIndex <= 0)
                {
                    GUI.PushDef(m_input, null);
                }
                else
                {
                    GUI.PushDef(m_input, EditorDefLibrary.GenerateDef(m_defNames[m_seletedIndex]));
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