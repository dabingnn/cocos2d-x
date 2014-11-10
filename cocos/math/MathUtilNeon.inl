/**
 Copyright 2013 BlackBerry Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 Original file from GamePlay3D: http://gameplay3d.org

 This file was modified to fit the cocos2d-x project
 */

NS_CC_MATH_BEGIN

#if defined(__aarch64__)
//64bit arm arch
inline void MathUtil::addMatrix(const float* m, float scalar, float* dst)
{
    asm volatile(
	    "ld4  {v0.4s, v1.4s, v2.4s, v3.4s}, [%1]    	\n\t" // M[m0-m7] M[m8-m15]
	    "ld1r {v4.4s}, [%2]				                \n\t" //ssss

	    "fadd v8.4s, v0.4s, v4.4s			\n\t" // DST->M[m0-m3] = M[m0-m3] + s
	    "fadd v9.4s, v1.4s, v4.4s			\n\t" // DST->M[m4-m7] = M[m4-m7] + s
	    "fadd v10.4s, v2.4s, v4.4s			\n\t" // DST->M[m8-m11] = M[m8-m11] + s
	    "fadd v11.4s, v3.4s, v4.4s			\n\t" // DST->M[m12-m15] = M[m12-m15] + s
	
        "st4 {v8.4s, v9.4s, v10.4s, v11.4s}, [%0] 	\n\t"    // Result in V9
	    :
        : "r"(dst), "r"(m), "r"(&scalar)
        : "v0", "v1", "v2", "v3", "v4", "v8", "v9", "v10", "v11", "memory"
    );
}

inline void MathUtil::addMatrix(const float* m1, const float* m2, float* dst)
{
    asm volatile(
        "ld4     {v0.4s, v1.4s, v2.4s, v3.4s},     [%1] 	\n\t" // M1[m0-m7] M1[m8-m15]
        "ld4     {v8.4s, v9.4s, v10.4s, v11.4s},   [%2] 	\n\t" // M2[m0-m7] M2[m8-m15]

        "fadd   v12.4s, v0.4s, v8.4s          \n\t" // DST->M[m0-m3] = M1[m0-m3] + M2[m0-m3]
        "fadd   v13.4s, v1.4s, v9.4s          \n\t" // DST->M[m4-m7] = M1[m4-m7] + M2[m4-m7]
        "fadd   v14.4s, v2.4s, v10.4s         \n\t" // DST->M[m8-m11] = M1[m8-m11] + M2[m8-m11]
        "fadd   v15.4s, v3.4s, v11.4s         \n\t" // DST->M[m12-m15] = M1[m12-m15] + M2[m12-m15]

        "st4    {v12.4s, v13.4s, v14.4s, v15.4s}, [%0]    \n\t" // DST->M[m0-m7] DST->M[m8-m15] 
        :
        : "r"(dst), "r"(m1), "r"(m2)
        : "v0", "v1", "v2", "v3", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "memory"
    );
}

//############
inline void MathUtil::subtractMatrix(const float* m1, const float* m2, float* dst)
{
    asm volatile(
        "ld4     {v0.4s, v1.4s, v2.4s, v3.4s},     [%1]  \n\t" // M1[m0-m7] M1[m8-m15]
        "ld4     {v8.4s, v9.4s, v10.4s, v11.4s},   [%2]  \n\t" // M2[m0-m7] M2[m8-m15] 

        "fsub   v12.4s, v0.4s, v8.4s         \n\t" // DST->M[m0-m3] = M1[m0-m3] - M2[m0-m3]
        "fsub   v13.4s, v1.4s, v9.4s         \n\t" // DST->M[m4-m7] = M1[m4-m7] - M2[m4-m7]
        "fsub   v14.4s, v2.4s, v10.4s        \n\t" // DST->M[m8-m11] = M1[m8-m11] - M2[m8-m11]
        "fsub   v15.4s, v3.4s, v11.4s        \n\t" // DST->M[m12-m15] = M1[m12-m15] - M2[m12-m15]

        "st4    {v12.4s, v13.4s, v14.4s, v15.4s}, [%0]   \n\t" // DST->M[m0-m7] DST->M[m8-m15]
        :
        : "r"(dst), "r"(m1), "r"(m2)
        : "v0", "v1", "v2", "v3", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "memory"
    );
}

