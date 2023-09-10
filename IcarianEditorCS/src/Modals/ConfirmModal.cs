using IcarianEngine.Maths;

namespace IcarianEditor.Modals
{
    public class ConfirmModal : Modal
    {
        public delegate void ConfirmModalCallback();

        string               m_text;

        ConfirmModalCallback m_callback;

        public ConfirmModal(string a_text, ConfirmModalCallback a_callback) : base("Confirm", new Vector2(300.0f, 80.0f))
        {
            m_text = a_text;

            m_callback = a_callback;
        }

        public override bool Update()
        {
            GUI.Label(m_text);

            if (GUI.Button("Ok"))
            {
                if (m_callback != null)
                {
                    m_callback();
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