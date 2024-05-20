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