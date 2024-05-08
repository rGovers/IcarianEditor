using IcarianEngine;
using IcarianEngine.Rendering;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "InteropBinding.h"
#include "EditorFileHandlerInterop.h"
#include "EditorFileHandlerInteropStructures.h"

FILEHANDLER_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEditor
{
    public static class FileHandler
    {
        internal static void Update()
        {
            
        }

        internal static void Clear()
        {

        }

        static void GetFileHandle(string a_file)
        {
            FileTextureHandle handle = new FileTextureHandle();
            handle.Addr = uint.MaxValue;
            handle.Mode = FileTextureMode.Null;

            switch (Path.GetExtension(a_file))
            {
            case ".png":
            case ".ktx2":
            {
                Texture tex = AssetLibrary.LoadTexture(a_file);
                if (tex != null)
                {
                    Type type = typeof(Texture);
                    FieldInfo info = type.GetField("m_bufferAddr", BindingFlags.NonPublic | BindingFlags.Instance);

                    handle.Addr = (uint)info.GetValue(tex);
                    handle.Mode = FileTextureMode.Texture;
                }

                break;
            }
            }

            FileHandlerInterop.SetFileHandle(handle.Addr, (uint)handle.Mode);
        }
    }
}