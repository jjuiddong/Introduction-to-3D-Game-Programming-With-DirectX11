//***************************************************************************************
// GpuWaves.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "GpuWaves.h"
#include "Effects.h"
#include <algorithm>
#include <vector>
#include <cassert>

GpuWaves::GpuWaves()
: mNumRows(0), mNumCols(0), mVertexCount(0), mTriangleCount(0), 
  mTimeStep(0.0f), mSpatialStep(0.0f),
  mWavesPrevSolSRV(0), mWavesCurrSolSRV(0), mWavesNextSolSRV(0), 
  mWavesPrevSolUAV(0), mWavesCurrSolUAV(0), mWavesNextSolUAV(0)
{
	mK[0] = 0.0f;
	mK[1] = 0.0f;
	mK[2] = 0.0f;
}

GpuWaves::~GpuWaves()
{
	ReleaseCOM(mWavesPrevSolSRV);
	ReleaseCOM(mWavesCurrSolSRV);
	ReleaseCOM(mWavesNextSolSRV);

	ReleaseCOM(mWavesPrevSolUAV);
	ReleaseCOM(mWavesCurrSolUAV);
	ReleaseCOM(mWavesNextSolUAV);
}

UINT GpuWaves::RowCount()const
{
	return mNumRows;
}

UINT GpuWaves::ColumnCount()const
{
	return mNumCols;
}

UINT GpuWaves::VertexCount()const
{
	return mVertexCount;
}

UINT GpuWaves::TriangleCount()const
{
	return mTriangleCount;
}

float GpuWaves::Width()const
{
	return mNumCols*mSpatialStep;
}

float GpuWaves::Depth()const
{
	return mNumRows*mSpatialStep;
}

ID3D11ShaderResourceView* GpuWaves::GetDisplacementMap()
{
	// After an Update, the current solution stores the solution we want to render.
	return mWavesCurrSolSRV;
}

void GpuWaves::Init(ID3D11Device* device, UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	mNumRows  = m;
	mNumCols  = n;

	mVertexCount   = m*n;
	mTriangleCount = (m-1)*(n-1)*2;

	mTimeStep    = dt;
	mSpatialStep = dx;

	float d = damping*dt+2.0f;
	float e = (speed*speed)*(dt*dt)/(dx*dx);
	mK[0]   = (damping*dt-2.0f)/ d;
	mK[1]   = (4.0f-8.0f*e) / d;
	mK[2]   = (2.0f*e) / d;

	BuildWaveSimulationViews(device);
}

void GpuWaves::Update(ID3D11DeviceContext* dc, float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if( t >= mTimeStep )
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		Effects::WaveSimFX->SetWaveConstants(mK);

		Effects::WaveSimFX->SetPrevSolInput(mWavesPrevSolSRV);
		Effects::WaveSimFX->SetCurrSolInput(mWavesCurrSolSRV);
		Effects::WaveSimFX->SetNextSolOutput(mWavesNextSolUAV);

		Effects::WaveSimFX->UpdateWavesTech->GetDesc( &techDesc );
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			ID3DX11EffectPass* pass = Effects::WaveSimFX->UpdateWavesTech->GetPassByIndex(p);
			pass->Apply(0, dc);

			// How many groups do we need to dispatch to cover the wave grid.  
			// Note that mNumRows and mNumCols should be divisible by 16
			// so there is no remainder.
			UINT numGroupsX = mNumCols / 16;
			UINT numGroupsY = mNumRows / 16;
			dc->Dispatch(numGroupsX, numGroupsY, 1);
		}

		// Unbind the input textures from the CS for good housekeeping.
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		dc->CSSetShaderResources( 0, 1, nullSRV );

		// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
		// and a resource cannot be both an output and input at the same time.
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		dc->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );

		// Disable compute shader.
		dc->CSSetShader(0, 0, 0);

		//
		// Ping-pong buffers in preparation for the next update.
		// The previous solution is no longer needed and becomes the target of the next solution in the next update.
		// The current solution becomes the previous solution.
		// The next solution becomes the current solution.
		//

		ID3D11ShaderResourceView* srvTemp = mWavesPrevSolSRV;
		mWavesPrevSolSRV = mWavesCurrSolSRV;
		mWavesCurrSolSRV = mWavesNextSolSRV;
		mWavesNextSolSRV = srvTemp;

		ID3D11UnorderedAccessView* uavTemp = mWavesPrevSolUAV;
		mWavesPrevSolUAV = mWavesCurrSolUAV;
		mWavesCurrSolUAV = mWavesNextSolUAV;
		mWavesNextSolUAV = uavTemp;

		t = 0.0f; // reset time
	}
}

