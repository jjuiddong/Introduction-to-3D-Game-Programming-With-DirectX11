//=============================================================================
// WaveSim.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// UpdateWavesCS(): Solves 2D wave equation using the compute shader.
//
// DisturbWavesCS(): Runs one thread to disturb a grid height and its
//     neighbors to generate a wave. 
//=============================================================================

// For updating the simulation.
cbuffer cbUpdateSettings
{
	float gWaveConstants[3];
};

// For generating a wave.
cbuffer cbDisturbSettings
{
	float gDisturbMag;
	int2 gDisturbIndex;
};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

Texture2D gPrevSolInput; 
Texture2D gCurrSolInput;
RWTexture2D<float> gCurrSolOutput;
RWTexture2D<float> gNextSolOutput;

[numthreads(16, 16, 1)]
void UpdateWavesCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
	// We do not need to do bounds checking because:
	//	 *out-of-bounds reads return 0, which works for us--it just means the boundary of 
	//    our water simulation is clamped to 0 in local space.
	//   *out-of-bounds writes are a no-op.
	
	int x = dispatchThreadID.x;
	int y = dispatchThreadID.y;

	gNextSolOutput[int2(x,y)] = 
		gWaveConstants[0]* gPrevSolInput[int2(x,y)].r +
		gWaveConstants[1]* gCurrSolInput[int2(x,y)].r +
		gWaveConstants[2]*(
			gCurrSolInput[int2(x,y+1)].r + 
			gCurrSolInput[int2(x,y-1)].r + 
			gCurrSolInput[int2(x+1,y)].r + 
			gCurrSolInput[int2(x-1,y)].r);
			
/*
	// Equivalently using SampleLevel() instead of operator [].
	int x = dispatchThreadID.x;
	int y = dispatchThreadID.y;

	float2 c = float2(x,y)/512.0f;
	float2 t = float2(x,y-1)/512.0;
	float2 b = float2(x,y+1)/512.0;
	float2 l = float2(x-1,y)/512.0;
	float2 r = float2(x+1,y)/512.0;

	gNextSolOutput[int2(x,y)] = 
		gWaveConstants[0]*gPrevSolInput.SampleLevel(samPoint, c, 0.0f).r +
		gWaveConstants[1]*gCurrSolInput.SampleLevel(samPoint, c, 0.0f).r +
		gWaveConstants[2]*(
			gCurrSolInput.SampleLevel(samPoint, b, 0.0f).r + 
			gCurrSolInput.SampleLevel(samPoint, t, 0.0f).r + 
			gCurrSolInput.SampleLevel(samPoint, r, 0.0f).r + 
			gCurrSolInput.SampleLevel(samPoint, l, 0.0f).r);*/
}

technique11 UpdateWaves
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, UpdateWavesCS() ) );
    }
}

[numthreads(1, 1, 1)]
void DisturbWavesCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
	// We do not need to do bounds checking because:
	//	 *out-of-bounds reads return 0, which works for us--it just means the boundary of 
	//    our water simulation is clamped to 0 in local space.
	//   *out-of-bounds writes are a no-op.
	
	int x = gDisturbIndex.x;
	int y = gDisturbIndex.y;

	float halfMag = 0.5f*gDisturbMag;

	// Buffer is RW so operator += is well defined.
	gCurrSolOutput[int2(x,y)]   += gDisturbMag;
	gCurrSolOutput[int2(x+1,y)] += halfMag;
	gCurrSolOutput[int2(x-1,y)] += halfMag;
	gCurrSolOutput[int2(x,y+1)] += halfMag;
	gCurrSolOutput[int2(x,y-1)] += halfMag;
}

technique11 DisturbWaves
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, DisturbWavesCS() ) );
    }
}