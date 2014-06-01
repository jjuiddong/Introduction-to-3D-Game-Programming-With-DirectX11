//***************************************************************************************
// Ssao.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Ssao.h"
#include "Camera.h"
#include "Effects.h"
#include "Vertex.h"

Ssao::Ssao(ID3D11Device* device, ID3D11DeviceContext* dc, int width, int height, float fovy, float farZ)
	: md3dDevice(device), mDC(dc), mScreenQuadVB(0), mScreenQuadIB(0), mRandomVectorSRV(0),
	  mNormalDepthRTV(0), mNormalDepthSRV(0), mAmbientRTV0(0), mAmbientSRV0(0), mAmbientRTV1(0), mAmbientSRV1(0)

{
	OnSize(width, height, fovy, farZ);

	BuildFullScreenQuad();
	BuildOffsetVectors();
	BuildRandomVectorTexture();
}

Ssao::~Ssao()
{
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);
	ReleaseCOM(mRandomVectorSRV);

	ReleaseTextureViews();
}

ID3D11ShaderResourceView* Ssao::NormalDepthSRV()
{
	return mNormalDepthSRV;
}

ID3D11ShaderResourceView* Ssao::AmbientSRV()
{
	return mAmbientSRV0;
}

void Ssao::OnSize(int width, int height, float fovy, float farZ)
{
	mRenderTargetWidth = width;
	mRenderTargetHeight = height;

	// We render to ambient map at half the resolution.
	mAmbientMapViewport.TopLeftX = 0.0f;
	mAmbientMapViewport.TopLeftY = 0.0f;
	mAmbientMapViewport.Width = width / 2.0f;
	mAmbientMapViewport.Height = height / 2.0f;
	mAmbientMapViewport.MinDepth = 0.0f;
	mAmbientMapViewport.MaxDepth = 1.0f;
	
	BuildFrustumFarCorners(fovy, farZ);
    BuildTextureViews();
}
 
void Ssao::SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv)
{
    ID3D11RenderTargetView* renderTargets[1] = {mNormalDepthRTV};
    mDC->OMSetRenderTargets(1, renderTargets, dsv);

	// Clear view space normal to (0,0,-1) and clear depth to be very far away.  
	float clearColor[] = {0.0f, 0.0f, -1.0f, 1e5f};
	mDC->ClearRenderTargetView(mNormalDepthRTV, clearColor);
}

void Ssao::ComputeSsao(const Camera& camera)
{
	// Bind the ambient map as the render target.  Observe that this pass does not bind 
	// a depth/stencil buffer--it does not need it, and without one, no depth test is
	// performed, which is what we want.
	ID3D11RenderTargetView* renderTargets[1] = {mAmbientRTV0};
	mDC->OMSetRenderTargets(1, renderTargets, 0);
	mDC->ClearRenderTargetView(mAmbientRTV0, reinterpret_cast<const float*>(&Colors::Black));
	mDC->RSSetViewports(1, &mAmbientMapViewport);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P  = camera.Proj();
	XMMATRIX PT = XMMatrixMultiply(P, T);

	Effects::SsaoFX->SetViewToTexSpace(PT);
	Effects::SsaoFX->SetOffsetVectors(mOffsets);
	Effects::SsaoFX->SetFrustumCorners(mFrustumFarCorner);
	Effects::SsaoFX->SetNormalDepthMap(mNormalDepthSRV);
	Effects::SsaoFX->SetRandomVecMap(mRandomVectorSRV);

	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

	mDC->IASetInputLayout(InputLayouts::Basic32);
    mDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDC->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mDC->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);
	
	ID3DX11EffectTechnique* tech = Effects::SsaoFX->SsaoTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		tech->GetPassByIndex(p)->Apply(0, mDC);
		mDC->DrawIndexed(6, 0, 0);
    }
}

void Ssao::BlurAmbientMap(int blurCount)
{
	for(int i = 0; i < blurCount; ++i)
	{
		// Ping-pong the two ambient map textures as we apply
		// horizontal and vertical blur passes.
		BlurAmbientMap(mAmbientSRV0, mAmbientRTV1, true);
		BlurAmbientMap(mAmbientSRV1, mAmbientRTV0, false);
	}
}