//###############
inline void MathUtil::multiplyMatrix(const float* m, float scalar, float* dst)
{
    asm volatile(
        "ld1     {v0.s}[0],         [%2]            \n\t" //s  
        "ld4     {v4.4s, v5.4s, v6.4s, v7.4s}, [%1]       \n\t" //M[m0-m7] M[m8-m15]

        "fmul     v8.4s, v4.4s, v0.s[0]               \n\t" // DST->M[m0-m3] = M[m0-m3] * s
        "fmul     v9.4s, v5.4s, v0.s[0]               \n\t" // DST->M[m4-m7] = M[m4-m7] * s
        "fmul     v10.4s, v6.4s, v0.s[0]              \n\t" // DST->M[m8-m11] = M[m8-m11] * s
        "fmul     v11.4s, v7.4s, v0.s[0]              \n\t" // DST->M[m12-m15] = M[m12-m15] * s

        "st4     {v8.4s, v9.4s, v10.4s, v11.4s},           [%0]     \n\t" // DST->M[m0-m7] DST->M[m8-m15]
        :
        : "r"(dst), "r"(m), "r"(&scalar)
        : "v0", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "memory"
    );
}

//#########
inline void MathUtil::multiplyMatrix(const float* m1, const float* m2, float* dst)
{
    asm volatile(
        "ld1     {v8.4s, v9.4s, v10.4s, v11.4s}, [%1] \n\t"       // M1[m0-m7] M1[m8-m15] M2[m0-m7]  M2[m8-m15]
        "ld4     {v0.4s, v1.4s, v2.4s, v3.4s},  [%2]   \n\t"       // M2[m0-m15]


        "fmul    v12.4s, v8.4s, v0.s[0]     \n\t"         // DST->M[m0-m3] = M1[m0-m3] * M2[m0]
        "fmul    v13.4s, v8.4s, v0.s[1]     \n\t"         // DST->M[m4-m7] = M1[m4-m7] * M2[m4]
        "fmul    v14.4s, v8.4s, v0.s[2]     \n\t"         // DST->M[m8-m11] = M1[m8-m11] * M2[m8]
        "fmul    v15.4s, v8.4s, v0.s[3]     \n\t"         // DST->M[m12-m15] = M1[m12-m15] * M2[m12]

        "fmla    v12.4s, v9.4s, v1.s[0]     \n\t"         // DST->M[m0-m3] += M1[m0-m3] * M2[m1]
        "fmla    v13.4s, v9.4s, v1.s[1]     \n\t"         // DST->M[m4-m7] += M1[m4-m7] * M2[m5]
        "fmla    v14.4s, v9.4s, v1.s[2]     \n\t"         // DST->M[m8-m11] += M1[m8-m11] * M2[m9]
        "fmla    v15.4s, v9.4s, v1.s[3]     \n\t"         // DST->M[m12-m15] += M1[m12-m15] * M2[m13]

        "fmla    v12.4s, v10.4s, v2.s[0]    \n\t"         // DST->M[m0-m3] += M1[m0-m3] * M2[m2]
        "fmla    v13.4s, v10.4s, v2.s[1]    \n\t"         // DST->M[m4-m7] += M1[m4-m7] * M2[m6]
        "fmla    v14.4s, v10.4s, v2.s[2]    \n\t"         // DST->M[m8-m11] += M1[m8-m11] * M2[m10]
        "fmla    v15.4s, v10.4s, v2.s[3]    \n\t"         // DST->M[m12-m15] += M1[m12-m15] * M2[m14]

        "fmla    v12.4s, v11.4s, v3.s[0]    \n\t"         // DST->M[m0-m3] += M1[m0-m3] * M2[m3]
        "fmla    v13.4s, v11.4s, v3.s[1]    \n\t"         // DST->M[m4-m7] += M1[m4-m7] * M2[m7]
        "fmla    v14.4s, v11.4s, v3.s[2]    \n\t"         // DST->M[m8-m11] += M1[m8-m11] * M2[m11]
        "fmla    v15.4s, v11.4s, v3.s[3]    \n\t"         // DST->M[m12-m15] += M1[m12-m15] * M2[m15]

        "st1    {v12.4s, v13.4s, v14.4s, v15.4s}, [%0]  \n\t"       // DST->M[m0-m7]// DST->M[m8-m15]

        : // output
        : "r"(dst), "r"(m1), "r"(m2) // input - note *value* of pointer doesn't change.
        : "memory", "v0", "v1", "v2", "v3", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15"
     );
}

