using IcarianEngine;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace IcarianEditor
{
    public static class AnimationMaster
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateSkeletonBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushBoneData(uint a_addr, string a_object, uint a_parent, Matrix4 a_bindPose);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindSkeletonBuffer(uint a_addr);
        
        struct SkeletonData
        {
            public uint BufferAddr;
            public SkeletonAnimator Animator;
        }

        static List<Skeleton> s_updatedSkeletons = new List<Skeleton>();

        static Dictionary<Skeleton, SkeletonData> s_skeletonData = new Dictionary<Skeleton, SkeletonData>();

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

            SkeletonData data;
            if (!s_skeletonData.TryGetValue(skeleton, out data))
            {
                Type type = a_def.ComponentType;

                uint buffer = GenerateSkeletonBuffer();
                foreach (Bone bone in skeleton.Bones)
                {
                    PushBoneData(buffer, bone.Name, bone.Parent, bone.BindingPose);
                }

                SkeletonAnimator animator = Activator.CreateInstance(type) as SkeletonAnimator;

                // The field does not exist but the property does?
                // But how can m_def not exist?
                // I think reflection has had enough of my shit and C# is screaming for mercy
                PropertyInfo propertyInfo = type.GetProperty("Def");
                propertyInfo.SetValue(animator, a_def);

                animator.Init();

                data.Animator = animator;
                data.BufferAddr = buffer;

                s_skeletonData.Add(skeleton, data);
            }

            data.Animator.Update(s_deltaTime);

            s_updatedSkeletons.Add(skeleton);
        }

        public static void DrawSkeleton(Skeleton a_skeleton, Model a_model, Matrix4 a_matrix)
        {
            if (!s_skeletonData.ContainsKey(a_skeleton))
            {
                return;
            }
            
            SkeletonData data = s_skeletonData[a_skeleton];
            
            BindSkeletonBuffer(data.BufferAddr);

            RenderCommand.DrawModel(a_matrix, a_model);
            // Gizmos.DrawCapsule(a_matrix[3].XYZ, 1.0f, 0.5f, 8, 0.01f, Color.Red);
        }
    }
}