//***************************************************************************************
// AmbientOcclusionApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates static precomputed vertex ambient occlusion.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Octree.h"

class AmbientOcclusionApp : public D3DApp 
{
public:
	AmbientOcclusionApp(HINSTANCE hInstance);
	~AmbientOcclusionApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildVertexAmbientOcclusion(
		std::vector<Vertex::AmbientOcclusion>& vertices,
	    const std::vector<UINT>& indices);

	void BuildSkullGeometryBuffers();

private:

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;

	Camera mCam;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	AmbientOcclusionApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

AmbientOcclusionApp::AmbientOcclusionApp(HINSTANCE hInstance)
: D3DApp(hInstance), mSkullVB(0), mSkullIB(0), mSkullIndexCount(0)
{
	mMainWndCaption = L"Ambient Occlusion";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 5.0f, -5.0f);
	mCam.LookAt(
		XMFLOAT3(-4.0f, 4.0f, -4.0f),
		XMFLOAT3(0.0f, 2.2f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));
}

AmbientOcclusionApp::~AmbientOcclusionApp()
{
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll(); 
}

bool AmbientOcclusionApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	BuildSkullGeometryBuffers();

	return true;
}

void AmbientOcclusionApp::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void AmbientOcclusionApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if( GetAsyncKeyState('W') & 0x8000 )
		mCam.Walk(10.0f*dt);

	if( GetAsyncKeyState('S') & 0x8000 )
		mCam.Walk(-10.0f*dt);

	if( GetAsyncKeyState('A') & 0x8000 )
		mCam.Strafe(-10.0f*dt);

	if( GetAsyncKeyState('D') & 0x8000 )
		mCam.Strafe(10.0f*dt);
}

void AmbientOcclusionApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::AmbientOcclusion);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex::AmbientOcclusion);
    UINT offset = 0;

	mCam.UpdateViewMatrix();
 
	XMMATRIX view     = mCam.View();
	XMMATRIX proj     = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

 
	ID3DX11EffectTechnique* activeSkullTech = Effects::AmbientOcclusionFX->AmbientOcclusionTech;

    D3DX11_TECHNIQUE_DESC techDesc;
	activeSkullTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the skull.

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::AmbientOcclusionFX->SetWorldViewProj(worldViewProj);

		activeSkullTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void AmbientOcclusionApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void AmbientOcclusionApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void AmbientOcclusionApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void AmbientOcclusionApp::BuildVertexAmbientOcclusion(
	std::vector<Vertex::AmbientOcclusion>& vertices,
	const std::vector<UINT>& indices)
{
	UINT vcount = vertices.size();
	UINT tcount = indices.size()/3;

	std::vector<XMFLOAT3> positions(vcount);
	for(UINT i = 0; i < vcount; ++i)
		positions[i] = vertices[i].Pos;

	Octree octree;
	octree.Build(positions, indices);

	// For each vertex, count how many triangles contain the vertex.
	std::vector<int> vertexSharedCount(vcount);
	for(UINT i = 0; i < tcount; ++i)
	{
		UINT i0 = indices[i*3+0];
		UINT i1 = indices[i*3+1];
		UINT i2 = indices[i*3+2];

		XMVECTOR v0 = XMLoadFloat3(&vertices[i0].Pos);
		XMVECTOR v1 = XMLoadFloat3(&vertices[i1].Pos);
		XMVECTOR v2 = XMLoadFloat3(&vertices[i2].Pos);

		XMVECTOR edge0 = v1 - v0;
		XMVECTOR edge1 = v2 - v0;

		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

		XMVECTOR centroid = (v0 + v1 + v2)/3.0f;

		// Offset to avoid self intersection.
		centroid += 0.001f*normal;

		const int NumSampleRays = 32;
		float numUnoccluded = 0;
		for(int j = 0; j < NumSampleRays; ++j)
		{
			XMVECTOR randomDir = MathHelper::RandHemisphereUnitVec3(normal);

			// TODO: Technically we should not count intersections that are far 
			// away as occluding the triangle, but this is OK for demo.
			if( !octree.RayOctreeIntersect(centroid, randomDir) )
			{
				numUnoccluded++;
			}
		}
		
		float ambientAccess = numUnoccluded / NumSampleRays;

		// Average with vertices that share this face.
		vertices[i0].AmbientAccess += ambientAccess;
		vertices[i1].AmbientAccess += ambientAccess;
		vertices[i2].AmbientAccess += ambientAccess;

		vertexSharedCount[i0]++;
		vertexSharedCount[i1]++;
		vertexSharedCount[i2]++;
	}

	// Finish average by dividing by the number of samples we added.
	for(UINT i = 0; i < vcount; ++i)
	{
		vertices[i].AmbientAccess /= vertexSharedCount[i];
	}
}

void AmbientOcclusionApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");
	
	if(!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;
	
	std::vector<Vertex::AmbientOcclusion> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3*tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
	}

	fin.close();


	BuildVertexAmbientOcclusion(vertices, indices);


    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::AmbientOcclusion) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

