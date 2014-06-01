#ifndef LOADM3D_H
#define LOADM3D_H

#include "MeshGeometry.h"
#include "LightHelper.h"
#include "Vertex.h"

struct M3dMaterial
{
	Material Mat;
	bool AlphaClip;
	std::string EffectTypeName;
	std::wstring DiffuseMapName;
	std::wstring NormalMapName;
};

class M3DLoader
{
public:
	bool LoadM3d(const std::string& filename, 
		std::vector<Vertex::PosNormalTexTan>& vertices,
		std::vector<USHORT>& indices,
		std::vector<MeshGeometry::Subset>& subsets,
		std::vector<M3dMaterial>& mats);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
};

#endif // LOADM3D_H