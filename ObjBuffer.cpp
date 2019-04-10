#include <Eigen/Geometry>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ObjBuffer.h"

float clamp(float x, float low, float high)
{
    return min(high, max(x, low));
}

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

float getDist(Vector3f vt, Vector3f p){
	Vector3f dvt = vt - p;
	return dvt.x() * dvt.x() + dvt.y() * dvt.y() + dvt.z() * dvt.z();
}

// TO DO
// Temporary implementation.
// To be updated to http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.104.4264&rep=rep1&type=pdf
Vector3f ObjBuffer::getClosestPointTo(Vector3f p) {
	float minDistQuad = MAXVALUE;
	Vector3f pc;
	for (int i = 0; i < nVertices; i++) {
		float distQuad = getDist(vertices[i], p);
		if (distQuad < minDistQuad) {
			minDistQuad = distQuad;
			pc = vertices[i];
		}
	}

	for (int i = 0; i < mFaces; i++) {
		Vector3i f = faces[i];
		Vector3f vt0 = vertices[f[0] - 1];
		Vector3f vt1 = vertices[f[1] - 1];
		Vector3f vt2 = vertices[f[2] - 1];	
		Vector3f vt_center = (vt0 + vt1 + vt2) / 3.0f;			
		float dist_center = getDist(vt_center, p);
		if (dist_center < minDistQuad) {
			minDistQuad = dist_center;
			pc = vt_center;
		}
	}
	// for (int i = 0; i < mFaces; i++) {
	// 	Vector3f p1 = getClosesPoint(faces[i], p);
	// 	Vector3f p1p = p - p1;
	// 	if(p.x() != 0 && p.y() != 0 && p.z() != 0){
	// 		float distQuad = p1p.x() * p1p.x() + p1p.y() * p1p.y() + p1p.z() * p1p.z();
	// 		if (distQuad < minDistQuad) {
	// 			pc = p1;
	// 		}
	// 	}
	// }
	return pc;
}

// https://math.stackexchange.com/questions/544946/determine-if-projection-of-3d-point-onto-plane-is-within-a-triangle/544947
// Vector3f ObjBuffer::getClosesPoint(Vector3i f, Vector3f p)
// {
// 	Vector3f vt0 = vertices[f[0] - 1];
// 	Vector3f vt1 = vertices[f[1] - 1];
// 	Vector3f vt2 = vertices[f[2] - 1];
//     Vector3f u = vt1 - vt0;
//     Vector3f v = vt2 - vt0;
//     Vector3f n = u.cross(v);
//     Vector3f w = p - vt0;
//     // Barycentric coordinates of the projection P′of P onto T:
//     float gamma = u.cross(w).dot(n) / (n.dot(n));
//     float beta = w.cross(v).dot(n) / (n.dot(n));
//     float alpha = 1 - gamma - beta;
// 	cout << "alpha " << alpha << endl;
// 	cout << "beta " << beta << endl;
// 	cout << "gamma " << gamma << endl;
//     // The point P′ lies inside T if:
//     bool isInside = ((0 <= alpha) && (alpha <= 1) &&
//             (0 <= beta)  && (beta  <= 1) &&
//             (0 <= gamma) && (gamma <= 1));
// 	if(isInside){
// 		cout << "isInside" << endl;
// 		return alpha * vt0 + beta * vt1 + gamma * vt2;
// 	}else{
// 		cout << "outSide" << endl;
// 		return Vector3f(0,0,0);
// 	}
// }

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
		error = abs(v.x() - x) + 2 * abs(v.y() - y) + 3 * abs(v.z() - z);
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

Vector3f ChairPartBuffer::getTransformed(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f v) {
	Vector3f offsetbase = p1 - p0;
	float v1x = v.x() + offsetbase.x();
	float v1y = v.y() + offsetbase.y();
	float scaleZ = (p1.z() - pb.z()) / (p0.z() - pb.z());
	float v1z = (v.z() - pb.z()) * scaleZ  + pb.z();
	return Vector3f(v1x, v1y, v1z);
}

void ChairPartBuffer::transformSingle(Vector3f pb, Vector3f p0, Vector3f p1) {
	for (int i = 0; i < nVertices; i++) {
		vertices[i] = getTransformed(pb, p0, p1, vertices[i]);
	}
}

Vector3f ChairPartBuffer::getTransformedXSym(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f v) {
	Vector3f offsetbase = p1 - p0;
	float v1x = v.x() + (pb.x() - v.x()) / (pb.x() - p0.x()) * offsetbase.x();
	float v1y = v.y() + offsetbase.y();
	float scaleZ = (p1.z() - pb.z()) / (p0.z() - pb.z());
	float v1z = (v.z() - pb.z()) * scaleZ  + pb.z();
	return Vector3f(v1x, v1y, v1z);
}

void ChairPartBuffer::transformSingleXSym(Vector3f pb, Vector3f p0, Vector3f p1) {
	for (int i = 0; i < nVertices; i++) {
		vertices[i] = getTransformedXSym(pb, p0, p1, vertices[i]);
	}
}

void ChairPartBuffer::transformDouleXSym(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f q0, Vector3f q1) {
	for (int i = 0; i < nVertices; i++) {
		Vector3f vp = getTransformedXSym(pb, p0, p1, vertices[i]);
		Vector3f vq = getTransformedXSym(pb, q0, q1, vertices[i]);
		float wp = vertices[i].y() > p0.y() ? 1.0f :
		            vertices[i].y() < q0.y() ? 0.0f :
					(vertices[i].y() - q0.y()) / (p0.y() - q0.y());
					
		vertices[i] = vp * wp + vq * (1.0f - wp);
	}
}

void ChairPartBuffer::align(Vector3f p_target) {
	Vector3f pb(bound.getCenter().x(), bound.getCenter().y(), bound.getCenter().z());
	float offsetX = p_target.x() - pb.x();
	float offsetY = p_target.y() - pb.y();
	for (int i = 0; i < nVertices; i++) {
		vertices[i].x() += offsetX;
		vertices[i].y() += offsetY;
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