///###############################
inline void MathUtil::negateMatrix(const float* m, float* dst)
{
    asm volatile(
        "ld4     {v0.4s, v1.4s, v2.4s, v3.4s},  [%1]     \n\t" // load m0-m7 load m8-m15

        "fneg     v4.4s, v0.4s             \n\t" // negate m0-m3
        "fneg     v5.4s, v1.4s             \n\t" // negate m4-m7
        "fneg     v6.4s, v2.4s             \n\t" // negate m8-m15
        "fneg     v7.4s, v3.4s             \n\t" // negate m8-m15

        "st4     {v4.4s, v5.4s, v6.4s, v7.4s},  [%0]     \n\t" // store m0-m7 store m8-m15
        :
        : "r"(dst), "r"(m)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory"
    );
}

//#########
inline void MathUtil::transposeMatrix(const float* m, float* dst)
{
    asm volatile(
        "ld4 {v0.4s, v1.4s, v2.4s, v3.4s}, [%1]    \n\t" // DST->M[m0, m4, m8, m12] = M[m0-m3] 
							 //DST->M[m1, m5, m9, m12] = M[m4-m7] 
        "st1 {v0.4s, v1.4s, v2.4s, v3.4s}, [%0]    \n\t"
        :
        : "r"(dst), "r"(m)
        : "v0", "v1", "v2", "v3", "memory"
    );
}

//#########
inline void MathUtil::transformVec4(const float* m, float x, float y, float z, float w, float* dst)
{
    asm volatile(
        "ld1    {v0.s}[0],        [%1]    \n\t"    // V[x]
        "ld1    {v0.s}[1],        [%2]    \n\t"    // V[y]
        "ld1    {v0.s}[2],        [%3]    \n\t"    // V[z]
        "ld1    {v0.s}[3],        [%4]    \n\t"    // V[w]
        "ld1    {v9.4s, v10.4s, v11.4s, v12.4s}, [%5]   \n\t"    // M[m0-m7] M[m8-m15]

	
        "fmul v13.4s, v9.4s, v0.s[0]           \n\t"      // DST->V = M[m0-m3] * V[x]
        "fmla v13.4s, v10.4s, v0.s[1]           \n\t"    // DST->V += M[m4-m7] * V[y]
        "fmla v13.4s, v11.4s, v0.s[2]           \n\t"    // DST->V += M[m8-m11] * V[z]
        "fmla v13.4s, v12.4s, v0.s[3]           \n\t"    // DST->V += M[m12-m15] * V[w]

        //"st1 {v13.4s}, [%0]               \n\t"    // DST->V[x, y] // DST->V[z]
        "st1 {v13.2s}, [%0], 8               \n\t"
        "st1 {v13.s}[2], [%0]                \n\t"
        :
        : "r"(dst), "r"(&x), "r"(&y), "r"(&z), "r"(&w), "r"(m)
        : "v0", "v9", "v10","v11", "v12", "v13", "memory"
    );
}

//##########
inline void MathUtil::transformVec4(const float* m, const float* v, float* dst)
{
    asm volatile
    (
        "ld1    {v0.4s}, [%1]     \n\t"   // V[x, y, z, w]
        "ld1    {v9.4s, v10.4s, v11.4s, v12.4s}, [%2] \n\t"   // M[m0-m7] M[m8-m15]

        "fmul   v13.4s, v9.4s, v0.s[0]     \n\t"   // DST->V = M[m0-m3] * V[x]
        "fmla   v13.4s, v10.4s, v0.s[1]    \n\t"   // DST->V = M[m4-m7] * V[y]
        "fmla   v13.4s, v11.4s, v0.s[2]    \n\t"   // DST->V = M[m8-m11] * V[z]
        "fmla   v13.4s, v12.4s, v0.s[3]    \n\t"   // DST->V = M[m12-m15] * V[w]

        "st1    {v13.4s}, [%0]  	 \n\t"   // DST->V
        :
        : "r"(dst), "r"(v), "r"(m)
        : "v0", "v9", "v10","v11", "v12", "v13", "memory"
    );
}

