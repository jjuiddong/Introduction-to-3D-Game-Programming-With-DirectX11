//***************************************************************************************
// d3dUtil.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "d3dUtil.h"

#include <d3d11.h> // Include the Direct3D 11 header
#include <dxgi.h> // Include the DXGI header
#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXTex.h> // Include the DirectXTex header
//DirectXCollision.h
//constant changes: XNAMATH_VERSION  is now DIRECTXMATH_VERSION etc

//cm add
using namespace DirectX;
using namespace DirectX::PackedVector;
//cm end


ID3D11ShaderResourceView* d3dHelper::CreateTexture2DArraySRV(
    ID3D11Device* device, ID3D11DeviceContext* context,
    std::vector<std::wstring>& filenames,
    DXGI_FORMAT format,
    UINT filter, 
    UINT mipFilter)
{
    //
    // Load the texture elements individually from file.  These textures
    // won't be used by the GPU (0 bind flags), they are just used to 
    // load the image data from file.  We use the STAGING usage so the
    // CPU can read the resource.
    //

    UINT size = filenames.size();

    std::vector<ID3D11Texture2D*> srcTex(size);
    for(UINT i = 0; i < size; ++i)
    {
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(filenames[i].c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
        if (FAILED(hr))
        {
            // Handle error
            return nullptr;
        }

        hr = DirectX::CreateTexture(device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), (ID3D11Resource**)&srcTex[i]);
        if (FAILED(hr))
        {
            // Handle error
            return nullptr;
        }
    }

    //
    // Create the texture array.  Each element in the texture 
    // array has the same format/dimensions.
    //

    D3D11_TEXTURE2D_DESC texElementDesc;
    srcTex[0]->GetDesc(&texElementDesc);

    D3D11_TEXTURE2D_DESC texArrayDesc = {};
    texArrayDesc.Width = texElementDesc.Width;
    texArrayDesc.Height = texElementDesc.Height;
    texArrayDesc.MipLevels = texElementDesc.MipLevels;
    texArrayDesc.ArraySize = size;
    texArrayDesc.Format = texElementDesc.Format;
    texArrayDesc.SampleDesc.Count = 1;
    texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags = 0;
    texArrayDesc.MiscFlags = 0;

    ID3D11Texture2D* texArray = nullptr;
    HR(device->CreateTexture2D(&texArrayDesc, 0, &texArray));

    // Copy individual texture elements into the texture array.
    for (UINT texElement = 0; texElement < size; ++texElement)
    {
        for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
        {
            context->CopySubresourceRegion(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
                0, 0, 0, srcTex[texElement], mipLevel, 0);
        }
    }

    // Create a resource view to the texture array.
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = texArrayDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MostDetailedMip = 0;
    viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
    viewDesc.Texture2DArray.FirstArraySlice = 0;
    viewDesc.Texture2DArray.ArraySize = size;

    ID3D11ShaderResourceView* texArraySRV = nullptr;
    PrintDXError(device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

    // Cleanup
    for (UINT i = 0; i < size; ++i)
    {
        ReleaseCOM(srcTex[i]);
    }

    ReleaseCOM(texArray);

    return texArraySRV;
}

ID3D11ShaderResourceView* d3dHelper::CreateRandomTexture1DSRV(ID3D11Device* device)
{
	// 
	// Create the random data.
	//
	XMFLOAT4 randomValues[1024];

	for(int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].y = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].z = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].w = MathHelper::RandF(-1.0f, 1.0f);
	}

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024*sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
    PrintDXError(device->CreateTexture1D(&texDesc, &initData, &randomTex));

	//
	// Create the resource view.
	//
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	
	ID3D11ShaderResourceView* randomTexSRV = 0;
    PrintDXError(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

	ReleaseCOM(randomTex);

	return randomTexSRV;
}

void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M)
{
	//
	// Left
	//
	planes[0].x = M(0,3) + M(0,0);
	planes[0].y = M(1,3) + M(1,0);
	planes[0].z = M(2,3) + M(2,0);
	planes[0].w = M(3,3) + M(3,0);

	//
	// Right
	//
	planes[1].x = M(0,3) - M(0,0);
	planes[1].y = M(1,3) - M(1,0);
	planes[1].z = M(2,3) - M(2,0);
	planes[1].w = M(3,3) - M(3,0);

	//
	// Bottom
	//
	planes[2].x = M(0,3) + M(0,1);
	planes[2].y = M(1,3) + M(1,1);
	planes[2].z = M(2,3) + M(2,1);
	planes[2].w = M(3,3) + M(3,1);

	//
	// Top
	//
	planes[3].x = M(0,3) - M(0,1);
	planes[3].y = M(1,3) - M(1,1);
	planes[3].z = M(2,3) - M(2,1);
	planes[3].w = M(3,3) - M(3,1);

	//
	// Near
	//
	planes[4].x = M(0,2);
	planes[4].y = M(1,2);
	planes[4].z = M(2,2);
	planes[4].w = M(3,2);

	//
	// Far
	//
	planes[5].x = M(0,3) - M(0,2);
	planes[5].y = M(1,3) - M(1,2);
	planes[5].z = M(2,3) - M(2,2);
	planes[5].w = M(3,3) - M(3,2);

	// Normalize the plane equations.
	for(int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}