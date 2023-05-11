using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(SelectionObject))]
    public class SelectionObjectPropertiesWindow : PropertiesWindow
    {
        static GameObjectPropertiesWindow s_gameObjectWindow = new GameObjectPropertiesWindow();

        public override void OnGUI(object a_object)
        {
            SelectionObject selectionObject = (SelectionObject)a_object;

            switch (selectionObject.SelectionMode)
            {
            case SelectionObjectMode.SceneObject:
            {
                GUI.RVec3Field("Translation", ref selectionObject.SceneObject.Translation, Vector3.Zero);
                GUI.RQuaternionField("Rotation", ref selectionObject.SceneObject.Rotation, Quaternion.Identity);
                GUI.RVec3Field("Scale", ref selectionObject.SceneObject.Scale, Vector3.One);

                GUI.StringField("DefName", ref selectionObject.SceneObject.DefName);

                break;
            }
            case SelectionObjectMode.GameObjectDef:
            {
                GameObjectDef def = selectionObject.GameObject;
                // Allow inline editing for scene objects
                if (def != null && def.IsSceneDef)
                {
                    s_gameObjectWindow.OnGUI(def);  
                }

                break;
            }
            }

            a_object = selectionObject;
        }
    }
}