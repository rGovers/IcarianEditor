using IcarianEditor.Editor;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics;

namespace IcarianEditor
{
    [EDisplay(typeof(RigidBody))]
    public class RigidBodyEditorDisplay : EditorDisplay
    {
        public override void Render(bool a_selected, Def a_component, Matrix4 a_transform)
        {
            if (!a_selected)
            {
                return;
            }

            RigidBodyDef def = a_component as RigidBodyDef;
            if (def == null)
            {
                return;
            }

            ColliderRenderer.DrawCollider(a_transform, def.CollisionShape, Color.Red);
        }
    }
}