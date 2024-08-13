// Icarian Editor - Editor for the Icarian Game Engine
// 
// License at end of file.

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
        extern static void PushBoneData(uint a_addr, string a_object, uint a_parent, float[] a_bindPose, float[] a_inverseBindPose);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindSkeletonBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DrawBones(uint a_addr, float[] a_transform);
        
        struct SkeletonData
        {
            public uint BufferAddr;
            public SkeletonAnimator Animator;
        }

        struct BoneData
        {
            public string Object;
            public uint Parent;
            public float[] BindPose;
            public float[] InverseBindPose;
        }

        static List<Skeleton> s_updatedSkeletons = new List<Skeleton>();

        static Dictionary<Skeleton, SkeletonData> s_skeletonData = new Dictionary<Skeleton, SkeletonData>();

        static double s_deltaTime;

        internal static void Destroy()
        {
            foreach (KeyValuePair<Skeleton, SkeletonData> pair in s_skeletonData)
            {
                SkeletonData data = pair.Value;

                data.Animator.Dispose();
            }

            s_skeletonData.Clear();
        }

        internal static void Update(double a_delta)
        {
            s_deltaTime = a_delta;

            s_updatedSkeletons.Clear();
        }

        static void GenerateBones(uint a_buffer, Skeleton a_skeleton, Bone a_bone, Matrix4 a_inverse, ref Dictionary<uint, BoneData> a_boneData)
        {
            Matrix4 bindPose = a_bone.BindingPose;
            Matrix4 inverseBindPose = Matrix4.Inverse(bindPose);

            Matrix4 transform = bindPose * a_inverse;

            BoneData data = new BoneData()
            {
                Object = a_bone.Name,
                Parent = a_bone.Parent,
                BindPose = transform.ToArray(),
                InverseBindPose = inverseBindPose.ToArray()
            };
            a_boneData.Add(a_bone.Index, data);
            // PushBoneData(a_buffer, a_bone.Name, a_bone.Parent, transform.ToArray(), inverseBindPose.ToArray());

            IEnumerable<Bone> children = a_skeleton.GetChildren(a_bone);
            foreach (Bone child in children)
            {
                GenerateBones(a_buffer, a_skeleton, child, inverseBindPose, ref a_boneData);
            }
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

                Dictionary<uint, BoneData> boneData = new Dictionary<uint, BoneData>();
                
                foreach (Bone bone in skeleton.RootBones)
                {
                    GenerateBones(buffer, skeleton, bone, Matrix4.Identity, ref boneData);
                }

                foreach (Bone b in skeleton.Bones)
                {
                    BoneData bDat = boneData[b.Index];
                    PushBoneData(buffer, bDat.Object, bDat.Parent, bDat.BindPose, bDat.InverseBindPose);
                }

                SkeletonAnimator animator = Activator.CreateInstance(type) as SkeletonAnimator;

                // The field does not exist but the property does?
                // But how can m_def not exist?
                // I think reflection has had enough of my shit and C# is screaming for mercy
                PropertyInfo propertyInfo = type.GetProperty("Def");
                propertyInfo.SetValue(animator, a_def);

                // But it lets me get this one?
                FieldInfo fieldInfo = type.GetField("m_buffer", BindingFlags.Instance | BindingFlags.NonPublic);
                fieldInfo.SetValue(animator, buffer);

                animator.Init();

                data.Animator = animator;
                data.BufferAddr = buffer;

                s_skeletonData.Add(skeleton, data);
            }
            
            SkeletonAnimator anim = data.Animator;

            if (anim != null)
            {
                Type type = a_def.ComponentType;

                PropertyInfo propertyInfo = type.GetProperty("Def");
                propertyInfo.SetValue(anim, a_def);

                anim.Update(s_deltaTime);
            }

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
        }

        public static void DrawBones(Matrix4 a_transform, Skeleton a_skeleton)
        {
            if (!s_skeletonData.ContainsKey(a_skeleton))
            {
                return;
            }

            SkeletonData data = s_skeletonData[a_skeleton];

            DrawBones(data.BufferAddr, a_transform.ToArray());
        }
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.