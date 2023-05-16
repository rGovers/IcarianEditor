using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEditor
{
    public static class ColliderRenderer
    {
        public static void DrawCollider(Matrix4 a_mat, CollisionShapeDef a_def, Color a_color)
        {
            if (a_def == null)
            {
                return;
            }

            switch (a_def)
            {
            case SphereCollisionShapeDef def:
            {
                Gizmos.DrawIcoSphere(a_mat[3].XYZ, def.Radius, 1, 0.01f, a_color);

                break;
            }
            }
        }
    };
}