#include <string>
#include <vector>
#include <Eigen/Geometry>
#include "Constants.h"

using namespace std;
using Eigen::Vector3f;
using Eigen::Vector3i;
using Eigen::Vector4f;
using Eigen::Matrix3f;
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
	Vector3f* vertices;
	Vector3i* faces;

	vector<ObjGroup> groups;

	ObjBound bound;

    // Read obj file
	static ObjBuffer readObjFile(string filename);
	// Combine multiple ObjBuffers
	static ObjBuffer combineObjBuffers(vector<ObjBuffer*> objBuffers);
	// Generate a new ObjBuffer for group groupName
	ObjBuffer getGroup(string groupName);
	// Release vertices and faces
	void free();
	// Reset ObjBound
	void resetBound();
	// Get the closest point to p
	Vector3f getClosestPointTo(Vector3f p);
	// Get the closest point from p to face f
	Vector3f getClosesPoint(Vector3i f, Vector3f p);
};

struct ChairPartOrigSeatFeatures {
	Vector3f backTopCenter;
	Vector3f topCenter;
	Vector3f bottomCenter;
	float width;
	float depth;

	static ChairPartOrigSeatFeatures fromSeat(ObjBuffer& seat);
	static Vector3f transform(Matrix3f scale, Vector3f v, Vector3f oldBase, Vector3f newBase);
};

struct ChairPartFeatures {
	Vector3f topRightBack;
	Vector3f topRightFront;
	Vector3f topLeftFront;
	Vector3f topLeftBack;

	Vector3f bottomRightBack;
	Vector3f bottomRightFront;
	Vector3f bottomLeftFront;
	Vector3f bottomLeftBack;
};

struct ChairPartBuffer : ObjBuffer {
	ChairPartOrigSeatFeatures origSeatFeatures;
	ChairPartFeatures partFeatures;

	static ChairPartBuffer fromSeat(ObjBuffer& seat);
	static ChairPartBuffer fromPart(ObjBuffer& part, ChairPartBuffer& seat);
	void resetPartFeatures();
		Vector3f getTranslated(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f v);
	void singleTranslation(Vector3f pb, Vector3f p0, Vector3f p1);
	void doubleTranslation(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f q0, Vector3f q1);
	Matrix3f getScaleMatrix(Vector3f pb, Vector3f p0, Vector3f p1);
	void singleScale(Vector3f pb, Vector3f p0, Vector3f p1);
	// Inputs: p0.y() > q0.y()
	void doubleScale(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f q0, Vector3f q1);
	void align(Vector3f p_target);
private:
	Vector3f getFeature(float x, float y, float z);
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