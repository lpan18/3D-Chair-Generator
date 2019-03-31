#include <string>
#include <vector>
#include <Eigen/Geometry>
#include "Constants.h"

using namespace std;
using Eigen::Vector3f;
using Eigen::Vector3i;
using Eigen::Vector4f;
using Eigen::Matrix4f;
using Eigen::MatrixXf;

#ifndef OBJBUFFER_H
#define OBJBUFFER_H

struct ObjGroup
{
	string name;
	int vStart;
	int vEnd;
	int fStart;
	int fEnd;
};

// An in-memory representation of obj file
struct ObjBuffer
{
	int nVertices;
	int mFaces;
	Vector3f center;
	float scale;

	Vector3f* vertices;
	Vector3i* faces;
	vector<ObjGroup> groups;

    // Read obj file
	static ObjBuffer readObjFile(string filename);
	// Combine multiple ObjBuffers
	static ObjBuffer combineObjBuffers(vector<ObjBuffer> objBuffers);
	// Generate a new ObjBuffer for group groupName
	ObjBuffer getGroup(string groupName);
	// Delete vertices and faces
	void destroy();
	void setCenterAndScale();
};
#endif // OBJBUFFER_H