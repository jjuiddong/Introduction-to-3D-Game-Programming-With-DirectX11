//***************************************************************************************
// Octree.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Octree.h"


Octree::Octree()
	: mRoot(0)
{
}

Octree::~Octree()
{
	SafeDelete(mRoot);
}

void Octree::Build(const std::vector<XMFLOAT3>& vertices, const std::vector<UINT>& indices)
{
	// Cache a copy of the vertices.
	mVertices = vertices;

	// Build AABB to contain the scene mesh.
	XNA::AxisAlignedBox sceneBounds = BuildAABB();
	
	// Allocate the root node and set its AABB to contain the scene mesh.
	mRoot = new OctreeNode();
	mRoot->Bounds = sceneBounds;

	BuildOctree(mRoot, indices);
}

bool Octree::RayOctreeIntersect(FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	return RayOctreeIntersect(mRoot, rayPos, rayDir);
}

XNA::AxisAlignedBox Octree::BuildAABB()
{
	XMVECTOR vmin = XMVectorReplicate(+MathHelper::Infinity);
	XMVECTOR vmax = XMVectorReplicate(-MathHelper::Infinity);
	for(size_t i = 0; i < mVertices.size(); ++i)
	{
		XMVECTOR P = XMLoadFloat3(&mVertices[i]);

		vmin = XMVectorMin(vmin, P);
		vmax = XMVectorMax(vmax, P);
	}

	XNA::AxisAlignedBox bounds;
	XMVECTOR C = 0.5f*(vmin + vmax);
	XMVECTOR E = 0.5f*(vmax - vmin); 

	XMStoreFloat3(&bounds.Center, C); 
	XMStoreFloat3(&bounds.Extents, E); 

	return bounds;
}

void Octree::BuildOctree(OctreeNode* parent, const std::vector<UINT>& indices)
{
	size_t triCount = indices.size() / 3;

	if(triCount < 60) 
	{
		parent->IsLeaf = true;
		parent->Indices = indices;
	}
	else
	{
		parent->IsLeaf = false;

		XNA::AxisAlignedBox subbox[8];
		parent->Subdivide(subbox);

		for(int i = 0; i < 8; ++i)
		{
			// Allocate a new subnode.
			parent->Children[i] = new OctreeNode();
			parent->Children[i]->Bounds = subbox[i];

			// Find triangles that intersect this node's bounding box.
			std::vector<UINT> intersectedTriangleIndices;
			for(size_t j = 0; j < triCount; ++j)
			{
				UINT i0 = indices[j*3+0];
				UINT i1 = indices[j*3+1];
				UINT i2 = indices[j*3+2];

				XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
				XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
				XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

				if(XNA::IntersectTriangleAxisAlignedBox(v0, v1, v2, &subbox[i]))
				{
					intersectedTriangleIndices.push_back(i0);
					intersectedTriangleIndices.push_back(i1);
					intersectedTriangleIndices.push_back(i2);
				}
			}

			// Recurse.
			BuildOctree(parent->Children[i], intersectedTriangleIndices);
		}
	}
}

bool Octree::RayOctreeIntersect(OctreeNode* parent, FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	// Recurs until we find a leaf node (all the triangles are in the leaves).
	if( !parent->IsLeaf )
	{
		for(int i = 0; i < 8; ++i)
		{
			// Recurse down this node if the ray hit the child's box.
			float t;
			if( XNA::IntersectRayAxisAlignedBox(rayPos, rayDir, &parent->Children[i]->Bounds, &t) )
			{
				// If we hit a triangle down this branch, we can bail out that we hit a triangle.
				if( RayOctreeIntersect(parent->Children[i], rayPos, rayDir) )
					return true;
			}
		}

		// If we get here. then we did not hit any triangles.
		return false;
	}
	else
	{
		size_t triCount = parent->Indices.size() / 3;

		for(size_t i = 0; i < triCount; ++i)
		{
			UINT i0 = parent->Indices[i*3+0];
			UINT i1 = parent->Indices[i*3+1];
			UINT i2 = parent->Indices[i*3+2];

			XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
			XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
			XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

			float t;
			if( XNA::IntersectRayTriangle(rayPos, rayDir, v0, v1, v2, &t) )
				return true;
		}

		return false;
	}
}