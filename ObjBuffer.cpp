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

void ObjBuffer::destroy() {
	delete []vertices;
	delete []faces;
}

void ObjBuffer::setCenterAndScale() {
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

	center = Vector3f(maxX / 2.0f + minX / 2.0f, maxY / 2.0f + minY / 2.0f, maxZ / 2.0f + minZ / 2.0f);
	Vector3f maxOffset = Vector3f(maxX, maxY, maxZ) - center;
	scale = 1.0f / maxOffset.maxCoeff();
}