using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering.Animation;

namespace IcarianEditor.Editor
{
    [EDisplay(typeof(SkeletonAnimator))]
    public class SkeletonAnimatorEditorDisplay : EditorDisplay
    {
        public override void Render(bool a_selected, Def a_component, Matrix4 a_transform)
        {
            SkeletonAnimatorDef def = a_component as SkeletonAnimatorDef;
            if (def == null)
            {
                return;
            }

            AnimationMaster.UpdateSkeleton(def);
            
            if (a_selected)
            {
                Gizmos.DrawIcoSphere(a_transform[3].XYZ, 0.1f, 1, 0.01f, Color.Blue);
            }
        }
    }
}