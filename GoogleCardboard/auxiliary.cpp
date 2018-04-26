/*
 # Copyright 2016-2018 Ruben Jesus Garcia Hernandez
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 */

#include <math.h>
#include "auxiliary.h"

gvr::Mat4f TranslationMatrix (float x, float y, float z)
{
    gvr::Mat4f mvs;
    mvs.m[0][0]=1;mvs.m[0][1]=0;mvs.m[0][2]=0; mvs.m[0][3]=x;
    mvs.m[1][0]=0;mvs.m[1][1]=1;mvs.m[1][2]=0;mvs.m[1][3]=y;
    mvs.m[2][0]=0;mvs.m[2][1]=0;mvs.m[2][2]=1; mvs.m[2][3]=z;
    mvs.m[3][0]=0;mvs.m[3][1]=0;mvs.m[3][2]=0; mvs.m[3][3]=1;
    return mvs;
}

gvr::Mat4f TranslationMatrix (float v[3])
{
    return TranslationMatrix (v[0], v[1], v[2]);
}

gvr::Mat4f ScalingMatrix (float x, float y, float z)
{
    gvr::Mat4f mvs;
    
    mvs.m[0][0]=x;mvs.m[0][1]=0;mvs.m[0][2]=0; mvs.m[0][3]=0;
    mvs.m[1][0]=0;mvs.m[1][1]=y;mvs.m[1][2]=0;mvs.m[1][3]=0;
    mvs.m[2][0]=0;mvs.m[2][1]=0;mvs.m[2][2]=z; mvs.m[2][3]=0;
    mvs.m[3][0]=0;mvs.m[3][1]=0;mvs.m[3][2]=0; mvs.m[3][3]=1;
    return mvs;
}

gvr::Mat4f ScalingMatrix (float v[3])
{
    return ScalingMatrix(v[0], v[1], v[2]);
}

gvr::Mat4f ScalingMatrix (float s)
{
    return ScalingMatrix(s,s,s);
}


float getCofactor(float m0, float m1, float m2,
                  float m3, float m4, float m5,
                  float m6, float m7, float m8)
{
    return m0 * (m4 * m8 - m5 * m7) -
    m1 * (m3 * m8 - m5 * m6) +
    m2 * (m3 * m7 - m4 * m6);
}

