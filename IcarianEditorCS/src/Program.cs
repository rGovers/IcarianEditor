// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using IcarianEditor.Windows;
using IcarianEngine;
using IcarianEngine.Definitions;
using System;
using System.Reflection;

namespace IcarianEditor
{
    internal static class Program
    {
        static void Load()
        {
            EditorDefLibrary.Init();

            Type assetLibraryType = typeof(AssetLibrary);
            MethodInfo assetLibraryInitMethod = assetLibraryType.GetMethod("Init", BindingFlags.Static | BindingFlags.NonPublic);
            assetLibraryInitMethod.Invoke(null, new object[] { });

            ClipBoard.Init();
            PropertiesWindow.Init();
            EditorWindow.Init();
        }

        static void Update(double a_delta)
        {
            AnimationMaster.Update(a_delta);
            FileHandler.Update();
        }

        static void Unload()
        {
            ClipBoard.Clear();
            AssetLibrary.ClearAssets();
            EditorDefLibrary.Clear();
            AnimationMaster.Destroy();
            FileHandler.Clear();
        }
        
        // On Windows seems to need a main function and be executable to work
        // Needs it even if it is not used for some odd reason
        static void Main(string[] a_args)
        {

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