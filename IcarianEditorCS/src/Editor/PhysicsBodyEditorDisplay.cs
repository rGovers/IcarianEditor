using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics;

namespace IcarianEditor.Editor
{
    [EDisplay(typeof(PhysicsBody))]
    public class PhysicsBodyEditorDisplay : EditorDisplay
    {
        public override void Render(bool a_selected, Def a_component, Matrix4 a_transform)
        {
            if (!a_selected)
            {
                return;
            }

            PhysicsBodyDef def = a_component as PhysicsBodyDef;
            if (def == null)
            {
                return;
            }

            ColliderRenderer.DrawCollider(a_transform, def.CollisionShape, Color.Green);
        }
    }
}