//adapted from
//openvr-0.9.19\samples\shared\matrices.cpp
gvr::Mat4f invert (const gvr::Mat4f& m)
{
    gvr::Mat4f r;
    // get cofactors of minor matrices
    float cofactor0 = getCofactor(m.m[1][1],m.m[1][2],m.m[1][3], m.m[2][1],m.m[2][2],m.m[2][3], m.m[3][1],m.m[3][2],m.m[3][3]);
    float cofactor1 = getCofactor(m.m[1][0],m.m[1][2],m.m[1][3], m.m[2][0],m.m[2][2],m.m[2][3], m.m[3][0],m.m[3][2],m.m[3][3]);
    float cofactor2 = getCofactor(m.m[1][0],m.m[1][1],m.m[1][3], m.m[2][0],m.m[2][1], m.m[2][3], m.m[3][0],m.m[3][1],m.m[3][3]);
    float cofactor3 = getCofactor(m.m[1][0],m.m[1][1],m.m[1][2], m.m[2][0],m.m[2][1], m.m[2][2], m.m[3][0],m.m[3][1],m.m[3][2]);
    
    // get determinant
    float determinant = m.m[0][0] * cofactor0 - m.m[0][1] * cofactor1 + m.m[0][2] * cofactor2 - m.m[0][3] * cofactor3;
    
    if(fabs(determinant) <= EPSILON)
    {
        for (int i=0;i<4;i++)
            for (int j=0;j<4;j++)
                r.m[i][j]=i==j;
        return r;
    }
    
    // get rest of cofactors for adj(M)
    float cofactor4 = getCofactor(m.m[0][1],m.m[0][2],m.m[0][3], m.m[2][1],m.m[2][2],m.m[2][3], m.m[3][1],m.m[3][2],m.m[3][3]);
    float cofactor5 = getCofactor(m.m[0][0],m.m[0][2],m.m[0][3], m.m[2][0],m.m[2][2],m.m[2][3], m.m[3][0],m.m[3][2],m.m[3][3]);
    float cofactor6 = getCofactor(m.m[0][0],m.m[0][1],m.m[0][3], m.m[2][0],m.m[2][1], m.m[2][3], m.m[3][0],m.m[3][1],m.m[3][3]);
    float cofactor7 = getCofactor(m.m[0][0],m.m[0][1],m.m[0][2], m.m[2][0],m.m[2][1], m.m[2][2], m.m[3][0],m.m[3][1],m.m[3][2]);
    
    float cofactor8 = getCofactor(m.m[0][1],m.m[0][2],m.m[0][3], m.m[1][1],m.m[1][2], m.m[1][3],  m.m[3][1],m.m[3][2],m.m[3][3]);
    float cofactor9 = getCofactor(m.m[0][0],m.m[0][2],m.m[0][3], m.m[1][0],m.m[1][2], m.m[1][3],  m.m[3][0],m.m[3][2],m.m[3][3]);
    float cofactor10= getCofactor(m.m[0][0],m.m[0][1],m.m[0][3], m.m[1][0],m.m[1][1], m.m[1][3],  m.m[3][0],m.m[3][1],m.m[3][3]);
    float cofactor11= getCofactor(m.m[0][0],m.m[0][1],m.m[0][2], m.m[1][0],m.m[1][1], m.m[1][2],  m.m[3][0],m.m[3][1],m.m[3][2]);
    
    float cofactor12= getCofactor(m.m[0][1],m.m[0][2],m.m[0][3], m.m[1][1],m.m[1][2], m.m[1][3],  m.m[2][1], m.m[2][2],m.m[2][3]);
    float cofactor13= getCofactor(m.m[0][0],m.m[0][2],m.m[0][3], m.m[1][0],m.m[1][2], m.m[1][3],  m.m[2][0], m.m[2][2],m.m[2][3]);
    float cofactor14= getCofactor(m.m[0][0],m.m[0][1],m.m[0][3], m.m[1][0],m.m[1][1], m.m[1][3],  m.m[2][0], m.m[2][1], m.m[2][3]);
    float cofactor15= getCofactor(m.m[0][0],m.m[0][1],m.m[0][2], m.m[1][0],m.m[1][1], m.m[1][2],  m.m[2][0], m.m[2][1], m.m[2][2]);
    
    // build inverse matrix = adj(M) / det(M)
    // adjugate of M is the transpose of the cofactor matrix of M
    float invDeterminant = 1.0f / determinant;
    r.m[0][0] =  invDeterminant * cofactor0;
    r.m[0][1] = -invDeterminant * cofactor4;
    r.m[0][2] =  invDeterminant * cofactor8;
    r.m[0][3] = -invDeterminant * cofactor12;
    
    r.m[1][0] = -invDeterminant * cofactor1;
    r.m[1][1] =  invDeterminant * cofactor5;
    r.m[1][2] = -invDeterminant * cofactor9;
    r.m[1][3] =  invDeterminant * cofactor13;
    
    r.m[2][0] =  invDeterminant * cofactor2;
    r.m[2][1] = -invDeterminant * cofactor6;
    r.m[2][2]=  invDeterminant * cofactor10;
    r.m[2][3]= -invDeterminant * cofactor14;
    
    r.m[3][0]= -invDeterminant * cofactor3;
    r.m[3][1]=  invDeterminant * cofactor7;
    r.m[3][2]= -invDeterminant * cofactor11;
    r.m[3][3]=  invDeterminant * cofactor15;
    return r;
}
