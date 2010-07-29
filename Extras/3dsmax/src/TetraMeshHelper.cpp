#include "TetraMeshHelper.h"
#include <stdio.h>


TetraMeshHelper::TetraMeshHelper() 
{
	clear();
}

void TetraMeshHelper::clear()
{
	mTetraLinks.clear();
	mBounds.setEmpty();
}

// to attach to tetra mesh
void TetraMeshHelper::buildTetraLinks(const NxSoftBodyMeshDesc& desc)
{
	const NxVec3 *vertices = (const NxVec3 *) desc.vertices;
	const void *tetIndices = desc.tetrahedra;
	bool is32Bits = !(desc.flags & NX_SOFTBODY_MESH_16_BIT_INDICES);
	const NxU32 numTets = desc.numTetrahedra;

	mTetraLinks.clear();

	MeshHash* hash = new MeshHash();

	// hash tetrahedra for faster search
	hash->setGridSpacing(mBounds.min.distance(mBounds.max) * 0.1f);

	NxU8* startIndex = (NxU8*) tetIndices;
	if(is32Bits)
	{
		for (NxU32 i = 0; i < numTets; i++) {
			const NxU32 *ix = ((NxU32*) startIndex) + 4*i;
			NxBounds3 tetraBounds;
			tetraBounds.setEmpty();
			tetraBounds.include(vertices[*ix++]);
			tetraBounds.include(vertices[*ix++]);
			tetraBounds.include(vertices[*ix++]);
			tetraBounds.include(vertices[*ix++]);
			hash->add(tetraBounds, i);
		}
		for (NxU32 i = 0; i < maxMesh.numVerts; i++) 
		{
			MeshTetraLink tmpLink;

			NxVec3 triVert(maxMesh.verts[i].x, maxMesh.verts[i].y, maxMesh.verts[i].z);
			std::vector<int> itemIndices;
			hash->queryUnique(triVert, itemIndices);

			NxReal minDist = 0.0f;
			NxVec3 b;
			int num, isize;
			num = isize = itemIndices.size();
			if (num == 0) num = numTets;

			for (int i = 0; i < num; i++) {
				int j = i;
				if (isize > 0) j = itemIndices[i];

				const NxU32 *ix = ((NxU32*) startIndex) + 4*j;
				const NxVec3 &p0 = vertices[*ix++];
				const NxVec3 &p1 = vertices[*ix++];
				const NxVec3 &p2 = vertices[*ix++];
				const NxVec3 &p3 = vertices[*ix++];

				NxVec3 b = computeBaryCoords(triVert, p0, p1, p2, p3);

				// is the vertex inside the tetrahedron? If yes we take it
				if (b.x >= 0.0f && b.y >= 0.0f && b.z >= 0.0f && (b.x + b.y + b.z) <= 1.0f) {
					tmpLink.barycentricCoords = b;
					tmpLink.tetraNr = j;
					break;
				}

				// otherwise, if we are not in any tetrahedron we take the closest one
				NxReal dist = 0.0f;
				if (b.x + b.y + b.z > 1.0f) dist = b.x + b.y + b.z - 1.0f;
				if (b.x < 0.0f) dist = (-b.x < dist) ? dist : -b.x;
				if (b.y < 0.0f) dist = (-b.y < dist) ? dist : -b.y;
				if (b.z < 0.0f) dist = (-b.z < dist) ? dist : -b.z;

				if (i == 0 || dist < minDist) {
					minDist = dist;
					tmpLink.barycentricCoords = b;
					tmpLink.tetraNr = j;
				}
			}

			mTetraLinks.push_back(tmpLink);
		}
	}
	else
	{
		for (NxU32 i = 0; i < numTets; i++) {
			const NxU16 *ix = ((NxU16*) startIndex) + 4*i;
			NxBounds3 tetraBounds;
			tetraBounds.setEmpty();
			tetraBounds.include(vertices[*ix++]);
			tetraBounds.include(vertices[*ix++]);
			tetraBounds.include(vertices[*ix++]);
			tetraBounds.include(vertices[*ix++]);
			hash->add(tetraBounds, i);
		}
		for (NxU32 i = 0; i < maxMesh.numVerts; i++) 
		{
			MeshTetraLink tmpLink;

			NxVec3 triVert(maxMesh.verts[i].x, maxMesh.verts[i].y, maxMesh.verts[i].z);
			std::vector<int> itemIndices;
			hash->queryUnique(triVert, itemIndices);

			NxReal minDist = 0.0f;
			NxVec3 b;
			int num, isize;
			num = isize = itemIndices.size();
			if (num == 0) num = numTets;

			for (int i = 0; i < num; i++) {
				int j = i;
				if (isize > 0) j = itemIndices[i];

				const NxU16 *ix = ((NxU16*) startIndex) + 4*j;
				const NxVec3 &p0 = vertices[*ix++];
				const NxVec3 &p1 = vertices[*ix++];
				const NxVec3 &p2 = vertices[*ix++];
				const NxVec3 &p3 = vertices[*ix++];

				NxVec3 b = computeBaryCoords(triVert, p0, p1, p2, p3);

				// is the vertex inside the tetrahedron? If yes we take it
				if (b.x >= 0.0f && b.y >= 0.0f && b.z >= 0.0f && (b.x + b.y + b.z) <= 1.0f) {
					tmpLink.barycentricCoords = b;
					tmpLink.tetraNr = j;
					break;
				}

				// otherwise, if we are not in any tetrahedron we take the closest one
				NxReal dist = 0.0f;
				if (b.x + b.y + b.z > 1.0f) dist = b.x + b.y + b.z - 1.0f;
				if (b.x < 0.0f) dist = (-b.x < dist) ? dist : -b.x;
				if (b.y < 0.0f) dist = (-b.y < dist) ? dist : -b.y;
				if (b.z < 0.0f) dist = (-b.z < dist) ? dist : -b.z;

				if (i == 0 || dist < minDist) {
					minDist = dist;
					tmpLink.barycentricCoords = b;
					tmpLink.tetraNr = j;
				}
			}

			mTetraLinks.push_back(tmpLink);
		}
	}

	delete hash;
}

