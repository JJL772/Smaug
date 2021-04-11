#pragma once
#include "glm/vec3.hpp"
#include <vector>

/*

Half Edge data structure
http://sccg.sk/~samuelcik/dgs/half_edge.pdf

V=vertex
E=vertex.edge.vertex
N=vertex.edge.next.vertex
P=vertex.edge.next.next.pair.next.vertex

     E
    / \
   V---N
    \ /
     P
	 

*/

// Might be worth turning this whole thing into a class of sorts?
// It's a little bit messy currently...


struct halfEdge_t;
struct face_t;
struct mesh_t;

struct vertex_t
{
	// Should refer back to the face's verts
	glm::vec3* vert = nullptr;

	// The edge that stems out of this vert
	halfEdge_t* edge = nullptr;
};


struct halfEdge_t
{
	// This edge stems out of a vertex. vert is the vertex at the end of this edge.
	vertex_t* vert = nullptr;

	// This edge is half of another edge. This is the other side of the edge located on another face.
	halfEdge_t* pair = nullptr;

	// The face this edge belongs to.
	face_t* face = nullptr;

	// Next edge. Same as vert->edge.
	halfEdge_t* next = nullptr;
};


struct meshPart_t;
struct face_t
{
	~face_t();

	// These edges use the verts 
	std::vector<halfEdge_t*> edges;

	std::vector<vertex_t*> verts;

	// What part of the mesh we belong to.
	meshPart_t* meshPart = nullptr;
};

// Axis Aligned Bounding Box. Absolute minimum and maximum of a shape
struct aabb_t
{
	glm::vec3 min;
	glm::vec3 max;
};

// A mesh part is a face that is part of a larger mesh that holds more faces
struct meshPart_t : public face_t
{
	~meshPart_t();

	// Triangles of the mesh. Use for geo testing and etc.
	std::vector<face_t*> fullFaces;

	// Cut faces of the mesh. This is what gets rendered. Use faces if size = 0. 
	std::vector<face_t*> cutFaces;
	
	bool isCut = false;

	// Calculated from face verts
	//aabb_t aabb;

	// What mesh do we belong to
	mesh_t* mesh = nullptr;
};

struct mesh_t
{
	~mesh_t();

	// The faces of this mesh
	std::vector<meshPart_t*> parts;
	
	// Not ordered! Do no depend on this!
	// Pointers so that adding doesn't delete the memory
	std::vector<glm::vec3*> verts;

	// Calculated from part aabbs
	//aabb_t aabb;

	// Transform of the mesh
	glm::vec3 origin;
};

// A mesh that can be sliced by other meshes
struct cuttableMesh_t : public mesh_t
{

	// These cut into this mesh without changing the real structure of it
	std::vector<mesh_t*> cutters;

	// We cut these meshes
	std::vector<mesh_t*> cutting;

	// Not ordered! Do no depend on this!
	// Pointers so that adding doesn't delete the memory
	// Added during cutting
	std::vector<glm::vec3*> cutVerts;
};


// Returns start of the points within mesh.verts
glm::vec3** addMeshVerts(mesh_t& mesh, glm::vec3* points, int pointCount);

// Does not add points to mesh! Only adds face
void addMeshFace(mesh_t& mesh, glm::vec3** points, int pointCount);

void defineMeshPartFaces(meshPart_t& mesh);


aabb_t meshAABB(mesh_t& mesh);
void recenterMesh(mesh_t& mesh);
glm::vec3 faceCenter(face_t* face);
glm::vec3 faceNormal(face_t* face, glm::vec3* outCenter = nullptr);

void faceFromLoop(halfEdge_t* startEdge, face_t* faceToFill);


void cloneFaceInto(face_t* in, face_t* cloneOut);