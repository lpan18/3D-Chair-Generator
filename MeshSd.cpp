#include "MeshSd.h"

// Loop Subdivision, update the positions of existing vertices
Vector3f sdLoopVertex(Vertex* v) {
    Vector3f vec(0, 0, 0);
	int k = v->countFaces();
	// Pre-calculate beta to improve efficiency
	float beta = 1.0f / k * (5.0f / 8.0f - pow(3.0f / 8.0f + 1.0f / 4.0f * cos(2.0f * PI / k), 2));
	float q = 1 - k * beta;

	vec += v->p * q;
	W_edge *e0 = v->edge->end == v ? v->edge->leftW_edge() : v->edge;
	W_edge *edge = e0;
	do {
		if (edge->end == v) {
			vec += edge->start->p * beta;
			edge = edge->right_next;
		} else {
			vec += edge->end->p * beta;
			edge = edge->left_next;
		}
	} while (edge != e0);

	return vec;
}

// Loop Subdivision, new vertices at edges
Vector3f sdLoopEdge(W_edge* w_edge) {
	Vector3f vec(0, 0, 0);
	vec += 3.0f / 8.0f * w_edge->start->p;
	vec += 3.0f / 8.0f * w_edge->end->p;
	vec += 1.0f / 8.0f * w_edge->right_next->end->p;
	vec += 1.0f / 8.0f * w_edge->left_next->end->p;

	return vec;
}

// Butterfly Subdivision, both sides regular
Vector3f sdBtflEdgeBothRegular(W_edge* w_edge) {
	Vector3f vec(0, 0, 0);
	vec += 1.0f / 2.0f * w_edge->start->p;
	vec += 1.0f / 2.0f * w_edge->end->p;
	vec += 1.0f / 8.0f * w_edge->right_next->end->p;
	vec += 1.0f / 8.0f * w_edge->left_next->end->p;
	vec += -1.0f / 16.0f * w_edge->right_next->left_next->end->p;
	vec += -1.0f / 16.0f * w_edge->right_prev->left_next->end->p;
	vec += -1.0f / 16.0f * w_edge->left_next->left_next->end->p;
	vec += -1.0f / 16.0f * w_edge->left_prev->left_next->end->p;
	
	return vec;
}

float getS(int j, int k) {
	if (j >= k) throw "Invalid j";
	if (k <= 4) throw "Invalid k";

	return 1.0f / k * (0.25f + cos(2.0f * j * PI / k) + 0.5f * cos(4.0f * j * PI / k));
}

// Butterfly Subdivision, start vertex regular
Vector3f sdBtflEdgeStartRegular(W_edge* w_edge, int k) {
	Vector3f vec(0, 0, 0);

	if (k == 3) {
		// v0
		vec += 5.0f / 12.0f * w_edge->start->p;
		// v1
		vec += -1.0f / 12.0f * w_edge->right_next->end->p;
		// v2
		vec += -1.0f / 12.0f * w_edge->right_next->left_next->end->p;
		// vq
		vec += 3.0f / 4.0f * w_edge->end->p;
	} else if (k == 4) {
		// v0
		vec += 3.0f / 8.0f * w_edge->start->p;
		// v1 is 0
		// v2
		vec += -1.0f / 8.0f * w_edge->right_next->left_next->end->p;
		// v3 is 0
		// vq
		vec += 3.0f / 4.0f * w_edge->end->p;
	} else {
		float s = 0;
		float sum_s = 0;

		s = getS(0, k);
		vec += s * w_edge->start->p;
		sum_s += s;

		W_edge* btflEdge = w_edge->right_next;
		for (int j = 1; j < k; j++) {
			s = getS(j, k);
			vec += s * btflEdge->end->p;
			sum_s += s;

			btflEdge = btflEdge->left_next;
		}

		float q = 1 - sum_s;
		vec += q * w_edge->end->p;
	}
	return vec;
}

