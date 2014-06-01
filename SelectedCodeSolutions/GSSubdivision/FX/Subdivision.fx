//***************************************************************************************
// Subdivision.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Uses the geometry shader to subdivide the geometry based on distance to the eye.
//***************************************************************************************

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
};

cbuffer cbFixed
{
	float3 gIsocahedronPosW = {0.0f, 0.0f, 0.0f};
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex     : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormalL;
 
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}

void Subdivide(VertexOut inVerts[3], out VertexOut outVerts[6])
{
	//       1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// 0    m2     2

	VertexOut m[3];

	// Compute edge midpoints.
	m[0].PosL = 0.5f*(inVerts[0].PosL+inVerts[1].PosL);
	m[1].PosL = 0.5f*(inVerts[1].PosL+inVerts[2].PosL);
	m[2].PosL = 0.5f*(inVerts[2].PosL+inVerts[0].PosL);
 
	// Project onto unit sphere
	m[0].PosL = normalize(m[0].PosL);
	m[1].PosL = normalize(m[1].PosL);
	m[2].PosL = normalize(m[2].PosL);

	// Derive normals.
	m[0].NormalL = m[0].PosL;
	m[1].NormalL = m[1].PosL;
	m[2].NormalL = m[2].PosL;

	// Interpolate texture coordinates.
	m[0].Tex = 0.5f*(inVerts[0].Tex+inVerts[1].Tex);
	m[1].Tex = 0.5f*(inVerts[1].Tex+inVerts[2].Tex);
	m[2].Tex = 0.5f*(inVerts[2].Tex+inVerts[0].Tex);

	outVerts[0] = inVerts[0];
	outVerts[1] = m[0];
	outVerts[2] = m[2];
	outVerts[3] = m[1];
	outVerts[4] = inVerts[2];
	outVerts[5] = inVerts[1];
};

void OutputSubdivision(VertexOut v[6], inout TriangleStream<GeoOut> triStream)
{
	GeoOut gout[6];

	[unroll]
	for(int i = 0; i < 6; ++i)
	{
		// Transform to world space space. 
		gout[i].PosW    = mul(float4(v[i].PosL, 1.0f), gWorld).xyz;
		gout[i].NormalW = mul(v[i].NormalL, (float3x3)gWorldInvTranspose);

		// Transform to homogeneous clip space.
		gout[i].PosH = mul(float4(v[i].PosL, 1.0f), gWorldViewProj);

		gout[i].Tex  = v[i].Tex;
	}
		
	//       1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// 0    m2     2
	//
	// We can draw the subdivision in two strips:
	//     Strip 1: bottom three triangles
	//     Strip 2: top triangle
	
	[unroll]
	for(int j = 0; j < 5; ++j)
	{
		triStream.Append(gout[j]);
	}
	triStream.RestartStrip();
	
	triStream.Append(gout[1]);
	triStream.Append(gout[5]);
	triStream.Append(gout[3]);	
}

// IF we subdivide twice, we can output 32 vertices.
[maxvertexcount(32)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> triStream)
{
	float d = distance(gEyePosW, gIsocahedronPosW);

	if( d < 15 ) // Subdivide twice.
	{
		VertexOut v[6];
		Subdivide(gin, v);

		// Subdivide each triangle from the previous subdivision.
		VertexOut tri0[3] = {v[0], v[1], v[2]};
		VertexOut tri1[3] = {v[1], v[3], v[2]};
		VertexOut tri2[3] = {v[2], v[3], v[4]};
		VertexOut tri3[3] = {v[1], v[5], v[3]};
 
		Subdivide(tri0, v);
		OutputSubdivision(v, triStream);
		triStream.RestartStrip();
		
		Subdivide(tri1, v);
		OutputSubdivision(v, triStream);
		triStream.RestartStrip();
		
		Subdivide(tri2, v);
		OutputSubdivision(v, triStream);
		triStream.RestartStrip();
		
		Subdivide(tri3, v);
		OutputSubdivision(v, triStream);
	}
	else if( d < 30.0f ) // Subdivide once.
	{
		VertexOut v[6];
		Subdivide(gin, v);
		OutputSubdivision(v, triStream);
	}
	else // No subdivision
	{
		GeoOut gout[3];
		[unroll]
		for(int i = 0; i < 3; ++i)
		{
			// Transform to world space space. 
			gout[i].PosW    = mul(float4(gin[i].PosL, 1.0f), gWorld).xyz;
			gout[i].NormalW = mul(gin[i].NormalL, (float3x3)gWorldInvTranspose);

			// Transform to homogeneous clip space.
			gout[i].PosH = mul(float4(gin[i].PosL, 1.0f), gWorldViewProj);

			gout[i].Tex  = gin[i].Tex;

			triStream.Append(gout[i]);
		}
	}
}

float4 PS(GeoOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gAlphaClip, uniform bool gFogEnabled) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
   
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}

	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{
		// Start with a sum of zero.
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, false, false) ) );
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, false) ) );
    }
}

technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, true) ) );
    }
}

technique11 Light3TexFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, false, true) ) );
    }
}

technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, true) ) );
    }
}