#include "LoadM3d.h"
 
bool M3DLoader::LoadM3d(const std::string& filename, 
						std::vector<Vertex::PosNormalTexTan>& vertices,
						std::vector<USHORT>& indices,
						std::vector<MeshGeometry::Subset>& subsets,
						std::vector<M3dMaterial>& mats)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices  = 0;
	UINT numTriangles = 0;
	UINT numBones     = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if( fin )
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;
 
		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
	    ReadVertices(fin, numVertices, vertices);
	    ReadTriangles(fin, numTriangles, indices);
 
		return true;
	 }
    return false;
}

void M3DLoader::ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats)
{
	 std::string ignore;
     mats.resize(numMaterials);

	 std::string diffuseMapName;
	 std::string normalMapName;

     fin >> ignore; // materials header text
	 for(UINT i = 0; i < numMaterials; ++i)
	 {
			fin >> ignore >> mats[i].Mat.Ambient.x  >> mats[i].Mat.Ambient.y  >> mats[i].Mat.Ambient.z;
			fin >> ignore >> mats[i].Mat.Diffuse.x  >> mats[i].Mat.Diffuse.y  >> mats[i].Mat.Diffuse.z;
			fin >> ignore >> mats[i].Mat.Specular.x >> mats[i].Mat.Specular.y >> mats[i].Mat.Specular.z;
			fin >> ignore >> mats[i].Mat.Specular.w;
			fin >> ignore >> mats[i].Mat.Reflect.x >> mats[i].Mat.Reflect.y >> mats[i].Mat.Reflect.z;
			fin >> ignore >> mats[i].AlphaClip;
			fin >> ignore >> mats[i].EffectTypeName;
			fin >> ignore >> diffuseMapName;
			fin >> ignore >> normalMapName;

			mats[i].DiffuseMapName.resize(diffuseMapName.size(), ' ');
			mats[i].NormalMapName.resize(normalMapName.size(), ' ');
			std::copy(diffuseMapName.begin(), diffuseMapName.end(), mats[i].DiffuseMapName.begin());
			std::copy(normalMapName.begin(), normalMapName.end(), mats[i].NormalMapName.begin());
		}
}

void M3DLoader::ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets)
{
    std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // subset header text
	for(UINT i = 0; i < numSubsets; ++i)
	{
        fin >> ignore >> subsets[i].Id;
		fin >> ignore >> subsets[i].VertexStart;
		fin >> ignore >> subsets[i].VertexCount;
		fin >> ignore >> subsets[i].FaceStart;
		fin >> ignore >> subsets[i].FaceCount;
    }
}

void M3DLoader::ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices)
{
	std::string ignore;
    vertices.resize(numVertices);

    fin >> ignore; // vertices header text
    for(UINT i = 0; i < numVertices; ++i)
    {
	    fin >> ignore >> vertices[i].Pos.x      >> vertices[i].Pos.y      >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> vertices[i].TangentU.w;
	    fin >> ignore >> vertices[i].Normal.x   >> vertices[i].Normal.y   >> vertices[i].Normal.z;
	    fin >> ignore >> vertices[i].Tex.x      >> vertices[i].Tex.y;
    }
}

void M3DLoader::ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices)
{
	std::string ignore;
    indices.resize(numTriangles*3);

    fin >> ignore; // triangles header text
    for(UINT i = 0; i < numTriangles; ++i)
    {
        fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
    }
}
 