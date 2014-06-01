//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, device, &mFX));
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}
#pragma endregion

#pragma region AmbientOcclusionEffect
AmbientOcclusionEffect::AmbientOcclusionEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	AmbientOcclusionTech = mFX->GetTechniqueByName("AmbientOcclusion");

	WorldViewProj        = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

AmbientOcclusionEffect::~AmbientOcclusionEffect()
{
}
#pragma endregion

#pragma region Effects

AmbientOcclusionEffect* Effects::AmbientOcclusionFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
	AmbientOcclusionFX = new AmbientOcclusionEffect(device, L"FX/AmbientOcclusion.fxo");
}

void Effects::DestroyAll()
{
	SafeDelete(AmbientOcclusionFX);
}
#pragma endregion