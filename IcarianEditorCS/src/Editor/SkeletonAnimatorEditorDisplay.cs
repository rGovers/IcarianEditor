using IcarianEngine;
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

            // Engine need full def to work properly cannot do partial generation
            SkeletonAnimatorDef skeletonAnimatorDef = EditorDefLibrary.GenerateDef<SkeletonAnimatorDef>(def.DefName, true);
            if (skeletonAnimatorDef == null)
            {
                return;
            }

            AnimationMaster.UpdateSkeleton(skeletonAnimatorDef);
            
            if (a_selected)
            {
                AnimationMaster.DrawBones(a_transform, AssetLibrary.LoadSkeleton(skeletonAnimatorDef.SkeletonPath));
            }
        }
    }
}