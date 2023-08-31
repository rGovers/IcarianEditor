using IcarianEngine.Definitions;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(SpotLightDef))]
    public class SpotLightPropertiesWindow : LightPropertiesWindow
    {
        public override void OnGUI(object a_object)
        {
            SpotLightDef def = a_object as SpotLightDef;
            if (def == null)
            {
                return;
            }

            GUI.FloatField("Radius", ref def.Radius);
            GUI.Tooltip("Radius", "Radius of the light.");

            GUI.FloatField("Inner Cutoff Angle", ref def.InnerCutoffAngle);
            GUI.Tooltip("Inner Cutoff Angle", "Inner cutoff angle of the light.");

            GUI.FloatField("Outer Cutoff Angle", ref def.OuterCutoffAngle);
            GUI.Tooltip("Outer Cutoff Angle", "Outer cutoff angle of the light.");

            base.OnGUI(a_object);
        }
    }
}