#include <Eigen/Core>
#include <string>
#include "W_edge.h"

using namespace std;
using Eigen::Vector3f;
using Eigen::Vector3i;
using Eigen::Vector4f;
using Eigen::Matrix4f;
using Eigen::MatrixXf;

#ifndef MESH_H
#define MESH_H

class ObjGroup
{
public:
	string name;
	int vStart;
	int vEnd;
	int fStart;
	int fEnd;
};

// An in-memory representation of obj file
class ObjBuffer
{
public:
	int nVertices;
	int mFaces;
	Vector3f center;
	float scale;

	Vector3f* vertices;
	Vector3i* faces;
	vector<ObjGroup> groups;

    // Read obj file
	static ObjBuffer readObjFile(string filename);
	ObjBuffer getGroup(string groupName);
	void setCenterAndScale();
};

// Base class of mesh
class Mesh
{
public:
	Mesh(ObjBuffer buffer) {
		readObjBuffer(buffer);
		constructLeft();
	}
	~Mesh() {
		delete[] vertices;
		delete[] faces;
		delete[] w_edges;
	}
	// Get mesh vertex positions
	MatrixXf getPositions();
	// Get normals for flat shading
	MatrixXf getNormals(MatrixXf* positions);
	// Get normals for smooth shading
	MatrixXf getSmoothNormals(MatrixXf* normals);
	// Get colors
	MatrixXf getColors();
	// Get vertex count
	int getVertexCount() {
		return nVertices;
	}
	// Write mesh to an obj file
	void writeObj(string fileName);
protected:
    // Number of vertices
	int nVertices = 0;
	// Number of faces
	int mFaces = 0;
	// Number of W_edges
	int lW_edges = 0;

	// Pointer to vertex array
	Vertex* vertices;
	// Pointer to face array
	Face* faces;
	// Pointer to w_edge array
	W_edge* w_edges;
    
	// Center of the mesh
	Vector3f center;
	// Scale to be multiplied to the original mesh
	float scale;
	
	// Read obj buffer
	void readObjBuffer(ObjBuffer buffer);
	// Fill in left parameters (left_prev, left_next, and left) of W_edge
	void constructLeft();
	// Get vertex normals for smooth shading
	Vector3f getVertexSN(Vertex* v, MatrixXf* normals);
};
#endif //MESH_H
