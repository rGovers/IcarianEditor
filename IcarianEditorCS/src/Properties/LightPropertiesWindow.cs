using IcarianEngine.Definitions;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(LightDef))]
    public class LightPropertiesWindow : PropertiesEditorWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            LightDef def = a_object as LightDef;
            if (def == null)
            {
                return;
            }

            GUI.ColorField("Color", ref def.Color);
            GUI.Tooltip("Color", "Color of the light.");

            GUI.FloatField("Intensity", ref def.Intensity);
            GUI.Tooltip("Intensity", "Intensity of the light.");

            GUI.BitField("Render Layer", ref def.RenderLayer, 32);
        }
    }
}