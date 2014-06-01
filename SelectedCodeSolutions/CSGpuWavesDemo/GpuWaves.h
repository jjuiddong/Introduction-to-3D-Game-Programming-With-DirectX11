//***************************************************************************************
// GpuWaves.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs the calculations for the wave simulation using the ComputeShader on the GPU.  
// The solution is saved to a floating-point texture.  The client must then set this 
// texture as a SRV and do the displacement mapping in the vertex shader over a grid.
//***************************************************************************************

#ifndef GPUWAVES_H
#define GPUWAVES_H

#include <Windows.h>
#include <xnamath.h>
#include "d3dUtil.h"

class GpuWaves
{
public:
	GpuWaves();
	~GpuWaves();

	UINT RowCount()const;
	UINT ColumnCount()const;
	UINT VertexCount()const;
	UINT TriangleCount()const;
	float Width()const;
	float Depth()const;

	ID3D11ShaderResourceView* GetDisplacementMap();

	// How many groups do we need to dispatch to cover the wave grid.  
	// Note that mNumRows and mNumCols should be divisible by 16
	// so there is no remainder when we divide into thread groups.
	void Init(ID3D11Device* device, UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(ID3D11DeviceContext* dc, float dt);
	void Disturb(ID3D11DeviceContext* dc, UINT i, UINT j, float magnitude);

private:
	void BuildWaveSimulationViews(ID3D11Device* device);

private:

	UINT mNumRows;
	UINT mNumCols;

	UINT mVertexCount;
	UINT mTriangleCount;

	// Simulation constants we can precompute.
	float mK[3];

	float mTimeStep;
	float mSpatialStep;

	ID3D11ShaderResourceView* mWavesPrevSolSRV;
	ID3D11ShaderResourceView* mWavesCurrSolSRV;
	ID3D11ShaderResourceView* mWavesNextSolSRV;

	ID3D11UnorderedAccessView* mWavesPrevSolUAV;
	ID3D11UnorderedAccessView* mWavesCurrSolUAV;
	ID3D11UnorderedAccessView* mWavesNextSolUAV;
};

#endif // GPUWAVES_H