//##########
inline void MathUtil::crossVec3(const float* v1, const float* v2, float* dst)
{
    asm volatile(
        "ld1 {v0.2s},    [%2]         \n\t" //
        "ld1 {v0.s}[3],  [%1]         \n\t" //
        "mov v0.s[2], v0.s[1]         \n\t" // q0 = (v1y, v1z, v1z, v1x)

        "ld1 {v1.s}[1],  [%3]           \n\t" //
        "ld1 {v1.s}[2], [%4], 4         \n\t" //
        "ld1 {v1.s}[3], [%4]            \n\t" //
        "mov v1.s[0], v1.s[3]           \n\t" // q1 = (v2z, v2x, v2y, v2z)

        "fmul v2.4s, v0.4s, v1.4s            \n\t" // x = v1y * v2z, y = v1z * v2x
        "fsub s8, s8, s10            	     \n\t"
        "fsub s9, s9, s11                    \n\t"  // x -= v1z * v2y, y-= v1x - v2z

        "fmul s10, s3, s6         \n\t" // z = v1x * v2y
        "fmul s11, s0, s5         \n\t" // z-= v1y * vx
        "fsub s10, s10, s11	      \n\t"

        "st1 {v2.2s},       [%0], 8      \n\t" // V[x, y]
        "st1 {v2.s}[2],     [%0]         \n\t" // V[z]
        :
        : "r"(dst), "r"(v1), "r"((v1+1)), "r"(v2), "r"((v2+1))
        : "v0", "v1", "v2", "memory"
    );
}

#else
//32bit arm arch
inline void MathUtil::addMatrix(const float* m, float scalar, float* dst)
{
    asm volatile(
        "vld1.32 {q0, q1}, [%1]!    \n\t" // M[m0-m7]
        "vld1.32 {q2, q3}, [%1]     \n\t" // M[m8-m15]
        "vld1.32 {d8[0]},  [%2]     \n\t" // s
        "vmov.f32 s17, s16          \n\t" // s
        "vmov.f32 s18, s16          \n\t" // s
        "vmov.f32 s19, s16          \n\t" // s

        "vadd.f32 q8, q0, q4        \n\t" // DST->M[m0-m3] = M[m0-m3] + s
        "vadd.f32 q9, q1, q4        \n\t" // DST->M[m4-m7] = M[m4-m7] + s
        "vadd.f32 q10, q2, q4       \n\t" // DST->M[m8-m11] = M[m8-m11] + s
        "vadd.f32 q11, q3, q4       \n\t" // DST->M[m12-m15] = M[m12-m15] + s

        "vst1.32 {q8, q9}, [%0]!    \n\t" // DST->M[m0-m7]
        "vst1.32 {q10, q11}, [%0]   \n\t" // DST->M[m8-m15]
        :
        : "r"(dst), "r"(m), "r"(&scalar)
        : "q0", "q1", "q2", "q3", "q4", "q8", "q9", "q10", "q11", "memory"
    );
}

