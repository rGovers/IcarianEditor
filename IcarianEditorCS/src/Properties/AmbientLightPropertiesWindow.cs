using IcarianEngine.Definitions;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(AmbientLightDef))]
    public class AmbientLightPropertiesWindow : LightPropertiesWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            base.OnGUI(a_object, a_sceneObject);
        }
    }
}