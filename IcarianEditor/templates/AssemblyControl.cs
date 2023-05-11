using IcarianEngine;
using IcarianEngine.Mod;

namespace @ProjectName@
{
    public class @ScriptName@ : AssemblyControl
    {
        public override void Init()
        {
            // Assembly Initialization
            Logger.Message("Hello World from Flare Engine");
        }

        public override void Update()
        {
            // Assembly Update
        }

        public override void Close()
        {
            // Assembly Shutdown
            Logger.Message("Flare Engine says goodbye");
        }
    }
}
