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

// TO DO
// Temporary implementation.
// To be updated to http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.104.4264&rep=rep1&type=pdf
Vector3f ObjBuffer::getClosestPointTo(Vector3f p) {
	float minDistQuad = MAXVALUE;
	Vector3f pc;
	for (int i = 0; i < mFaces; i++) {
		Vector3f p1 = getClosesPoint(faces[i], p);
		Vector3f p1p = p - p1;
		float distQuad = p1p.x() * p1p.x() + p1p.y() * p1p.y() + p1p.z() * p1p.z();
		if (distQuad < minDistQuad) {
			pc = p1;
		}
	}

	return pc;
}

Vector3f ObjBuffer::getClosesPoint(Vector3i f, Vector3f p)
{
	Vector3f vt0 = vertices[f[0] - 1];
	Vector3f vt1 = vertices[f[1] - 1];
	Vector3f vt2 = vertices[f[2] - 1];

    Vector3f edge0 = vt1 - vt0;
    Vector3f edge1 = vt2 - vt0;
    Vector3f v0 = vt0 - p;

    float a = edge0.dot( edge0 );
    float b = edge0.dot( edge1 );
    float c = edge1.dot( edge1 );
    float d = edge0.dot( v0 );
    float e = edge1.dot( v0 );

    float det = a*c - b*b;
    float s = b*e - c*d;
    float t = b*d - a*e;

    if ( s + t < det )
    {
        if ( s < 0.f )
        {
            if ( t < 0.f )
            {
                if ( d < 0.f )
                {
                    s = clamp( -d/a, 0.f, 1.f );
                    t = 0.f;
                }
                else
                {
                    s = 0.f;
                    t = clamp( -e/c, 0.f, 1.f );
                }
            }
            else
            {
                s = 0.f;
                t = clamp( -e/c, 0.f, 1.f );
            }
        }
        else if ( t < 0.f )
        {
            s = clamp( -d/a, 0.f, 1.f );
            t = 0.f;
        }
        else
        {
            float invDet = 1.f / det;
            s *= invDet;
            t *= invDet;
        }
    }
    else
    {
        if ( s < 0.f )
        {
            float tmp0 = b+d;
            float tmp1 = c+e;
            if ( tmp1 > tmp0 )
            {
                float numer = tmp1 - tmp0;
                float denom = a-2*b+c;
                s = clamp( numer/denom, 0.f, 1.f );
                t = 1-s;
            }
            else
            {
                t = clamp( -e/c, 0.f, 1.f );
                s = 0.f;
            }
        }
        else if ( t < 0.f )
        {
            if ( a+d > b+e )
            {
                float numer = c+e-b-d;
                float denom = a-2*b+c;
                s = clamp( numer/denom, 0.f, 1.f );
                t = 1-s;
            }
            else
            {
                s = clamp( -e/c, 0.f, 1.f );
                t = 0.f;
            }
        }
        else
        {
            float numer = c+e-b-d;
            float denom = a-2*b+c;
            s = clamp( numer/denom, 0.f, 1.f );
            t = 1.f - s;
        }
    }

    return vt0 + s * edge0 + t * edge1;
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

Matrix3f ChairPartBuffer::getScaleMatrix(Vector3f pb, Vector3f p0, Vector3f p1) {
	Vector3f v_b0 = p0 - pb;
	Vector3f v_b1 = p1 - pb;

	float x0 = v_b0.x();
	float y0 = v_b0.y();
	float z0 = v_b0.z();
	float x1 = v_b1.x();
	float y1 = v_b1.y();
	float z1 = v_b1.z();

    float sX = abs(x0) > EPSILON && abs(x1) > EPSILON ? x1 / x0 : 1;
    float sY = abs(y0) > EPSILON && abs(y1) > EPSILON ? y1 / y0 : 1;
    float sZ = abs(z0) > EPSILON && abs(z1) > EPSILON ? z1 / z0 : 1;

	Matrix3f scale;
	scale << sX, 0, 0,
	        0, sY, 0,
			0, 0, sZ;

	return scale;
}

void ChairPartBuffer::singleScale(Vector3f pb, Vector3f p0, Vector3f p1) {
	Matrix3f scale = getScaleMatrix(pb, p0, p1);
	for (int i = 0; i < nVertices; i++) {
		vertices[i] = scale * (vertices[i] - pb) + pb;
	}
}

void ChairPartBuffer::doubleScale(Vector3f pb, Vector3f p0, Vector3f p1, Vector3f q0, Vector3f q1) {
	Matrix3f scaleP = getScaleMatrix(pb, p0, p1);
	Matrix3f scaleQ = getScaleMatrix(pb, q0, q1);
	// Weight of P
	float wp;
	// Weight of Q
	float wq;
	// Vertex with P transformation
	Vector3f vp;
	// Vertex with Q transformation
	Vector3f vq;
	for (int i = 0; i < nVertices; i++) {
		if (vertices[i].y() > p0.y()) {
			wp = 1;
			wq = 0;
		} else if (vertices[i].y() < q0.y()) {
			wp = 0;
			wq = 1;
		} else {
			wp = (p0.y() - vertices[i].y()) / (p0.y() - q0.y());
			wq = 1 - wp;
		}
		vp = scaleP * (vertices[i] - pb) + pb;
		vq = scaleQ * (vertices[i] - pb) + pb;
		vertices[i] = wp * vp + wq * vq; 
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