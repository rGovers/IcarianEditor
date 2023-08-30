using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEditor
{
    public static class AnimationMaster
    {
        static List<Skeleton> s_updatedSkeletons = new List<Skeleton>();

        static Dictionary<Skeleton, SkeletonAnimator> s_skeletonAnimators = new Dictionary<Skeleton, SkeletonAnimator>();

        static double s_deltaTime;

        internal static void Update(double a_delta)
        {
            s_deltaTime = a_delta;

            s_updatedSkeletons.Clear();
        }

        public static void UpdateSkeleton(SkeletonAnimatorDef a_def)
        {
            Skeleton skeleton = AssetLibrary.LoadSkeleton(a_def.SkeletonPath);
            if (skeleton == null)
            {
                return;
            }

            if (s_updatedSkeletons.Contains(skeleton))
            {
                return;
            }

            SkeletonAnimator animator = null;
            if (!s_skeletonAnimators.TryGetValue(skeleton, out animator))
            {
                animator = Activator.CreateInstance(a_def.ComponentType) as SkeletonAnimator;

                Type animatorType = animator.GetType();
                // The field does not exist but the property does?
                // But how can m_def not exist?
                // I think reflection has had enough of my shit and C# is screaming for mercy
                PropertyInfo propertyInfo = animatorType.GetProperty("Def");
                propertyInfo.SetValue(animator, a_def);

                animator.Init();

                s_skeletonAnimators.Add(skeleton, animator);
            }

            animator.Update(s_deltaTime);

            s_updatedSkeletons.Add(skeleton);
        }

        public static void DrawSkeleton(Skeleton a_skeleton, Model a_model, Matrix4 a_matrix)
        {
            Gizmos.DrawCapsule(a_matrix[3].XYZ, 1.0f, 0.5f, 8, 0.01f, Color.Red);
        }
    }
}