//=============================================================================
// SsaoBlur.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a bilateral edge preserving blur of the ambient map.  We use 
// a pixel shader instead of compute shader to avoid the switch from 
// compute mode to rendering mode.  The texture cache makes up for some of the
// loss of not having shared memory.  The ambient map uses 16-bit texture
// format, which is small, so we should be able to fit a lot of texels
// in the cache.
//=============================================================================

cbuffer cbPerFrame
{
	float gTexelWidth;
	float gTexelHeight;
};

cbuffer cbSettings
{
	float gWeights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 5;
};
 
// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalDepthMap;
Texture2D gInputImage;
 
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU  = CLAMP;
    AddressV  = CLAMP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
	float2 Tex   : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Already in NDC space.
	vout.PosH = float4(vin.PosL, 1.0f);

	// Pass onto pixel shader.
	vout.Tex = vin.Tex;
	
    return vout;
}


float4 PS(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	float2 texOffset;
	if(gHorizontalBlur)
	{
		texOffset = float2(gTexelWidth, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, gTexelHeight);
	}

	// The center value always contributes to the sum.
	float4 color      = gWeights[5]*gInputImage.SampleLevel(samInputImage, pin.Tex, 0.0);
	float totalWeight = gWeights[5];
	 
	float4 centerNormalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);

	for(float i = -gBlurRadius; i <=gBlurRadius; ++i)
	{
		// We already added in the center weight.
		if( i == 0 )
			continue;

		float2 tex = pin.Tex + i*texOffset;

		float4 neighborNormalDepth = gNormalDepthMap.SampleLevel(
			samNormalDepth, tex, 0.0f);

		//
		// If the center value and neighbor values differ too much (either in 
		// normal or depth), then we assume we are sampling across a discontinuity.
		// We discard such samples from the blur.
		//
	
		if( dot(neighborNormalDepth.xyz, centerNormalDepth.xyz) >= 0.8f &&
		    abs(neighborNormalDepth.a - centerNormalDepth.a) <= 0.2f )
		{
			float weight = gWeights[i+gBlurRadius];

			// Add neighbor pixel to blur.
			color += weight*gInputImage.SampleLevel(
				samInputImage, tex, 0.0);
		
			totalWeight += weight;
		}
	}

	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(true) ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(false) ) );
    }
}
 