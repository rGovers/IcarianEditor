using IcarianEngine.Physics;

namespace @ProjectName@
{
    // RigidBody component that can be attached to a GameObject
    public class @ScriptName@ : RigidBody
    {
        public override void Init()
        {
            // RigidBody Initialization
        }

        public override void OnCollisionEnter(PhysicsBody a_other, CollisionData a_data)
        {
            // RigidBody OnCollisionEnter
        }
        public override void OnCollisionStay(PhysicsBody a_other, CollisionData a_data)
        {
            // RigidBody OnCollisionStay
        }
        public override void OnCollisionExit(PhysicsBody a_other)
        {
            // RigidBody OnCollisionExit
        }
    }
}
