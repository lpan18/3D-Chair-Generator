#include <Eigen/Geometry>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ObjBuffer.h"

// Read Obj file to ObjBuffer
ObjBuffer ObjBuffer::readObjFile(string filename) {
	string line;
	int vn = 0, fm = 0;

	// Read the file and get the numbers of vertices and faces.
	ifstream fin(filename);
	while (getline(fin, line)) {
		if (line.length() > 1) {
			if (line[0] == 'v' && line[1] == ' ') {
				vn++;
			}
			else if (line[0] == 'f' && line[1] == ' ') {
				fm++;
			}
		}
	}

	ObjBuffer buffer;
	buffer.nVertices = vn;
	buffer.mFaces = fm;
	buffer.vertices = new Vector3f[buffer.nVertices];
	buffer.faces = new Vector3i[buffer.mFaces];

	// read the file again.
	ifstream fin1(filename);
	int vi = 0, fi = 0;
	float x, y, z;
	int v1, v2, v3;

	while (getline(fin1, line)) {
		if (line.length() > 0) {
			if (line[0] == 'v' && line[1] == ' ') {
				string str = line.substr(2, line.size() - 1);
				istringstream iss(str);
				iss >> x >> y >> z;
				buffer.vertices[vi] = Vector3f(x, y, z);
				vi++;
			} else if (line[0] == 'f' && line[1] == ' ') {
				string str = line.substr(2, line.size() - 1);
				istringstream iss(str);
				iss >> v1 >> v2 >> v3;
				buffer.faces[fi] = Vector3i(v1, v2, v3);
				fi++;
			} else if (line.rfind("# Starting mesh", 0) == 0) {
				ObjGroup group;
				group.name = line.substr(16, line.size() - 17);
				transform(group.name.begin(), group.name.end(), group.name.begin(), ::tolower);
				group.vStart = vi;
				buffer.groups.push_back(group);
			} else if (line.rfind("g ", 0) == 0) {
				buffer.groups.back().vEnd = vi - 1;
				buffer.groups.back().fStart = fi;
			} else if (line.rfind("# End of mesh", 0) == 0) {
				buffer.groups.back().fEnd = fi - 1;
			}
		}
	}

	// Set Center and Scale
	buffer.resetBound();

	return buffer;
}

ObjBuffer ObjBuffer::combineObjBuffers(vector<ObjBuffer*> objBuffers) {
	ObjBuffer buffer;
	// First, get vCount and fCount and initialize buffer
	int vCount = 0;
	int fCount = 0;
	for (auto b : objBuffers) {
		vCount += b->nVertices;
		fCount += b->mFaces;
	}

	buffer.nVertices = vCount;
	buffer.mFaces = fCount;
	buffer.vertices = new Vector3f[vCount];
	buffer.faces = new Vector3i[fCount];

	// Iterate objBuffers again for vertices and faces
	int vi = 0;
	int fi = 0;
	for (auto b : objBuffers) {
		int viOffset = vi;
		
		for (int i = 0; i < b->nVertices; i++) {
			buffer.vertices[vi] = b->vertices[i];
			vi++;
		}
		
		for (int i = 0; i < b->mFaces; i++) {
			buffer.faces[fi] = b->faces[i] + Vector3i(viOffset, viOffset, viOffset);
			fi++;
		}
	}

	// Reset vertices and faces for objBuffers
	Vector3f* vCurrent = buffer.vertices;
	Vector3i* fCurrent = buffer.faces;
	for (auto b : objBuffers) {
		b->vertices = vCurrent;
		b->faces = fCurrent;

		vCurrent += b->nVertices;
		fCurrent += b->mFaces;
	}

	buffer.resetBound();
	
	return buffer;
}

ObjBuffer ObjBuffer::getGroup(string groupName) {
	ObjBuffer buffer;

	// First, get vCount and fCount and initialize buffer
	int vCount = 0;
	int fCount = 0;
	for (auto g : groups) {
		if (g.name.rfind(groupName, 0) == 0) {
			vCount += g.vEnd - g.vStart + 1;
			fCount += g.fEnd - g.fStart + 1;
		}
	}

	buffer.nVertices = vCount;
	buffer.mFaces = fCount;
	buffer.vertices = new Vector3f[vCount];
	buffer.faces = new Vector3i[fCount];

	// Iterate groups again for vertices and faces
	int vi = 0;
	int fi = 0;
	for (auto g : groups) {
		if (g.name.rfind(groupName, 0) == 0) {
			int viOffset = g.vStart - vi;
			for (int i = g.vStart; i <= g.vEnd; i++) {
				buffer.vertices[vi] = vertices[i];
				vi++;
			}

			for (int i = g.fStart; i <= g.fEnd; i++) {
				buffer.faces[fi] = faces[i] - Vector3i(viOffset, viOffset, viOffset);
				fi++;
			}
		}
	}

	buffer.resetBound();

	return buffer;
}

void ObjBuffer::free() {
	delete []vertices;
	delete []faces;
}

void ObjBuffer::resetBound() {
	float maxX, maxY, maxZ;
	float minX, minY, minZ;
	maxX = maxY = maxZ = -MAXVALUE;
	minX = minY = minZ = MAXVALUE;

	for (int i = 0; i < nVertices; i++) {
		maxX = vertices[i].x() > maxX ? vertices[i].x() : maxX;
		maxY = vertices[i].y() > maxY ? vertices[i].y() : maxY;
		maxZ = vertices[i].z() > maxZ ? vertices[i].z() : maxZ;
		minX = vertices[i].x() < minX ? vertices[i].x() : minX;
		minY = vertices[i].y() < minY ? vertices[i].y() : minY;
		minZ = vertices[i].z() < minZ ? vertices[i].z() : minZ;
	}

	bound.maxX = maxX;
	bound.maxY = maxY;
	bound.maxZ = maxZ;
	bound.minX = minX;
	bound.minY = minY;
	bound.minZ = minZ;
}