void Ssao::BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur)
{
	ID3D11RenderTargetView* renderTargets[1] = {outputRTV};
	mDC->OMSetRenderTargets(1, renderTargets, 0);
	mDC->ClearRenderTargetView(outputRTV, reinterpret_cast<const float*>(&Colors::Black));
	mDC->RSSetViewports(1, &mAmbientMapViewport);

	Effects::SsaoBlurFX->SetTexelWidth(1.0f / mAmbientMapViewport.Width );
	Effects::SsaoBlurFX->SetTexelHeight(1.0f / mAmbientMapViewport.Height );
	Effects::SsaoBlurFX->SetNormalDepthMap(mNormalDepthSRV);
	Effects::SsaoBlurFX->SetInputImage(inputSRV);

	ID3DX11EffectTechnique* tech;
	if(horzBlur)
	{
		tech = Effects::SsaoBlurFX->HorzBlurTech;
	}
	else
	{
		tech = Effects::SsaoBlurFX->VertBlurTech;
	}

	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

	mDC->IASetInputLayout(InputLayouts::Basic32);
    mDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDC->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mDC->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		tech->GetPassByIndex(p)->Apply(0, mDC);
		mDC->DrawIndexed(6, 0, 0);

		// Unbind the input SRV as it is going to be an output in the next blur.
		Effects::SsaoBlurFX->SetInputImage(0);
		tech->GetPassByIndex(p)->Apply(0, mDC);
    }
}

void Ssao::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)mRenderTargetWidth / (float)mRenderTargetHeight;

	float halfHeight = farZ * tanf( 0.5f*fovy );
	float halfWidth  = aspect * halfHeight;

	mFrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	mFrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	mFrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	mFrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}

void Ssao::BuildFullScreenQuad()
{
	Vertex::Basic32 v[4];

	v[0].Pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].Pos = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].Pos = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].Pos = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
	v[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].Normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].Normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].Tex = XMFLOAT2(0.0f, 1.0f);
	v[1].Tex = XMFLOAT2(0.0f, 0.0f);
	v[2].Tex = XMFLOAT2(1.0f, 0.0f);
	v[3].Tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 4;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
	
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));
	
	USHORT indices[6] = 
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(USHORT) * 6;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;

	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void Ssao::BuildTextureViews()
{
	ReleaseTextureViews();
	
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mRenderTargetWidth;
	texDesc.Height = mRenderTargetHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	
 
	ID3D11Texture2D* normalDepthTex = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &normalDepthTex));
	HR(md3dDevice->CreateShaderResourceView(normalDepthTex, 0, &mNormalDepthSRV));
	HR(md3dDevice->CreateRenderTargetView(normalDepthTex, 0, &mNormalDepthRTV));
	
	// view saves a reference.
	ReleaseCOM(normalDepthTex);
	
	// Render ambient map at half resolution.
	texDesc.Width = mRenderTargetWidth / 2;
	texDesc.Height = mRenderTargetHeight / 2;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	ID3D11Texture2D* ambientTex0 = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex0));
	HR(md3dDevice->CreateShaderResourceView(ambientTex0, 0, &mAmbientSRV0));
	HR(md3dDevice->CreateRenderTargetView(ambientTex0, 0, &mAmbientRTV0));
	
	ID3D11Texture2D* ambientTex1 = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex1));
	HR(md3dDevice->CreateShaderResourceView(ambientTex1, 0, &mAmbientSRV1));
	HR(md3dDevice->CreateRenderTargetView(ambientTex1, 0, &mAmbientRTV1));


	// view saves a reference.
	ReleaseCOM(ambientTex0);
	ReleaseCOM(ambientTex1);
}

void Ssao::ReleaseTextureViews()
{
	ReleaseCOM(mNormalDepthRTV);
	ReleaseCOM(mNormalDepthSRV);

	ReleaseCOM(mAmbientRTV0);
	ReleaseCOM(mAmbientSRV0);

	ReleaseCOM(mAmbientRTV1);
	ReleaseCOM(mAmbientSRV1);
}

void Ssao::BuildRandomVectorTexture()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA initData = {0};
	initData.SysMemPitch = 256*sizeof(XMCOLOR);

	XMCOLOR color[256*256];
	for(int i = 0; i < 256; ++i)
	{
		for(int j = 0; j < 256; ++j)
		{
			XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			color[i*256+j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;
	
	ID3D11Texture2D* tex = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, &initData, &tex));
	
	HR(md3dDevice->CreateShaderResourceView(tex, 0, &mRandomVectorSRV));
	
	// view saves a reference.
	ReleaseCOM(tex);
}

void Ssao::BuildOffsetVectors()
{
    // Start with 14 uniformly distributed vectors.  We choose the 8 corners of the cube
	// and the 6 center points along each cube face.  We always alternate the points on 
	// opposites sides of the cubes.  This way we still get the vectors spread out even
	// if we choose to use less than 14 samples.
	
	// 8 cube corners
	mOffsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	mOffsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	mOffsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	mOffsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

    for(int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = MathHelper::RandF(0.25f, 1.0f);
		
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&mOffsets[i]));
		
		XMStoreFloat4(&mOffsets[i], v);
	}
}