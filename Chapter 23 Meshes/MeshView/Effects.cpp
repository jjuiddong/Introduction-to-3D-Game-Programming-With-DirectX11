//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	std::ifstream fin(filename.c_str(), std::ios::binary);

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

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech    = mFX->GetTechniqueByName("Light1");
	Light2Tech    = mFX->GetTechniqueByName("Light2");
	Light3Tech    = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech    = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech    = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech    = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	Light1ReflectTech    = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech    = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech    = mFX->GetTechniqueByName("Light3Reflect");

	Light0TexReflectTech = mFX->GetTechniqueByName("Light0TexReflect");
	Light1TexReflectTech = mFX->GetTechniqueByName("Light1TexReflect");
	Light2TexReflectTech = mFX->GetTechniqueByName("Light2TexReflect");
	Light3TexReflectTech = mFX->GetTechniqueByName("Light3TexReflect");

	Light0TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipReflect");
	Light1TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipReflect");
	Light2TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipReflect");
	Light3TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipReflect");

	Light1FogReflectTech    = mFX->GetTechniqueByName("Light1FogReflect");
	Light2FogReflectTech    = mFX->GetTechniqueByName("Light2FogReflect");
	Light3FogReflectTech    = mFX->GetTechniqueByName("Light3FogReflect");

	Light0TexFogReflectTech = mFX->GetTechniqueByName("Light0TexFogReflect");
	Light1TexFogReflectTech = mFX->GetTechniqueByName("Light1TexFogReflect");
	Light2TexFogReflectTech = mFX->GetTechniqueByName("Light2TexFogReflect");
	Light3TexFogReflectTech = mFX->GetTechniqueByName("Light3TexFogReflect");

	Light0TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogReflect");
	Light1TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogReflect");
	Light2TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogReflect");
	Light3TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogReflect");

	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	WorldViewProjTex  = mFX->GetVariableByName("gWorldViewProjTex")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	ShadowTransform   = mFX->GetVariableByName("gShadowTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor          = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart          = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange          = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights         = mFX->GetVariableByName("gDirLights");
	Mat               = mFX->GetVariableByName("gMaterial");
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap           = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	ShadowMap         = mFX->GetVariableByName("gShadowMap")->AsShaderResource();
	SsaoMap           = mFX->GetVariableByName("gSsaoMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region NormalMapEffect
NormalMapEffect::NormalMapEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech    = mFX->GetTechniqueByName("Light1");
	Light2Tech    = mFX->GetTechniqueByName("Light2");
	Light3Tech    = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech    = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech    = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech    = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	Light1ReflectTech    = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech    = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech    = mFX->GetTechniqueByName("Light3Reflect");

	Light0TexReflectTech = mFX->GetTechniqueByName("Light0TexReflect");
	Light1TexReflectTech = mFX->GetTechniqueByName("Light1TexReflect");
	Light2TexReflectTech = mFX->GetTechniqueByName("Light2TexReflect");
	Light3TexReflectTech = mFX->GetTechniqueByName("Light3TexReflect");

	Light0TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipReflect");
	Light1TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipReflect");
	Light2TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipReflect");
	Light3TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipReflect");

	Light1FogReflectTech    = mFX->GetTechniqueByName("Light1FogReflect");
	Light2FogReflectTech    = mFX->GetTechniqueByName("Light2FogReflect");
	Light3FogReflectTech    = mFX->GetTechniqueByName("Light3FogReflect");

	Light0TexFogReflectTech = mFX->GetTechniqueByName("Light0TexFogReflect");
	Light1TexFogReflectTech = mFX->GetTechniqueByName("Light1TexFogReflect");
	Light2TexFogReflectTech = mFX->GetTechniqueByName("Light2TexFogReflect");
	Light3TexFogReflectTech = mFX->GetTechniqueByName("Light3TexFogReflect");

	Light0TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogReflect");
	Light1TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogReflect");
	Light2TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogReflect");
	Light3TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogReflect");

	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	WorldViewProjTex  = mFX->GetVariableByName("gWorldViewProjTex")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	ShadowTransform   = mFX->GetVariableByName("gShadowTransform")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor          = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart          = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange          = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights         = mFX->GetVariableByName("gDirLights");
	Mat               = mFX->GetVariableByName("gMaterial");
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap           = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	NormalMap         = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	ShadowMap         = mFX->GetVariableByName("gShadowMap")->AsShaderResource();
	SsaoMap           = mFX->GetVariableByName("gSsaoMap")->AsShaderResource();
}

NormalMapEffect::~NormalMapEffect()
{
}
#pragma endregion

#pragma region BuildShadowMapEffect
BuildShadowMapEffect::BuildShadowMapEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	BuildShadowMapTech           = mFX->GetTechniqueByName("BuildShadowMapTech");
	BuildShadowMapAlphaClipTech  = mFX->GetTechniqueByName("BuildShadowMapAlphaClipTech");

	TessBuildShadowMapTech           = mFX->GetTechniqueByName("TessBuildShadowMapTech");
	TessBuildShadowMapAlphaClipTech  = mFX->GetTechniqueByName("TessBuildShadowMapAlphaClipTech");
	
	ViewProj          = mFX->GetVariableByName("gViewProj")->AsMatrix();
	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	HeightScale       = mFX->GetVariableByName("gHeightScale")->AsScalar();
	MaxTessDistance   = mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance   = mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	MinTessFactor     = mFX->GetVariableByName("gMinTessFactor")->AsScalar();
	MaxTessFactor     = mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap         = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
}

