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
            case BoxCollisionShapeDef def:
            {
                Vector3 halfExtents = def.Extents * 0.5f;

                Vector3 pos;
                Vector3 right;
                Vector3 up;
                Vector3 forward;
                Vector3 scale;
                Matrix4.Decompose(a_mat, out pos, out right, out up, out forward, out scale);                

                right *= halfExtents.X;
                up *= halfExtents.Y;
                forward *= halfExtents.Z;

                Gizmos.DrawLine(pos + forward + up + right, pos + forward + up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward + up + right, pos - forward + up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward + up + right, pos - forward + up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward + up - right, pos - forward + up - right, 0.01f, a_color);

                Gizmos.DrawLine(pos + forward - up + right, pos + forward - up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward - up + right, pos - forward - up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward - up + right, pos - forward - up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward - up - right, pos - forward - up - right, 0.01f, a_color);

                Gizmos.DrawLine(pos + forward + up + right, pos + forward - up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos + forward + up - right, pos + forward - up - right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward + up + right, pos - forward - up + right, 0.01f, a_color);
                Gizmos.DrawLine(pos - forward + up - right, pos - forward - up - right, 0.01f, a_color);

                break;
            }
            case CapsuleCollisionShapeDef def:
            {
                Gizmos.DrawCapsule(a_mat[3].XYZ, def.Height, def.Radius, 8, 0.01f, a_color);

                break;
            }
            case CylinderCollisionShapeDef def:
            {
                Gizmos.DrawCylinder(a_mat[3].XYZ, def.Height, def.Radius, 8, 0.01f, a_color);

                break;
            }
            }
        }
    };
}