inline void MathUtil::addMatrix(const float* m1, const float* m2, float* dst)
{
    asm volatile(
        "vld1.32     {q0, q1},     [%1]! \n\t" // M1[m0-m7]
        "vld1.32     {q2, q3},     [%1]  \n\t" // M1[m8-m15]
        "vld1.32     {q8, q9},     [%2]! \n\t" // M2[m0-m7]
        "vld1.32     {q10, q11}, [%2]    \n\t" // M2[m8-m15]

        "vadd.f32   q12, q0, q8          \n\t" // DST->M[m0-m3] = M1[m0-m3] + M2[m0-m3]
        "vadd.f32   q13, q1, q9          \n\t" // DST->M[m4-m7] = M1[m4-m7] + M2[m4-m7]
        "vadd.f32   q14, q2, q10         \n\t" // DST->M[m8-m11] = M1[m8-m11] + M2[m8-m11]
        "vadd.f32   q15, q3, q11         \n\t" // DST->M[m12-m15] = M1[m12-m15] + M2[m12-m15]

        "vst1.32    {q12, q13}, [%0]!    \n\t" // DST->M[m0-m7]
        "vst1.32    {q14, q15}, [%0]     \n\t" // DST->M[m8-m15]
        :
        : "r"(dst), "r"(m1), "r"(m2)
        : "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "memory"
    );
}

inline void MathUtil::subtractMatrix(const float* m1, const float* m2, float* dst)
{
    asm volatile(
        "vld1.32     {q0, q1},     [%1]!  \n\t" // M1[m0-m7]
        "vld1.32     {q2, q3},     [%1]   \n\t" // M1[m8-m15]
        "vld1.32     {q8, q9},     [%2]!  \n\t" // M2[m0-m7]
        "vld1.32     {q10, q11}, [%2]     \n\t" // M2[m8-m15]

        "vsub.f32   q12, q0, q8         \n\t" // DST->M[m0-m3] = M1[m0-m3] - M2[m0-m3]
        "vsub.f32   q13, q1, q9         \n\t" // DST->M[m4-m7] = M1[m4-m7] - M2[m4-m7]
        "vsub.f32   q14, q2, q10        \n\t" // DST->M[m8-m11] = M1[m8-m11] - M2[m8-m11]
        "vsub.f32   q15, q3, q11        \n\t" // DST->M[m12-m15] = M1[m12-m15] - M2[m12-m15]

        "vst1.32    {q12, q13}, [%0]!   \n\t" // DST->M[m0-m7]
        "vst1.32    {q14, q15}, [%0]    \n\t" // DST->M[m8-m15]
        :
        : "r"(dst), "r"(m1), "r"(m2)
        : "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "memory"
    );
}

inline void MathUtil::multiplyMatrix(const float* m, float scalar, float* dst)
{
    asm volatile(
        "vld1.32     {d0[0]},         [%2]        \n\t" // M[m0-m7]
        "vld1.32    {q4-q5},          [%1]!       \n\t" // M[m8-m15]
        "vld1.32    {q6-q7},          [%1]        \n\t" // s

        "vmul.f32     q8, q4, d0[0]               \n\t" // DST->M[m0-m3] = M[m0-m3] * s
        "vmul.f32     q9, q5, d0[0]               \n\t" // DST->M[m4-m7] = M[m4-m7] * s
        "vmul.f32     q10, q6, d0[0]              \n\t" // DST->M[m8-m11] = M[m8-m11] * s
        "vmul.f32     q11, q7, d0[0]              \n\t" // DST->M[m12-m15] = M[m12-m15] * s

        "vst1.32     {q8-q9},           [%0]!     \n\t" // DST->M[m0-m7]
        "vst1.32     {q10-q11},         [%0]      \n\t" // DST->M[m8-m15]
        :
        : "r"(dst), "r"(m), "r"(&scalar)
        : "q0", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "memory"
    );
}

