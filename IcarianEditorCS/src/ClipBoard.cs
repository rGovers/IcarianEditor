// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

using System.Collections.Generic;

namespace IcarianEditor
{
    public enum ClipBoardItemType
    {
        Null,
        SceneObject
    }

    public struct ClipBoardItem
    {
        public ClipBoardItemType ItemType;
        public object Data;
    }

    // May need to move to C++ at some point but just in C# at this point 
    public static class ClipBoard
    {
        const uint ClipBoardSize = 128;

        static List<ClipBoardItem> s_items;

        internal static void Init()
        {
            s_items = new List<ClipBoardItem>();
        }

        // Not the most efficent but will deal with it later if it becomes an issue
        public static void AddItem(ClipBoardItemType a_type, object a_data)
        {
            ClipBoardItem item = new ClipBoardItem();
            item.ItemType = a_type;
            item.Data = a_data;

            s_items.Insert(0, item);

            int count = s_items.Count;
            if (count > ClipBoardSize)
            {
                s_items.RemoveAt(count);
            }
        }

        public static void Clear()
        {
            s_items.Clear();
        }

        public static bool GetItem(ClipBoardItemType a_type, out ClipBoardItem a_item)
        {
            a_item = new ClipBoardItem();

            foreach (ClipBoardItem item in s_items)
            {
                if (item.ItemType == a_type)
                {
                    a_item = item;

                    return true;
                }
            }

            return false;
        }
        public static bool GetItem(ClipBoardItemType[] a_types, out ClipBoardItem a_item)
        {
            a_item = new ClipBoardItem();

            foreach (ClipBoardItem item in s_items)
            {
                foreach (ClipBoardItemType type in a_types)
                {
                    if (item.ItemType == type)
                    {
                        a_item = item;

                        return true;
                    }
                }
            }

            return false;
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