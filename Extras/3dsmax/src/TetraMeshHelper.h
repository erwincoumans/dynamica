#ifndef TETRA_MESH_HELPER_H
#define TETRA_MESH_HELPER_H

#include "MeshHash.h"
#include <max.h>
#include <vector>
#include <iostream>

//------------------------------------------------------------------------------------
struct TetraMeshTriangle
{
public:
	void init() {
		vertexNr[0] = -1; vertexNr[1] = -1; vertexNr[2] = -1;
	}
	void set(int v0, int v1, int v2, int mat) {
		vertexNr[0] = v0; vertexNr[1] = v1; vertexNr[2] = v2;
	}
	bool containsVertexNr(int vNr) const {
		return vNr == vertexNr[0] || vNr == vertexNr[1] || vNr == vertexNr[2];
	}

	// representation
	int vertexNr[3];
};

//------------------------------------------------------------------------------------
struct MeshTetraLink
{
	int tetraNr;
	NxVec3 barycentricCoords;
};

class TetraMeshHelper
{
public:
	TetraMeshHelper();

	// to attach to tetra mesh
	void buildTetraLinks(const NxSoftBodyMeshDesc& mesh);

	void clear();

	void transform(const NxMat34 &a);

	void getBounds(NxBounds3 &bounds) const { bounds = mBounds; }

	void setMesh(const Mesh& pMax);
	Mesh& getMesh() { return maxMesh; }
	void updateBounds();
	bool updateTetraLinks(const NxMeshData &tetraMeshData);

private:

	NxVec3 computeBaryCoords(NxVec3 vertex, NxVec3 p0, NxVec3 p1, NxVec3 p2, NxVec3 p3);

	std::vector<MeshTetraLink> mTetraLinks;
	NxBounds3 mBounds;

	Mesh maxMesh;
};


#endif
