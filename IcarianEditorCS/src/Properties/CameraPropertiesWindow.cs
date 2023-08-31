using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(CameraDef))]
    public class CameraPropertiesWindow : PropertiesWindow
    {
        public override void OnGUI(object a_object)
        {
            CameraDef def = a_object as CameraDef;
            if (def == null)
            {
                return;
            }

            GUI.FloatField("FOV", ref def.FOV);
            GUI.Tooltip("FOV", "Field of View for the camera.");
            GUI.RFloatField("Near", ref def.Near, 0.1f);
            GUI.Tooltip("Near", "Near clipping plane for the camera.");
            GUI.RFloatField("Far", ref def.Far, 100.0f);
            GUI.Tooltip("Far", "Far clipping plane for the camera.");
            GUI.BitField("Render Layer", ref def.RenderLayer, 32);

            if (GUI.StructView("Viewport"))
            {
                GUI.Tooltip("Viewport", "Viewport to determine the portion of the screen rendered to.");
                GUI.Indent();

                GUI.RVec2Field("Position", ref def.Viewport.Position, Vector2.Zero);
                GUI.RVec2Field("Size", ref def.Viewport.Size, Vector2.One);

                GUI.Unindent();
            }

            if (GUI.StructView("Render Texture"))
            {
                GUI.Tooltip("Render Texture", "Render Texture to render to instead of the screen.");
                GUI.Indent();

                GUI.RUIntField("Count", ref def.RenderTexture.Count, 0);
                if (def.RenderTexture.Count > 0)
                {   
                    GUI.RUIntField("Width", ref def.RenderTexture.Width, uint.MaxValue);
                    GUI.RUIntField("Height", ref def.RenderTexture.Height, uint.MaxValue);
                    GUI.Checkbox("HDR", ref def.RenderTexture.HDR);
                }

                GUI.Unindent();
            }
        }
    }
}