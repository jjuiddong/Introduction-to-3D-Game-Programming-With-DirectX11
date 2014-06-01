//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dUtil.h"

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region AmbientOcclusionEffect
class AmbientOcclusionEffect : public Effect
{
public:
	AmbientOcclusionEffect(ID3D11Device* device, const std::wstring& filename);
	~AmbientOcclusionEffect();

	void SetWorldViewProj(CXMMATRIX M)  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	ID3DX11EffectTechnique* AmbientOcclusionTech;
	ID3DX11EffectMatrixVariable* WorldViewProj;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static AmbientOcclusionEffect* AmbientOcclusionFX;
};
#pragma endregion

#endif // EFFECTS_H