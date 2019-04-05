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
	buffer.setCenterAndScale();

	return buffer;
}

ObjBuffer ObjBuffer::combineObjBuffers(vector<ObjBuffer> objBuffers) {
	ObjBuffer buffer;
	// First, get vCount and fCount and initialize buffer
	int vCount = 0;
	int fCount = 0;
	for (auto b : objBuffers) {
		vCount += b.nVertices;
		fCount += b.mFaces;
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
		
		for (int i = 0; i < b.nVertices; i++) {
			buffer.vertices[vi] = b.vertices[i];
			vi++;
		}
		
		for (int i = 0; i < b.mFaces; i++) {
			buffer.faces[fi] = b.faces[i] + Vector3i(viOffset, viOffset, viOffset);
			fi++;
		}
	}

	buffer.setCenterAndScale();
	
	return buffer;
}

ObjBuffer ObjBuffer::getGroup(string groupName) {
	ObjBuffer buffer;
	buffer.center = center;
	buffer.scale = scale;

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

	return buffer;
}

void ObjBuffer::free() {
	delete []vertices;
	delete []faces;
}

void ObjBuffer::setCenterAndScale() {
	ObjBound bound = getBound();
	center = bound.getCenter();
	scale = bound.getScale();
}

ObjBound ObjBuffer::getBound() {
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

	ObjBound bound;
	bound.maxX = maxX;
	bound.maxY = maxY;
	bound.maxZ = maxZ;
	bound.minX = minX;
	bound.minY = minY;
	bound.minZ = minZ;

	return bound;
}

ChairPartBuffer ChairPartBuffer::fromSeat(ObjBuffer seat) {
	ChairPartBuffer seat1;
	seat1.nVertices = seat.nVertices;
	seat1.mFaces = seat.mFaces;
	seat1.center = seat.center;
	seat1.scale = seat.scale;
	seat1.vertices = seat.vertices;
	seat1.faces = seat.faces;

	ObjBound bound = seat1.getBound();
	Vector3f center = bound.getCenter();
	
	seat1.backCenter = Vector3f(center.x(), bound.maxY, bound.minZ);
	seat1.topCenter = Vector3f(center.x(), center.y(), bound.minZ);
	seat1.bottomCenter = Vector3f(center.x(), center.y(), bound.maxZ);
	seat1.width = bound.maxX - bound.minX;
	seat1.depth = bound.maxY - bound.minY;

	return seat1;
}

ChairPartBuffer ChairPartBuffer::fromPart(ObjBuffer part, ChairPartBuffer seat) {
	ChairPartBuffer part1;
	part1.nVertices = part.nVertices;
	part1.mFaces = part.mFaces;
	part1.center = part.center;
	part1.scale = part.scale;
	part1.vertices = part.vertices;
	part1.faces = part.faces;

	part1.backCenter = seat.backCenter;
	part1.topCenter = seat.topCenter;
	part1.bottomCenter = seat.bottomCenter;
	part1.width = seat.width;
	part1.depth = seat.depth;

	return part1;
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