inline void MathUtil::multiplyMatrix(const float* m1, const float* m2, float* dst)
{
    asm volatile(
        "vld1.32     {d16 - d19}, [%1]! \n\t"       // M1[m0-m7]
        "vld1.32     {d20 - d23}, [%1]  \n\t"       // M1[m8-m15]
        "vld1.32     {d0 - d3}, [%2]!   \n\t"       // M2[m0-m7]
        "vld1.32     {d4 - d7}, [%2]    \n\t"       // M2[m8-m15]

        "vmul.f32    q12, q8, d0[0]     \n\t"         // DST->M[m0-m3] = M1[m0-m3] * M2[m0]
        "vmul.f32    q13, q8, d2[0]     \n\t"         // DST->M[m4-m7] = M1[m4-m7] * M2[m4]
        "vmul.f32    q14, q8, d4[0]     \n\t"         // DST->M[m8-m11] = M1[m8-m11] * M2[m8]
        "vmul.f32    q15, q8, d6[0]     \n\t"         // DST->M[m12-m15] = M1[m12-m15] * M2[m12]

        "vmla.f32    q12, q9, d0[1]     \n\t"         // DST->M[m0-m3] += M1[m0-m3] * M2[m1]
        "vmla.f32    q13, q9, d2[1]     \n\t"         // DST->M[m4-m7] += M1[m4-m7] * M2[m5]
        "vmla.f32    q14, q9, d4[1]     \n\t"         // DST->M[m8-m11] += M1[m8-m11] * M2[m9]
        "vmla.f32    q15, q9, d6[1]     \n\t"         // DST->M[m12-m15] += M1[m12-m15] * M2[m13]

        "vmla.f32    q12, q10, d1[0]    \n\t"         // DST->M[m0-m3] += M1[m0-m3] * M2[m2]
        "vmla.f32    q13, q10, d3[0]    \n\t"         // DST->M[m4-m7] += M1[m4-m7] * M2[m6]
        "vmla.f32    q14, q10, d5[0]    \n\t"         // DST->M[m8-m11] += M1[m8-m11] * M2[m10]
        "vmla.f32    q15, q10, d7[0]    \n\t"         // DST->M[m12-m15] += M1[m12-m15] * M2[m14]

        "vmla.f32    q12, q11, d1[1]    \n\t"         // DST->M[m0-m3] += M1[m0-m3] * M2[m3]
        "vmla.f32    q13, q11, d3[1]    \n\t"         // DST->M[m4-m7] += M1[m4-m7] * M2[m7]
        "vmla.f32    q14, q11, d5[1]    \n\t"         // DST->M[m8-m11] += M1[m8-m11] * M2[m11]
        "vmla.f32    q15, q11, d7[1]    \n\t"         // DST->M[m12-m15] += M1[m12-m15] * M2[m15]

        "vst1.32    {d24 - d27}, [%0]!  \n\t"       // DST->M[m0-m7]
        "vst1.32    {d28 - d31}, [%0]   \n\t"       // DST->M[m8-m15]

        : // output
        : "r"(dst), "r"(m1), "r"(m2) // input - note *value* of pointer doesn't change.
        : "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
    );
}

inline void MathUtil::negateMatrix(const float* m, float* dst)
{
    asm volatile(
        "vld1.32     {q0-q1},  [%1]!     \n\t" // load m0-m7
        "vld1.32     {q2-q3},  [%1]      \n\t" // load m8-m15

        "vneg.f32     q4, q0             \n\t" // negate m0-m3
        "vneg.f32     q5, q1             \n\t" // negate m4-m7
        "vneg.f32     q6, q2             \n\t" // negate m8-m15
        "vneg.f32     q7, q3             \n\t" // negate m8-m15

        "vst1.32     {q4-q5},  [%0]!     \n\t" // store m0-m7
        "vst1.32     {q6-q7},  [%0]      \n\t" // store m8-m15
        :
        : "r"(dst), "r"(m)
        : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "memory"
    );
}

inline void MathUtil::transposeMatrix(const float* m, float* dst)
{
    asm volatile(
        "vld4.32 {d0[0], d2[0], d4[0], d6[0]}, [%1]!    \n\t" // DST->M[m0, m4, m8, m12] = M[m0-m3]
        "vld4.32 {d0[1], d2[1], d4[1], d6[1]}, [%1]!    \n\t" // DST->M[m1, m5, m9, m12] = M[m4-m7]
        "vld4.32 {d1[0], d3[0], d5[0], d7[0]}, [%1]!    \n\t" // DST->M[m2, m6, m10, m12] = M[m8-m11]
        "vld4.32 {d1[1], d3[1], d5[1], d7[1]}, [%1]     \n\t" // DST->M[m3, m7, m11, m12] = M[m12-m15]

        "vst1.32 {q0-q1}, [%0]!                         \n\t" // DST->M[m0-m7]
        "vst1.32 {q2-q3}, [%0]                          \n\t" // DST->M[m8-m15]
        :
        : "r"(dst), "r"(m)
        : "q0", "q1", "q2", "q3", "memory"
    );
}

