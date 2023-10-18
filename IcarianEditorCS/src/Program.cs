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

            PropertiesWindow.Init();
            EditorWindow.Init();
        }

        static void Update(double a_delta)
        {
            AnimationMaster.Update(a_delta);
        }

        static void Unload()
        {
            AssetLibrary.ClearAssets();
            EditorDefLibrary.Clear();
            AnimationMaster.Destroy();
        }
        
        // On Windows seems to need a main function and be executable to work
        // Needs it even if it is not used for some odd reason
        static void Main(string[] a_args)
        {

        }
    }
}