void TetraMeshHelper::updateBounds()
{
	mBounds.setEmpty();
	for (int i = 0; i < maxMesh.numVerts; i++)
	{
		NxVec3 v(maxMesh.verts[i].x, maxMesh.verts[i].y, maxMesh.verts[i].z);
		mBounds.include(v);
	}
}

bool TetraMeshHelper::updateTetraLinks(const NxMeshData &tetraMeshData)
{
	if (mTetraLinks.size() != maxMesh.numVerts) return false;

	NxU32 numVertices = *tetraMeshData.numVerticesPtr;
	NxU32 numTetrahedra = *tetraMeshData.numIndicesPtr / 4;
	const NxVec3 *vertices = (NxVec3*)tetraMeshData.verticesPosBegin;

	NxU8* startIndex = (NxU8*)tetraMeshData.indicesBegin;

	for (int i = 0; i < maxMesh.numVerts; i++) {
		MeshTetraLink &link = mTetraLinks[i];
		const NxU32 *ix = ((NxU32*) startIndex) + 4*link.tetraNr;
		const NxVec3 &p0 = vertices[*ix++];
		const NxVec3 &p1 = vertices[*ix++];
		const NxVec3 &p2 = vertices[*ix++];
		const NxVec3 &p3 = vertices[*ix++];

		NxVec3 &b = link.barycentricCoords;
		NxVec3 v = p0 * b.x + p1 * b.y + p2 * b.z + p3 * (1.0f - b.x - b.y - b.z);
		maxMesh.verts[i].x = v.x;
		maxMesh.verts[i].y = v.y;
		maxMesh.verts[i].z = v.z;
	}
	return true;
}

// ----------------------------------------------------------------------
// computes barycentric coordinates
NxVec3 TetraMeshHelper::computeBaryCoords(NxVec3 vertex, NxVec3 p0, NxVec3 p1, NxVec3 p2, NxVec3 p3)
{
	NxVec3 baryCoords;

	NxVec3 q  = vertex-p3;
	NxVec3 q0 = p0-p3;
	NxVec3 q1 = p1-p3;
	NxVec3 q2 = p2-p3;

	NxMat33 m;
	m.setColumn(0,q0);
	m.setColumn(1,q1);
	m.setColumn(2,q2);

	NxReal det = m.determinant();

	m.setColumn(0, q);
	baryCoords.x = m.determinant();

	m.setColumn(0, q0); m.setColumn(1,q);
	baryCoords.y = m.determinant();

	m.setColumn(1, q1); m.setColumn(2,q);
	baryCoords.z = m.determinant();

	if (det != 0.0f)
		baryCoords /= det;

	return baryCoords;
}

void TetraMeshHelper::setMesh(const Mesh& max)
{
	clear();
	maxMesh = max;
	updateBounds();
}
