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

struct ObjBound {
	float maxX;
	float maxY;
	float maxZ;
	float minX;
	float minY;
	float minZ;

	Vector3f getCenter() {
		return Vector3f(maxX / 2.0f + minX / 2.0f, maxY / 2.0f + minY / 2.0f, maxZ / 2.0f + minZ / 2.0f);
	}

	float getScale() {
		Vector3f center = getCenter();
		Vector3f maxOffset = Vector3f(maxX, maxY, maxZ) - center;
	    float scale = 1.0f / maxOffset.maxCoeff();
		return scale;
	}
};

struct ObjGroup {
	string name;
	int vStart;
	int vEnd;
	int fStart;
	int fEnd;
};

// An in-memory representation of obj file
struct ObjBuffer {
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
	// Release vertices and faces
	void free();
	// Reset center and scale of this ObjBuffer
	void setCenterAndScale();
	// Get ObjBound
	ObjBound getBound();
};

struct ChairPartBuffer : ObjBuffer {
	Vector3f backCenter;
	Vector3f topCenter;
	Vector3f bottomCenter;
	float width;
	float depth;

	static ChairPartBuffer fromSeat(ObjBuffer seat);
	static ChairPartBuffer fromPart(ObjBuffer part, ChairPartBuffer seat);
};

struct ChairBuffer {
	ObjBuffer chair;
	ChairPartBuffer seat;
	ChairPartBuffer leg;
	ChairPartBuffer back;
	ChairPartBuffer arm;

	static ChairBuffer readObjFile(string filename);
    // Release buffers
	void free();
};
#endif // OBJBUFFER_H