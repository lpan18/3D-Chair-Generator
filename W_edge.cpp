#include <cmath>
#include <Eigen/Geometry>
#include <iostream>

#include "W_edge.h"

using namespace std;

// v0, v1, and v2 are ordered clock-wise.
Vector3f calculateNormal(Vector3f v0, Vector3f v1, Vector3f v2) {
	Vector3f e1 = v2 - v0;
	Vector3f e2 = v1 - v0;
	Vector3f normal = (e1.cross(e2)).normalized();
	return normal;
}

// Set the value of f to [low, high].
float limitRange(float f, float low, float high) {
	return f < low ? low : f > high ? high : f;
}

// get the angle between two unit vectors v1 and v2
float getAngleUnitVectors(Vector3f v1, Vector3f v2) {
	float cosa = v1.dot(v2);
	cosa = limitRange(cosa, -1.0f, 1.0f);
	return acos(cosa);
}

// Pair this W_edge with its left W_edge.
void W_edge::PairLeftW_edge(W_edge *leftW_edge) {
	leftW_edge->left = right;
	leftW_edge->left_prev = right_prev;
	leftW_edge->left_next = right_next;

	left = leftW_edge->right;
	left_prev = leftW_edge->right_prev;
	left_next = leftW_edge->right_next;
}

Matrix4f W_edge::getQ() {
	return start->q + end->q;
}

// Calculate the target position that this edge will be collapsed to.
Vector4f W_edge::getTargetV() {
	Matrix4f q = getQ();
	Matrix4f drv;
	drv << q(0, 0), q(0, 1), q(0, 2), q(0, 3),
	       q(0, 1), q(1, 1), q(1, 2), q(1, 3),
		   q(0, 2), q(1, 2), q(2, 2), q(2, 3),
		   0, 0, 0, 1;
	Matrix4f drv_inv = drv.inverse();

	if (drv_inv.hasNaN()) {
		Vector4f sp = Vector4f(start->p.x(), start->p.y(), start->p.z(), 1.0f);
		Vector4f ep = Vector4f(end->p.x(), end->p.y(), end->p.z(), 1.0f);
		Vector4f mp = sp * 0.5f + ep * 0.5f;
		float se = sp.transpose() * q * sp;
		float ee = ep.transpose() * q * ep;
		float me = mp.transpose() * q * mp;

		if (se < ee && se < me) {
			return sp;
		} else if (ee < se && ee < me) {
			return ep;
		} else {
			return mp;
		}
	} else {
		return drv_inv * Vector4f(0, 0, 0, 1);
	}
}

// Mark this W_edge as null. For simplicity, only
// start and end are set to null.
void W_edge::toNull() {
	start = NULL;
	end = NULL;
}

// Check if this W_edge is null.
bool W_edge::isNull() {
	if (start == NULL && end == NULL) {
		return true;
	} else if (start != NULL && end != NULL) {
		return false;
	} else {
		throw "Invalid W_edge state. start and end must be both null, or not null.";
	}
}

// Detect if fold over will occur by collapsing this edge
bool W_edge::detectFoldOver() {
	Vector3f vt = getTargetV().head(3);

	vector<Face*> startFaces = start->getFaces();
	for (auto f : startFaces) {
		if (f != left && f != right
		&& f->detectFoldOver(start, vt)) {
			return true;
		}
	}

	vector<Face*> endFaces = end->getFaces();
	for (auto f : endFaces) {
		if (f != left && f != right
		&& f->detectFoldOver(end, vt)) {
			return true;
		}
	}

	return false;
}

vector<Face*> Vertex::getFaces() {
	vector<Face*> vec;
	W_edge *e0 = edge->end == this ? edge->leftW_edge() : edge;
	W_edge *e = e0;
    
	do {
		if (e->end == this) {
			vec.push_back(e->right);
			e = e->right_next;
		} else {
			vec.push_back(e->left);
			e = e->left_next;
		}
	} while (e != e0);

	if (vec.size() == 0) throw "No face at vertex v";

	return vec;
}

vector<W_edge*> Vertex::getAllW_edges() {
	vector<W_edge*> vec;
	W_edge *e0 = edge->end == this ? edge->leftW_edge() : edge;
	W_edge *e = e0;
    
	do {
		vec.push_back(e);
		vec.push_back(e->leftW_edge());
		if (e->end == this) {
			e = e->right_next;
		} else {
			e = e->left_next;
		}
	} while (e != e0);

	if (vec.size() == 0) throw "No edge at vertex v";

	return vec;
}

// Get all the W_edges who start from this vertex
vector<W_edge*> Vertex::getAllW_edgesStart() {
	vector<W_edge*> vec;
	W_edge *e0 = edge->end == this ? edge->leftW_edge() : edge;
	W_edge *e = e0;
    
	do {
		vec.push_back(e);
		e = e->left_next;
	} while (e != e0);

	if (vec.size() == 0) throw "No edge at vertex v";

	return vec;
}

// Count the total number of vertices who are neighbours of
// both this vertex and vertex v2
int Vertex::countJointNeighbourVertices(Vertex* v2) {
	int count = 0;
	for (auto v1e : this->getAllW_edges()) {
		for (auto v2e : v2->getAllW_edges()) {
			if (v1e->start == this && v2e->start == v2 && v1e->end == v2e->end) {
				count++;
			}
		}
	}
	return count;
}

// Set the initial value of Q
void Vertex::setInitialQ() {
	q = Matrix4f::Zero();
	vector<Face*> faces = getFaces();
	for (auto f : faces) {
		q += f->getK_p();
	}
}

// Number of faces at Vertex v
int Vertex::countFaces() {
	return (int)getFaces().size();
}

vector<W_edge*> Face::getW_edges() {
	vector<W_edge*> vec;
	W_edge *e = edge;

	do {
		vec.push_back(e);
		e = e->right_next;
	} while (e != edge);

	if (vec.size() < 3) throw "Less than 3 W_edges on face f";

	return vec;
}

vector<Vertex*> Face::getVertices() {
	vector<Vertex*> vec;
	W_edge *e = edge;

	do {
		vec.push_back(e->start);
		e = e->right_next;
	} while (e != edge);

	if (vec.size() < 3) throw "Less than 3 Vertices on face f";

	return vec;
}

Vector3f Face::getNormal() {
	vector<Vertex*> vertices = getVertices();
	return calculateNormal(vertices[0]->p, vertices[1]->p, vertices[2]->p);
}

// Get the new normal for this face when Vertex v moves to newP
Vector3f Face::getNewNormal(Vertex* v, Vector3f newP) {
	vector<Vertex*> vertices = getVertices();
	Vector3f v0 = vertices[0] == v ? newP : vertices[0]->p;
	Vector3f v1 = vertices[1] == v ? newP : vertices[1]->p;
	Vector3f v2 = vertices[2] == v ? newP : vertices[2]->p;
	return calculateNormal(v0, v1, v2);
}

bool Face::detectFoldOver(Vertex* v, Vector3f newP) {
	Vector3f n1 = getNormal();
	Vector3f n2 = getNewNormal(v, newP);
	float a = getAngleUnitVectors(n1, n2);
	return a > PI / 2 ? true : false;
}

Matrix4f Face::getK_p() {
	vector<Vertex*> vertices = getVertices();
	Vector3f normal = calculateNormal(vertices[0]->p, vertices[1]->p, vertices[2]->p);
	float d = (Vector3f::Zero() - vertices[0]->p).dot(normal);
	Vector4f p = Vector4f(normal.x(), normal.y(), normal.z(), d);
	return p * p.transpose();
}