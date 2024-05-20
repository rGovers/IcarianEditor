using IcarianEngine.Definitions;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(PointLightDef))]
    public class PointLightPropertiesWindow : LightPropertiesWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            PointLightDef def = a_object as PointLightDef;
            if (def == null)
            {
                return;
            }

            GUI.FloatField("Radius", ref def.Radius);

            base.OnGUI(a_object, a_sceneObject);
        }
    }
}