BuildShadowMapEffect::~BuildShadowMapEffect()
{
}
#pragma endregion

#pragma region SsaoNormalDepthEffect
SsaoNormalDepthEffect::SsaoNormalDepthEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	NormalDepthTech          = mFX->GetTechniqueByName("NormalDepth");
	NormalDepthAlphaClipTech = mFX->GetTechniqueByName("NormalDepthAlphaClip");

	WorldView             = mFX->GetVariableByName("gWorldView")->AsMatrix();
	WorldInvTransposeView = mFX->GetVariableByName("gWorldInvTransposeView")->AsMatrix();
	WorldViewProj         = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	TexTransform          = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	DiffuseMap            = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

SsaoNormalDepthEffect::~SsaoNormalDepthEffect()
{
}
#pragma endregion

#pragma region SsaoEffect
SsaoEffect::SsaoEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	SsaoTech           = mFX->GetTechniqueByName("Ssao");

	ViewToTexSpace     = mFX->GetVariableByName("gViewToTexSpace")->AsMatrix();
	OffsetVectors      = mFX->GetVariableByName("gOffsetVectors")->AsVector();
	FrustumCorners     = mFX->GetVariableByName("gFrustumCorners")->AsVector();
	OcclusionRadius    = mFX->GetVariableByName("gOcclusionRadius")->AsScalar();
	OcclusionFadeStart = mFX->GetVariableByName("gOcclusionFadeStart")->AsScalar();
	OcclusionFadeEnd   = mFX->GetVariableByName("gOcclusionFadeEnd")->AsScalar();
	SurfaceEpsilon     = mFX->GetVariableByName("gSurfaceEpsilon")->AsScalar();

	NormalDepthMap     = mFX->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	RandomVecMap       = mFX->GetVariableByName("gRandomVecMap")->AsShaderResource();
}

SsaoEffect::~SsaoEffect()
{
}
#pragma endregion

#pragma region SsaoBlurEffect
SsaoBlurEffect::SsaoBlurEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	HorzBlurTech    = mFX->GetTechniqueByName("HorzBlur");
	VertBlurTech    = mFX->GetTechniqueByName("VertBlur");

	TexelWidth      = mFX->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight     = mFX->GetVariableByName("gTexelHeight")->AsScalar();

	NormalDepthMap  = mFX->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	InputImage      = mFX->GetVariableByName("gInputImage")->AsShaderResource();
}

SsaoBlurEffect::~SsaoBlurEffect()
{
}
#pragma endregion

#pragma region SkyEffect
SkyEffect::SkyEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	SkyTech       = mFX->GetTechniqueByName("SkyTech");
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap       = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
}

SkyEffect::~SkyEffect()
{
}
#pragma endregion

#pragma region DebugTexEffect
DebugTexEffect::DebugTexEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	ViewArgbTech  = mFX->GetTechniqueByName("ViewArgbTech");
	ViewRedTech   = mFX->GetTechniqueByName("ViewRedTech");
	ViewGreenTech = mFX->GetTechniqueByName("ViewGreenTech");
	ViewBlueTech  = mFX->GetTechniqueByName("ViewBlueTech");
	ViewAlphaTech = mFX->GetTechniqueByName("ViewAlphaTech");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	Texture       = mFX->GetVariableByName("gTexture")->AsShaderResource();
}

DebugTexEffect::~DebugTexEffect()
{

}

#pragma endregion

#pragma region Effects

BasicEffect*           Effects::BasicFX           = 0;
NormalMapEffect*       Effects::NormalMapFX       = 0;
BuildShadowMapEffect*  Effects::BuildShadowMapFX  = 0;
SsaoNormalDepthEffect* Effects::SsaoNormalDepthFX = 0;
SsaoEffect*            Effects::SsaoFX            = 0;
SsaoBlurEffect*        Effects::SsaoBlurFX        = 0;
SkyEffect*             Effects::SkyFX             = 0;
DebugTexEffect*        Effects::DebugTexFX        = 0;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX           = new BasicEffect(device, L"FX/Basic.fxo");
	NormalMapFX       = new NormalMapEffect(device, L"FX/NormalMap.fxo");
	BuildShadowMapFX  = new BuildShadowMapEffect(device, L"FX/BuildShadowMap.fxo");
	SsaoNormalDepthFX = new SsaoNormalDepthEffect(device, L"FX/SsaoNormalDepth.fxo");
	SsaoFX            = new SsaoEffect(device, L"FX/Ssao.fxo");
	SsaoBlurFX        = new SsaoBlurEffect(device, L"FX/SsaoBlur.fxo");
	SkyFX             = new SkyEffect(device, L"FX/Sky.fxo");
	DebugTexFX        = new DebugTexEffect(device, L"FX/DebugTexture.fxo");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(NormalMapFX);
	SafeDelete(BuildShadowMapFX);
	SafeDelete(SsaoNormalDepthFX);
	SafeDelete(SsaoFX);
	SafeDelete(SsaoBlurFX);
	SafeDelete(SkyFX);
	SafeDelete(DebugTexFX);
}

#pragma endregion