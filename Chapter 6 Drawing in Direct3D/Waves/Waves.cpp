//=======================================================================================
// Waves.cpp by Frank Luna (C) 2008 All Rights Reserved.
//=======================================================================================

#include "Waves.h"
#include <algorithm>
#include <vector>
#include <cassert>

Waves::Waves()
: mNumRows(0), mNumCols(0), mVertexCount(0), mTriangleCount(0), 
  mK1(0.0f), mK2(0.0f), mK3(0.0f), mTimeStep(0.0f), mSpatialStep(0.0f),
  mPrevSolution(0), mCurrSolution(0)
{
}

Waves::~Waves()
{
	delete[] mPrevSolution;
	delete[] mCurrSolution;
}

UINT Waves::RowCount()const
{
	return mNumRows;
}

UINT Waves::ColumnCount()const
{
	return mNumCols;
}

UINT Waves::VertexCount()const
{
	return mVertexCount;
}

UINT Waves::TriangleCount()const
{
	return mTriangleCount;
}

void Waves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	mNumRows  = m;
	mNumCols  = n;

	mVertexCount   = m*n;
	mTriangleCount = (m-1)*(n-1)*2;

	mTimeStep    = dt;
	mSpatialStep = dx;

	float d = damping*dt+2.0f;
	float e = (speed*speed)*(dt*dt)/(dx*dx);
	mK1     = (damping*dt-2.0f)/ d;
	mK2     = (4.0f-8.0f*e) / d;
	mK3     = (2.0f*e) / d;

	// In case Init() called again.
	delete[] mPrevSolution;
	delete[] mCurrSolution;

	mPrevSolution = new XMFLOAT3[m*n];
	mCurrSolution = new XMFLOAT3[m*n];

	// Generate grid vertices in system memory.

	float halfWidth = (n-1)*dx*0.5f;
	float halfDepth = (m-1)*dx*0.5f;
	for(UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dx;
		for(UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;

			mPrevSolution[i*n+j] = XMFLOAT3(x, 0.0f, z);
			mCurrSolution[i*n+j] = XMFLOAT3(x, 0.0f, z);
		}
	}
}

void Waves::Update(float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if( t >= mTimeStep )
	{
		// Only update interior points; we use zero boundary conditions.
		for(DWORD i = 1; i < mNumRows-1; ++i)
		{
			for(DWORD j = 1; j < mNumCols-1; ++j)
			{
				// After this update we will be discarding the old previous
				// buffer, so overwrite that buffer with the new update.
				// Note how we can do this inplace (read/write to same element) 
				// because we won't need prev_ij again and the assignment happens last.

				// Note j indexes x and i indexes z: h(x_j, z_i, t_k)
				// Moreover, our +z axis goes "down"; this is just to 
				// keep consistent with our row indices going down.

				mPrevSolution[i*mNumCols+j].y = 
					mK1*mPrevSolution[i*mNumCols+j].y +
					mK2*mCurrSolution[i*mNumCols+j].y +
					mK3*(mCurrSolution[(i+1)*mNumCols+j].y + 
					     mCurrSolution[(i-1)*mNumCols+j].y + 
					     mCurrSolution[i*mNumCols+j+1].y + 
						 mCurrSolution[i*mNumCols+j-1].y);
			}
		}

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.
		std::swap(mPrevSolution, mCurrSolution);

		t = 0.0f; // reset time
	}
}

void Waves::Disturb(UINT i, UINT j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < mNumRows-2);
	assert(j > 1 && j < mNumCols-2);

	float halfMag = 0.5f*magnitude;

	// Disturb the ijth vertex height and its neighbors.
	mCurrSolution[i*mNumCols+j].y     += magnitude;
	mCurrSolution[i*mNumCols+j+1].y   += halfMag;
	mCurrSolution[i*mNumCols+j-1].y   += halfMag;
	mCurrSolution[(i+1)*mNumCols+j].y += halfMag;
	mCurrSolution[(i-1)*mNumCols+j].y += halfMag;
}
	