inline void MathUtil::transformVec4(const float* m, float x, float y, float z, float w, float* dst)
{
    asm volatile(
        "vld1.32    {d0[0]},        [%1]    \n\t"    // V[x]
        "vld1.32    {d0[1]},        [%2]    \n\t"    // V[y]
        "vld1.32    {d1[0]},        [%3]    \n\t"    // V[z]
        "vld1.32    {d1[1]},        [%4]    \n\t"    // V[w]
        "vld1.32    {d18 - d21},    [%5]!   \n\t"    // M[m0-m7]
        "vld1.32    {d22 - d25},    [%5]    \n\t"    // M[m8-m15]

        "vmul.f32 q13,  q9, d0[0]           \n\t"    // DST->V = M[m0-m3] * V[x]
        "vmla.f32 q13, q10, d0[1]           \n\t"    // DST->V += M[m4-m7] * V[y]
        "vmla.f32 q13, q11, d1[0]           \n\t"    // DST->V += M[m8-m11] * V[z]
        "vmla.f32 q13, q12, d1[1]           \n\t"    // DST->V += M[m12-m15] * V[w]

        "vst1.32 {d26}, [%0]!               \n\t"    // DST->V[x, y]
        "vst1.32 {d27[0]}, [%0]             \n\t"    // DST->V[z]
        :
        : "r"(dst), "r"(&x), "r"(&y), "r"(&z), "r"(&w), "r"(m)
        : "q0", "q9", "q10","q11", "q12", "q13", "memory"
    );
}

inline void MathUtil::transformVec4(const float* m, const float* v, float* dst)
{
    asm volatile
    (
        "vld1.32    {d0, d1}, [%1]     \n\t"   // V[x, y, z, w]
        "vld1.32    {d18 - d21}, [%2]! \n\t"   // M[m0-m7]
        "vld1.32    {d22 - d25}, [%2]  \n\t"    // M[m8-m15]

        "vmul.f32   q13, q9, d0[0]     \n\t"   // DST->V = M[m0-m3] * V[x]
        "vmla.f32   q13, q10, d0[1]    \n\t"   // DST->V = M[m4-m7] * V[y]
        "vmla.f32   q13, q11, d1[0]    \n\t"   // DST->V = M[m8-m11] * V[z]
        "vmla.f32   q13, q12, d1[1]    \n\t"   // DST->V = M[m12-m15] * V[w]

        "vst1.32    {d26, d27}, [%0]   \n\t"   // DST->V
        :
        : "r"(dst), "r"(v), "r"(m)
        : "q0", "q9", "q10","q11", "q12", "q13", "memory"
    );
}

inline void MathUtil::crossVec3(const float* v1, const float* v2, float* dst)
{
    asm volatile(
        "vld1.32 {d1[1]},  [%1]         \n\t" //
        "vld1.32 {d0},     [%2]         \n\t" //
        "vmov.f32 s2, s1                \n\t" // q0 = (v1y, v1z, v1z, v1x)

        "vld1.32 {d2[1]},  [%3]         \n\t" //
        "vld1.32 {d3},     [%4]         \n\t" //
        "vmov.f32 s4, s7                  \n\t" // q1 = (v2z, v2x, v2y, v2z)

        "vmul.f32 d4, d0, d2            \n\t" // x = v1y * v2z, y = v1z * v2x
        "vmls.f32 d4, d1, d3            \n\t" // x -= v1z * v2y, y-= v1x - v2z

        "vmul.f32 d5, d3, d1[1]         \n\t" // z = v1x * v2y
        "vmls.f32 d5, d0, d2[1]         \n\t" // z-= v1y * vx

        "vst1.32 {d4},       [%0]!      \n\t" // V[x, y]
        "vst1.32 {d5[0]}, [%0]          \n\t" // V[z]
        :
        : "r"(dst), "r"(v1), "r"((v1+1)), "r"(v2), "r"((v2+1))
        : "q0", "q1", "q2", "memory"
    );
}
#endif

NS_CC_MATH_END
