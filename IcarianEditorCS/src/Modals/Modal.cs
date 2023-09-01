using IcarianEngine;
using IcarianEngine.Maths;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace IcarianEditor.Modals
{
    public abstract class Modal : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushModal(string a_title, Vector2 a_size, uint a_index);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushModalState(uint a_index, uint a_state);

        static List<Modal> s_modals = new List<Modal>();

        bool m_disposed = false;

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public Modal(string a_title) : this(a_title, Vector2.Zero)
        {
            
        }
        public Modal(string a_title, Vector2 a_size)
        {
            int count = s_modals.Count;
            uint index = (uint)count;
            for (int i = 0; i < count; i++)
            {
                if (s_modals[(int)i] == null || s_modals[(int)i].IsDisposed)
                {
                    index = (uint)i;

                    s_modals[(int)i] = this;
                }
            }

            if (index == count)
            {
                s_modals.Add(this);
            }

            PushModal(a_title, a_size, index);
        }

        public abstract bool Update();

        static void UpdateModal(uint a_index)
        {
            if (a_index > s_modals.Count)
            {
                Logger.Error("Modal index out of range");
            }

            Modal modal = s_modals[(int)a_index];

            if (modal == null || modal.IsDisposed)
            {
                Logger.Error("Modal is null or disposed");
            }

            if (modal.Update())
            {
                PushModalState(a_index, 1);
            }
            else
            {
                PushModalState(a_index, 0);
            }
        }

        static void DisposeModal(uint a_index)
        {
            if (a_index > s_modals.Count)
            {
                Logger.Error("Modal index out of range");
            }

            Modal modal = s_modals[(int)a_index];
            s_modals[(int)a_index] = null;

            if (modal == null || modal.IsDisposed)
            {
                Logger.Error("Modal is null or disposed");
            }

            modal.Dispose();
        }

        void Dispose(bool a_disposing)
        {
            if (a_disposing)
            {
                if (!m_disposed)
                {
                    s_modals.Remove(this);
                }
                else
                {
                    Logger.Warning("Modal already disposed");
                }

                m_disposed = true;
            }
            else
            {
                Logger.Error("Modal not disposed");
            }
        }
        ~Modal()
        {
            Dispose(false);
        }
        public void Dispose()
        {
            Dispose(true);

            System.GC.SuppressFinalize(this);
        }
    }
}