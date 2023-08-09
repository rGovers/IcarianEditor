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

            // Material mat = AssetLibrary.GetMaterial(def.MaterialDef);
            // if (mat == null)
            // {
            //     return;
            // }

            Gizmos.DrawIcoSphere(a_transform[3].XYZ, 0.5f, 2, 0.01f, Color.Red);
        }
    }
}