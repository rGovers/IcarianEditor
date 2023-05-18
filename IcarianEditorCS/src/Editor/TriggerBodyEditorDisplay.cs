using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics;

namespace IcarianEditor.Editor
{
    [EDisplay(typeof(TriggerBody))]
    public class TriggerBodyEditorDisplay : EditorDisplay
    {
        public override void Render(bool a_selected, Def a_component, Matrix4 a_transform)
        {
            if (!a_selected)
            {
                return;
            }

            TriggerBodyDef def = a_component as TriggerBodyDef;
            if (def == null)
            {
                return;
            }

            ColliderRenderer.DrawCollider(a_transform, def.CollisionShape, Color.Blue);
        }
    }
}