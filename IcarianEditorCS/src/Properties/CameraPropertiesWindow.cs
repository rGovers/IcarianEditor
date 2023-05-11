using IcarianEngine.Definitions;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(CameraDef))]
    public class CameraProperties : GameObjectPropertiesWindow
    {
        static readonly CameraDef s_default = new CameraDef();

        public override void OnGUI(object a_object)
        {
            base.OnGUI(a_object);

            CameraDef def = a_object as CameraDef;
            if (def == null)
            {
                return;
            }

            GUI.RFloatField("FOV", ref def.FOV, s_default.FOV);

            GUI.RFloatField("Near", ref def.Near, s_default.Near);
            GUI.RFloatField("Far", ref def.Far, s_default.Far);

            GUI.UIntField("Render Layer", ref def.RenderLayer);

            if (GUI.StructView("Viewport"))
            {
                GUI.RVec2Field("Position", ref def.Viewport.Position, s_default.Viewport.Position);
                GUI.RVec2Field("Size", ref def.Viewport.Size, s_default.Viewport.Size);

                GUI.RFloatField("Min Depth", ref def.Viewport.MinDepth, s_default.Viewport.MinDepth);
                GUI.RFloatField("Max Depth", ref def.Viewport.MaxDepth, s_default.Viewport.MaxDepth);
            }
        }
    }
}