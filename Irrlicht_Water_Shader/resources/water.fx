//
// FX Effect file - Water
//

// Constants
float4 lhtPos < string UIPosition = "Light Position"; >;
vector vEyePos : EyePosition;
matrix mTot : WorldViewProjection;
matrix mView : View;
float ticks : Time;
float sinticks : SinTime;

// Model and Texture Names
string XFile = "WaterSurface.x";
texture tBump < string name = "WaterBump.tga"; >;
texture tEnv < string name = "WaterEnvMap.tga"; >;
texture tGradient < string name = "WaterGradient.tga"; >;

technique tec0
{ 
    pass p0
    {
        VertexShaderConstant[0] = { 0, 0.5, 1, 2 };
        VertexShaderConstant[1] = { 4, 1.57, 3.14, 6.28 };
        VertexShaderConstant[2] = { 1, -0.167, 0.00833, -0.0002 };
        VertexShaderConstant[3] = { 0.5, -0.0417, 0.00139, -0.00002 };
        VertexShaderConstant[4] = <mTot>;
        VertexShaderConstant[8] = <vEyePos>;
        VertexShaderConstant[9] = <lhtPos>;
        VertexShaderConstant[10] = { 0.9, 0.003, 0, 0 };
        VertexShaderConstant[11] = { 4, 3, 4, 4 };
        VertexShaderConstant[12] = { 0, 0.2, 0, 0 };
        VertexShaderConstant[13] = { 0.45, 0.1, 0.221, 0.04 };
        VertexShaderConstant[14] = { 6.22, -2.2, 0.55, 2.12 };
        VertexShaderConstant[15] = { 4, -2.33, 2.5, 0 };
        VertexShaderConstant[16] = <ticks>;
        VertexShaderConstant[17] = <sinticks>;
        VertexShaderConstant[18] = { -0.012, 0.025, -0.011, 0.046 };
        VertexShaderConstant[19] = { -0.05, 0.055, 0, 1 };
        VertexShaderConstant[20] = <mView>;

        PixelShaderConstant[0] = { 0.5, 0.5, 0.5, 1.0 };
        PixelShaderConstant[1] = { 0.129, 0.314, 0.4, 1 };

        Texture[0]   = <tBump>;
        Texture[1]   = <tBump>;
        Texture[2]   = <tEnv>;
        Texture[3]   = <tGradient>;

	ColorOp[0] = Modulate;
	ColorArg1[0] = Texture;
	ColorArg2[0] = Current;
	AlphaOp[0] = SelectArg1;
	AlphaArg1[0] = Texture;
	AlphaArg2[0] = Current;

	TextureTransformFlags[0] = DISABLE;

        Magfilter[0] = linear;
        Minfilter[0] = linear;
        Mipfilter[0] = linear;
        Magfilter[1] = linear;
        Minfilter[1] = linear;
        Mipfilter[1] = linear;
        Magfilter[2] = linear;
        Minfilter[2] = linear;
        Mipfilter[2] = linear;
        Magfilter[3] = linear;
        Minfilter[3] = linear;
        Mipfilter[3] = linear;

        AddressU[0] = wrap;
        AddressV[0] = wrap;
        AddressW[0] = wrap;
        AddressU[1] = wrap;
        AddressV[1] = wrap;
        AddressW[1] = wrap;
        AddressU[2] = clamp;
        AddressV[2] = clamp;
        AddressW[2] = clamp;
        AddressU[3] = clamp;
        AddressV[3] = clamp;
        AddressW[3] = clamp;


        VertexShader = 
        asm
        {
vs.2.0

dcl_position   v0
dcl_normal     v1
dcl_texcoord   v2
dcl_tangent    v3
dcl_binormal   v4

//============================================//
// use tex coords as input to sinusoidal warp //
//============================================//
mul  r0, c14, v2.x
mad  r0, c15, v2.y, r0


mov  r1, c16.x        // time...
mad  r0, r1, c13, r0  // add scaled time to move bumps according to freq
add  r0, r0, c12      // starting time offset
frc  r0.xy, r0        // take frac of all 4 components
frc  r1.xy, r0.zwzw   //
mov  r0.zw, r1.xyxy   //

mul  r0, r0, c10.x    // mul by fixup (due to inaccuracy)
sub  r0, r0, c0.y     // subtract 0.5
mul  r0, r0, c1.w     // mul tex coords by 2pi  (coords range from -pi to pi)

mul  r5, r0, r0       // (wave vec)^2
mul  r1, r5, r0       // (wave vec)^3
mul  r6, r1, r0       // (wave vec)^4
mul  r2, r6, r0       // (wave vec)^5
mul  r7, r2, r0       // (wave vec)^6
mul  r3, r7, r0       // (wave vec)^7
mul  r8, r3, r0       // (wave vec)^8

mad  r4, r1, c2.y, r0 // (wave vec) ((wave vec)^3)/3!
mad  r4, r2, c2.z, r4 // + ((wave vec)^5)/5!
mad  r4, r3, c2.w, r4 // ((wave vec)^7/7!

mov  r0, c0.z         // 1
mad  r5, r5, c3.x, r0 // -(wave vec)^2/2!
mad  r5, r6, c3.y, r5 // +(wave vec)^4/4!
mad  r5, r7, c3.z, r5 // -(wave vec)^6/6!
mad  r5, r8, c3.w, r5 // +(wave vec)^8/8!

sub  r0, c0.z, c0.x   //... 1-wave scale
mul  r4, r4, r0       // scale sin
mul  r5, r5, r0       // scale cos

dp4  r0, r4, c11      // mul by wave heights

mul r0.xyz, v1, r0    // mul wave mag at this vertex by normal
add r0.xyz, r0, v0    // add to position
mov r0.w, c0.z        // homogenous component

m4x4 oPos, r0, c4     // Outpos = ObjSpace * World-view-proj matrix

mul  r1, r5, c11      // cos * wave height
dp4  r9.x, -r1, c14   // normal x offset
dp4  r9.yzw, -r1, c15 // normal y offset and tangent offset
mov  r5, v1           // starting normal
mad  r5.xy, r9, c10.y, r5  //warped normal move nx, ny according to
                           // cos*wavedir*waveheight
mov  r4, v3           // tangent
mad  r4.z, -r9.x, c10.y, r4.z  // warped tangent vector

dp3 r10.x, r5, r5     // normalize the normal
rsq  r10.y, r10.x
mul  r5, r5, r10.y

dp3 r10.x, r4, r4     // normalize the tangent
rsq  r10.y, r10.x
mul  r4, r4, r10.y

mul  r3, r4.yzxw, r5.zxyw   // xprod to find binormal
mad  r3, r4.zxyw, -r5.yzxw, r3


mov  r6, c8           // get eye pos into object space
m4x4 r2, r6, c20

sub  r2, r2, r0       // find view vector

dp3  r10.x, r2, r2    // normalize view vector
rsq  r10.y, r10.x
mul  r2, r2, r10.y

mov  r0, c16.x
mul  r0, r0, c18.xyxy
frc  r0.xy, r0        // frac of incoming time
add  r0, v2, r0       // add time to tex coords
mov  oT0, r0          // output tex coords

mov  r0, c16.x
mul  r0, r0, c18.zwzw
frc  r0.xy, r0        // frac of incoming time
add  r0, v2, r0       // add time to tex coords
mov  oT1, r0.yxzw     // output distorted tex coord1

mov  oT2, r2          // pass in view vector (world space)
mov  oT3, r3          // tangent 
mov  oT4, r4          // binormal
mov  oT5, r5          // normal
        };


        PixelShader = 
        asm
        {
ps.1.4

texld  r0, t0     // bump map 0
texld  r1, t1     // bump map 1
texcrd r2.rgb, t2 // view vec
texcrd r3.rgb, t3 // tangent
texcrd r4.rgb, t4 // binormal
texcrd r5.rgb, t5 // normal

add_d4 r0.xy, r0_bx2, r1_bx2 // scaled avg of 2 bumpmap xy offsets

mul  r1.rgb, r0.x, r3        // put bump maps into world space
mad  r1.rgb, r0.y, r4, r1  
mad  r1.rgb, r0.z, r5, r1

dp3  r0.rgb, r1, r2          // V.N
mad  r2.rgb, r1, r0_x2, -r2  // R = 2N(V.N)-V

mov_sat  r0.rgb, r0_x2           // 2 * V.N (sample over range of 1D map)

phase

texld  r2, r2             // env map
texld  r3, r0             // index frenel map using 2*N.V

mul  r2.rgb, r2, r2          // square env map
+mul r2.a, r2.g, r2.g        // use green of env as specular
mul  r2.rgb, r2, 1-r0.r      // fresnel term
+mul r2.a, r2.a, r2.a        // specular ^4

add_d4_sat r2.rgb, r2, r3_x2  // += water color
+mul  r2.a, r2.a, r2.a        // specular ^8

mad_sat r0, r2.a, c1, r2      // += specular * specular color 
        };
    }
}