void GpuWaves::Disturb(ID3D11DeviceContext* dc, UINT i, UINT j, float magnitude)
{
	D3DX11_TECHNIQUE_DESC techDesc;

	// The grid element to displace.
	Effects::WaveSimFX->SetDisturbIndex(i, j);

	// The magnitude of the displacement.
	Effects::WaveSimFX->SetDisturbMag(magnitude);

	// Displace the current solution heights to generate a wave.
	Effects::WaveSimFX->SetCurrSolOutput(mWavesCurrSolUAV);

	Effects::WaveSimFX->DisturbWavesTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::WaveSimFX->DisturbWavesTech->GetPassByIndex(p);
		pass->Apply(0, dc);

		// One thread group kicks off one thread, which displaces the height of one
		// vertex and its neighbors.
		dc->Dispatch(1, 1, 1);
	}

	// Unbind output from compute shader so we can use it as a shader input (a resource cannot be bound
	// as an output and input).
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	dc->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
}

void GpuWaves::BuildWaveSimulationViews(ID3D11Device* device)
{
	ReleaseCOM(mWavesPrevSolSRV);
	ReleaseCOM(mWavesCurrSolSRV);
	ReleaseCOM(mWavesNextSolSRV);

	ReleaseCOM(mWavesPrevSolUAV);
	ReleaseCOM(mWavesCurrSolUAV);
	ReleaseCOM(mWavesNextSolUAV);

	// All the textures for the wave simulation will be bound as a shader resource and
	// unordered access view at some point since we ping-pong the buffers.
	
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width     = mNumCols;
	texDesc.Height    = mNumRows;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
	texDesc.Format    = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count   = 1;
	texDesc.SampleDesc.Quality = 0;
    texDesc.Usage     = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags      = 0;

	// Zero out the buffers initially.
	std::vector<float> zero(mNumRows*mNumCols, 0.0f);
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &zero[0];
	initData.SysMemPitch = mNumCols*sizeof(float);
	
	ID3D11Texture2D* prevWaveSolutionTex = 0;
	ID3D11Texture2D* currWaveSolutionTex = 0;
	ID3D11Texture2D* nextWaveSolutionTex = 0;
	HR(device->CreateTexture2D(&texDesc, &initData, &prevWaveSolutionTex));
	HR(device->CreateTexture2D(&texDesc, &initData, &currWaveSolutionTex));
	HR(device->CreateTexture2D(&texDesc, &initData, &nextWaveSolutionTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(prevWaveSolutionTex, &srvDesc, &mWavesPrevSolSRV));
	HR(device->CreateShaderResourceView(currWaveSolutionTex, &srvDesc, &mWavesCurrSolSRV));
	HR(device->CreateShaderResourceView(nextWaveSolutionTex, &srvDesc, &mWavesNextSolSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(device->CreateUnorderedAccessView(prevWaveSolutionTex, &uavDesc, &mWavesPrevSolUAV));
	HR(device->CreateUnorderedAccessView(currWaveSolutionTex, &uavDesc, &mWavesCurrSolUAV));
	HR(device->CreateUnorderedAccessView(nextWaveSolutionTex, &uavDesc, &mWavesNextSolUAV));

	// Views save a reference to the texture so we can release our reference.
	ReleaseCOM(prevWaveSolutionTex);
	ReleaseCOM(currWaveSolutionTex);
	ReleaseCOM(nextWaveSolutionTex);
}

	