// Butterfly Subdivision, new vertices at edges
Vector3f sdBtflEdge(W_edge* w_edge) {
	Vector3f vec;
    int startCount = w_edge->start->countFaces();
	int endCount = w_edge->end->countFaces();

	if (startCount == 6 && endCount == 6) {
		vec = sdBtflEdgeBothRegular(w_edge);
	} else if (startCount == 6) {
		vec = sdBtflEdgeStartRegular(w_edge, endCount);
	} else if (endCount == 6) {
		vec = sdBtflEdgeStartRegular(w_edge->leftW_edge(), startCount);
	} else {
		vec = sdBtflEdgeStartRegular(w_edge, endCount) / 2.0f + sdBtflEdgeStartRegular(w_edge->leftW_edge(), startCount) / 2.0f;
	}

	return vec;
}

// Loop subdivision
ObjBuffer MeshSd::sdLoop() {
	ObjBuffer sd;
	sd.nVertices = nVertices + lW_edges / 2;
	sd.mFaces = mFaces * 4;
	sd.center = center;
	sd.scale = scale;
	sd.vertices = new Vector3f[sd.nVertices];
	sd.faces = new Vector3i[sd.mFaces];

	int vi = 0;
	for (; vi < nVertices; vi++) {
		sd.vertices[vi] = sdLoopVertex(vertices + vi);
	}

	for (int j = 0; j < lW_edges; j++) {
		if (w_edges[j].edgeVertex == NULL) {
			sd.vertices[vi] = sdLoopEdge(w_edges + j);
			w_edges[j].edgeVertex = sd.vertices + vi;
			w_edges[j].leftW_edge()->edgeVertex = sd.vertices + vi;
			vi++;
		}
	}

	int v1, v2, v3, v4, v5, v6;
	for (int k = 0; k < mFaces; k++) {
		v1 = faces[k].edge->start - vertices + 1;
		v2 = faces[k].edge->end - vertices + 1;
		v3 = faces[k].edge->right_next->end - vertices + 1;
		v4 = faces[k].edge->edgeVertex - sd.vertices + 1;
		v5 = faces[k].edge->right_next->edgeVertex - sd.vertices + 1;
		v6 = faces[k].edge->right_prev->edgeVertex - sd.vertices + 1;

		sd.faces[k * 4] << v1, v6, v4;
		sd.faces[k * 4 + 1] << v3, v5, v6;
		sd.faces[k * 4 + 2] << v2, v4, v5;
		sd.faces[k * 4 + 3] << v4, v6, v5;
	}

	return sd;
}

// Butterfly subdivision
ObjBuffer MeshSd::sdBtfl() {
	ObjBuffer sd;
	sd.nVertices = nVertices + lW_edges / 2;
	sd.mFaces = mFaces * 4;
	sd.center = center;
	sd.scale = scale;
	sd.vertices = new Vector3f[sd.nVertices];
	sd.faces = new Vector3i[sd.mFaces];

	int vi = 0;
	for (; vi < nVertices; vi++) {
		sd.vertices[vi] = vertices[vi].p;
	}

	for (int j = 0; j < lW_edges; j++) {
		if (w_edges[j].edgeVertex == NULL) {
			sd.vertices[vi] = sdBtflEdge(w_edges + j);
			w_edges[j].edgeVertex = sd.vertices + vi;
			w_edges[j].leftW_edge()->edgeVertex = sd.vertices + vi;
			vi++;
		}
	}

	int v1, v2, v3, v4, v5, v6;
	for (int k = 0; k < mFaces; k++) {
		v1 = faces[k].edge->start - vertices + 1;
		v2 = faces[k].edge->end - vertices + 1;
		v3 = faces[k].edge->right_next->end - vertices + 1;
		v4 = faces[k].edge->edgeVertex - sd.vertices + 1;
		v5 = faces[k].edge->right_next->edgeVertex - sd.vertices + 1;
		v6 = faces[k].edge->right_prev->edgeVertex - sd.vertices + 1;

		sd.faces[k * 4] << v1, v6, v4;
		sd.faces[k * 4 + 1] << v3, v5, v6;
		sd.faces[k * 4 + 2] << v2, v4, v5;
		sd.faces[k * 4 + 3] << v4, v6, v5;
	}

	return sd;
}