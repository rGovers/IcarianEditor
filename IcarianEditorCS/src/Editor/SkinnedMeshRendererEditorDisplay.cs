using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;

namespace IcarianEditor.Editor
{
    [EDisplay(typeof(SkinnedMeshRenderer))]
    public class SkinnedMeshRendererEditorDisplay : EditorDisplay
    {
        public override void Render(bool a_selected, Def a_component, Matrix4 a_transform)
        {
            SkinnedMeshRendererDef def = a_component as SkinnedMeshRendererDef;
            if (def == null)
            {
                return;
            }

            Model model = AssetLibrary.LoadSkinnedModel(def.ModelPath);
            if (model == null)
            {
                return;
            }

            Skeleton skeleton = AssetLibrary.LoadSkeleton(def.SkeletonPath);
            if (skeleton == null)
            {
                return;
            }

            MaterialDef matDef = EditorDefLibrary.GenerateDef<MaterialDef>(def.MaterialDef.DefName);
            if (matDef == null)
            {
                return;
            }
            matDef.PostResolve();

            Material mat = AssetLibrary.GetMaterial(matDef);
            if (mat == null)
            {
                return;
            }

            RenderCommand.BindMaterial(mat);

            AnimationMaster.DrawSkeleton(skeleton, model, a_transform);
        }
    }
}