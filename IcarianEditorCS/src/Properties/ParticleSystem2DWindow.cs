using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;

namespace IcarianEditor.Properties
{
    [PWindow(typeof(ParticleSystem2DDef))]
    public class ParticleSystem2DWindow : PropertiesEditorWindow
    {
        public override void OnGUI(object a_object, bool a_sceneObject)
        {
            ParticleSystem2DDef def = a_object as ParticleSystem2DDef;
            if (def == null)
            {
                return;
            }

            ParticleEmitterType type = def.EmitterType;
            if (GUI.EnumField("Emitter Type", ref type))
            {
                def.EmitterType = type;
            }

            switch (type)
            {
            case ParticleEmitterType.Point:
            {
                break;
            }
            default:
            {
                break;
            }
            }

            GUI.UIntField("Max Particles", ref def.MaxParticles);

            GUI.BitField("Render Layer", ref def.RenderLayer);

            GUI.Checkbox("Auto Play", ref def.AutoPlay);
            GUI.Checkbox("Burst", ref def.Burst);
        }
    }
}