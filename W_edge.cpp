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

// Pair this W_edge with its left W_edge.
void W_edge::PairLeftW_edge(W_edge *leftW_edge) {
	leftW_edge->left = right;
	leftW_edge->left_prev = right_prev;
	leftW_edge->left_next = right_next;

	left = leftW_edge->right;
	left_prev = leftW_edge->right_prev;
	left_next = leftW_edge->right_next;
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