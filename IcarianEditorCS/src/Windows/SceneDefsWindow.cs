// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEditor.Modals;
using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor.Windows
{
    public static class SceneDefsWindow
    {
        static string s_searchString = string.Empty;

        static void OnGUI()
        {
            EditorScene scene = Workspace.GetScene();
            if (scene == null)
            {
                return;
            }

            if (GUI.Button("Create Def"))
            {
                new CreateSceneDefModal();
            }

            GUI.SameLine();

            GUI.StringField("Search", ref s_searchString);

            if (GUI.BeginChild("##DefList", Vector2.Zero))
            {
                IEnumerable<string> defNames = scene.DefNames;

                foreach (string defName in defNames)
                {
                    if (!string.IsNullOrWhiteSpace(s_searchString) && !defName.Contains(s_searchString))
                    {
                        continue;
                    }

                    GUI.PushID(defName);

                    if (GUI.Button("-"))
                    {
                        new ConfirmModal($"Delete {defName}?", () =>
                        {
                            EditorScene scene = Workspace.GetScene();
                            scene.RemoveDef(defName);

                            Workspace.ClearSelection();
                        });

                        GUI.PopID();

                        break;
                    }

                    GUI.SameLine();

                    if (GUI.Selectable(defName))
                    {
                        Workspace.ClearSelection();

                        Workspace.AddDefSelection(defName);
                    }

                    GUI.PopID();
                }

                GUI.EndChild();
            }
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