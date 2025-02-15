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
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
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

	float getAbsScale() {
		float distX = maxX - minX;
		float distY = maxY - minY;
		float distZ = maxZ - minZ;
		float maxVal = std::sqrt(distX * distX + distY * distY + distZ * distZ);
	    float scale = 1.0f / maxVal;
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
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
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
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Vector3f backTopCenter;
	Vector3f topCenter;
	Vector3f bottomCenter;
	float width;
	float depth;

	static ChairPartOrigSeatFeatures fromSeat(ObjBuffer& seat);
	static Vector3f transform(Matrix3f scale, Vector3f v, Vector3f oldBase, Vector3f newBase);
};

struct ChairPartFeatures {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
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
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	ChairPartOrigSeatFeatures origSeatFeatures;
	ChairPartFeatures partFeatures;

	static ChairPartBuffer fromSeat(ObjBuffer& seat);
	static ChairPartBuffer fromPart(ObjBuffer& part, ChairPartBuffer& seat);
	void resetPartFeatures();
	Vector3f getTransformed(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f v);
	void transformSingle(Vector3f pb, Vector3f p0, Vector3f p1);
	Vector3f getTransformedXSym(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f v, bool whetherScaleZ = true);
	void transformSingleXSym(Vector3f pb, Vector3f p0, Vector3f p1);
	void transformDouleXSym(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f q0, Vector3f q1, bool whetherScaleZ = true);
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