// TO DO
// Temporary implementation.
// To be updated to http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.104.4264&rep=rep1&type=pdf
Vector3f ObjBuffer::getClosestPointTo(Vector3f p1) {
	float minDistQuad = MAXVALUE;
	Vector3f p2;
	for (int i = 0; i < nVertices; i++) {
	    float distQuad = vertices[i].x() * p1.x() + vertices[i].y() * p1.y() + vertices[i].z() * p1.z();
		if (distQuad < minDistQuad) {
			minDistQuad = distQuad;
			p2 = vertices[i];
		}
	}
	return p2;
}

ChairPartOrigSeatFeatures ChairPartOrigSeatFeatures::fromSeat(ObjBuffer& seat) {
	seat.resetBound();
	ChairPartOrigSeatFeatures features;
	ObjBound bound = seat.bound;
	Vector3f center = bound.getCenter();
	
	features.backTopCenter = Vector3f(center.x(), bound.maxY, bound.maxZ);
	features.topCenter = Vector3f(center.x(), center.y(), bound.maxZ);
	features.bottomCenter = Vector3f(center.x(), center.y(), bound.minZ);
	features.width = bound.maxX - bound.minX;
	features.depth = bound.maxY - bound.minY;

	return features;
}

Vector3f ChairPartOrigSeatFeatures::transform(Matrix3f scale, Vector3f v, Vector3f oldBase, Vector3f newBase) {
    Vector3f offset = v - oldBase;
    return scale * offset + newBase;
}

ChairPartBuffer ChairPartBuffer::fromSeat(ObjBuffer& seat) {
	ChairPartBuffer seat1;
	seat1.nVertices = seat.nVertices;
	seat1.mFaces = seat.mFaces;
	seat1.vertices = seat.vertices;
	seat1.faces = seat.faces;
	seat1.bound = seat.bound;
	seat1.origSeatFeatures = ChairPartOrigSeatFeatures::fromSeat(seat1);
	seat1.resetPartFeatures();

	return seat1;
}

ChairPartBuffer ChairPartBuffer::fromPart(ObjBuffer& part, ChairPartBuffer& seat) {
	ChairPartBuffer part1;
	part1.nVertices = part.nVertices;
	part1.mFaces = part.mFaces;
	part1.vertices = part.vertices;
	part1.faces = part.faces;
	part1.bound = part.bound;
	part1.origSeatFeatures = seat.origSeatFeatures;
	part1.resetPartFeatures();

	return part1;
}

Vector3f ChairPartBuffer::getFeature(float x, float y, float z) {
	Vector3f feature;
	float minError = MAXVALUE;

	Vector3f v;
	float error;
	for (int i = 0; i < nVertices; i++) {
		v = vertices[i];
		error = abs(v.x() - x) + abs(v.y() - y) + 3 * abs(v.z() - z);
		if (error < minError) {
			feature = v;
			minError = error;
		}
	}

	return feature;
}

void ChairPartBuffer::resetPartFeatures() {
	resetBound();

	if (nVertices > 0) {
		partFeatures.topRightBack = getFeature(bound.minX, bound.maxY, bound.maxZ);
		partFeatures.topRightFront = getFeature(bound.minX, bound.minY, bound.maxZ);
		partFeatures.topLeftFront = getFeature(bound.maxX, bound.minY, bound.maxZ);
		partFeatures.topLeftBack = getFeature(bound.maxX, bound.maxY, bound.maxZ);

		partFeatures.bottomRightBack = getFeature(bound.minX, bound.maxY, bound.minZ);
		partFeatures.bottomRightFront = getFeature(bound.minX, bound.minY, bound.minZ);
		partFeatures.bottomLeftFront = getFeature(bound.maxX, bound.minY, bound.minZ);
		partFeatures.bottomLeftBack = getFeature(bound.maxX, bound.maxY, bound.minZ);
	} else {
		partFeatures.topRightBack =
		partFeatures.topRightFront =
		partFeatures.topLeftFront =
		partFeatures.topLeftBack =
		partFeatures.bottomRightBack =
		partFeatures.bottomRightFront =
		partFeatures.bottomLeftFront =
		partFeatures.bottomLeftBack = Vector3f::Zero();
	}
}

ChairBuffer ChairBuffer::readObjFile(string fileName) {
	ObjBuffer chair = ObjBuffer::readObjFile(fileName);
	ObjBuffer seat = chair.getGroup("seat");
	ObjBuffer leg = chair.getGroup("leg");
	ObjBuffer back = chair.getGroup("back");
	ObjBuffer arm = chair.getGroup("arm");

	ChairBuffer chairBuffer;
	chairBuffer.chair = chair;
	chairBuffer.seat = ChairPartBuffer::fromSeat(seat);
	chairBuffer.leg = ChairPartBuffer::fromPart(leg, chairBuffer.seat);
	chairBuffer.back = ChairPartBuffer::fromPart(back, chairBuffer.seat);
	chairBuffer.arm = ChairPartBuffer::fromPart(arm, chairBuffer.seat);

	return chairBuffer;
}

void ChairBuffer::free() {
	arm.free();
	back.free();
	leg.free();
	seat.free();
	chair.free();
}