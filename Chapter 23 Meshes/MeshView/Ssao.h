//***************************************************************************************
// Ssao.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Class encapsulates data and methods to perform screen space ambient occlusion.
//***************************************************************************************

#ifndef SSAO_H
#define SSAO_H

#include "d3dUtil.h"
 
 // Should we render AO at half resolution?
 
class Camera;

class Ssao
{
public:
	Ssao(ID3D11Device* device, ID3D11DeviceContext* dc, int width, int height, float fovy, float farZ);
	~Ssao();

	ID3D11ShaderResourceView* NormalDepthSRV();
	ID3D11ShaderResourceView* AmbientSRV();

	///<summary>
	/// Call when the backbuffer is resized.  
	///</summary>
	void OnSize(int width, int height, float fovy, float farZ);
 
	///<summary>
	/// Changes the render target to the NormalDepth render target.  Pass the 
	/// main depth buffer as the depth buffer to use when we render to the
	/// NormalDepth map.  This pass lays down the scene depth so that there in
	/// no overdraw in the subsequent rendering pass.
	///</summary>
	void SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv);

	///<summary>
	/// Changes the render target to the Ambient render target and draws a fullscreen
	/// quad to kick off the pixel shader to compute the AmbientMap.  We still keep the
	/// main depth buffer binded to the pipeline, but depth buffer read/writes
	/// are disabled, as we do not need the depth buffer computing the Ambient map.
	///</summary>
	void ComputeSsao(const Camera& camera);

	///<summary>
	/// Blurs the ambient map to smooth out the noise caused by only taking a
	/// few random samples per pixel.  We use an edge preserving blur so that 
	/// we do not blur across discontinuities--we want edges to remain edges.
	///</summary>
	void BlurAmbientMap(int blurCount);

private:
	Ssao(const Ssao& rhs);
	Ssao& operator=(const Ssao& rhs);

	void BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur);

	void BuildFrustumFarCorners(float fovy, float farZ);

	void BuildFullScreenQuad();

	void BuildTextureViews();
	void ReleaseTextureViews();

	void BuildRandomVectorTexture();

	void BuildOffsetVectors();

	void DrawFullScreenQuad();

private:
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDC;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	ID3D11ShaderResourceView* mRandomVectorSRV;

	ID3D11RenderTargetView* mNormalDepthRTV;
	ID3D11ShaderResourceView* mNormalDepthSRV;

	// Need two for ping-ponging during blur.
	ID3D11RenderTargetView* mAmbientRTV0;
	ID3D11ShaderResourceView* mAmbientSRV0;

	ID3D11RenderTargetView* mAmbientRTV1;
	ID3D11ShaderResourceView* mAmbientSRV1;


	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;

	XMFLOAT4 mFrustumFarCorner[4];

	XMFLOAT4 mOffsets[14];

	D3D11_VIEWPORT mAmbientMapViewport;
};

#endif // SSAO_H