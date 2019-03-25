#include "MeshMcd.h"

// Main routine for multiple-choice decimation
ObjBuffer MeshMcd::mcd(int k, int countCollapse) {
	if (k <= 0) throw "k must be great than 0";
	if (countCollapse >= lW_edges / 2) throw "countCollapse must be smaller than the number of edges";

	// Initialize Q for all vertices
	for (int i = 0; i < nVertices; i++) {
		vertices[i].setInitialQ();
	}

	// Initialize valid W_edges
	vector<W_edge*> validW_edges;
	validW_edges.reserve(lW_edges);
	for (int i = 0; i < lW_edges; i++) {
		validW_edges.push_back(w_edges + i);
	}
    
	// Initialize random number generator
	srand(12345678);

	// Main loop for mcd
	for (int i = 0; i < countCollapse; i++) {
		do {
			bool success = mcdOneStep(k, validW_edges);
			if (success) break;
		}
		while (true);
	}

	// Write the decimated mesh to an ObjBuffer
	// count of vertices after decimation
	int dvCount = 0;
	// count of faces after decimation
	int dfCount = 0;
	// An array that converts old vertex numbers to new vertex numbers
	int* dvNumber = new int[nVertices];
	for (int i = 0; i < nVertices; i++) {
		if (vertices[i].edge != NULL) {
			dvCount++;
			dvNumber[i] = dvCount;
		} else {
			dvNumber[i] = -1;
		}
	}
	for (int i = 0; i < mFaces; i++) {
		if (faces[i].edge != NULL) {
			dfCount++;
		}
	}

	ObjBuffer buffer;
	buffer.center = center;
	buffer.scale = scale;
	buffer.nVertices = dvCount;
	buffer.mFaces = dfCount;
	buffer.vertices = new Vector3f[buffer.nVertices];
	buffer.faces = new Vector3i[buffer.mFaces];

	int vi = 0;
	for (int i = 0; i < nVertices; i++) {
		if (vertices[i].edge != NULL) {
			buffer.vertices[vi] = vertices[i].p;
			vi++;
		}
	}

	int fi = 0;
	for (int i = 0; i < mFaces; i++) {
		if (faces[i].edge != NULL) {
			int v1 = dvNumber[faces[i].edge->start - vertices];
			int v2 = dvNumber[faces[i].edge->end - vertices];
			int v3 = dvNumber[faces[i].edge->right_next->end -vertices];
			buffer.faces[fi] = Vector3i(v3, v2, v1);
			fi++;
		}
	}

	return buffer;
}

// One step of multiple choice decimation
bool MeshMcd::mcdOneStep(int k, vector<W_edge*>& validW_edges) {
	// First, randomly select k elements from validW_edges and move them to the start of validW_edges.
	// Empty W_edges are removed when detected.
	for (int i = 0; i < k; i++) {
		// i must be smaller than validW_edges.size()
		if (i >= validW_edges.size()) {
			// This is very rare. It happens only when (the number of edges to collapse) + k > original number of vertices
			k = i;
			break;
		}

		do {
			int r = rand() % (validW_edges.size() - i) + i;
			W_edge* re = validW_edges[r];
			if (re->isNull()) {
				validW_edges.erase(validW_edges.begin() + r);
				continue;
			} else {
				if (i == r) {
					// DO NOTHING
				} else {
					// Switch the rth and ith elements in validW_edges
					W_edge* temp = validW_edges[r];
					validW_edges[r] = validW_edges[i];
					validW_edges[i] = temp;
				}
				break;
			}
		} while (true);
	}

	// Next, calculate the cost for each randomly selected edge and find the W_edge
	// with minimun collapse cost
	vector<float> errors;
	errors.reserve(k);
	for (int i = 0; i < k; i++) {
		nanogui::Vector4f vt = validW_edges[i]->getTargetV();
		float e = vt.transpose() * validW_edges[i]->getQ() * vt;
		errors.push_back(e);
	}
	int minEI = std::min_element(errors.begin(),errors.end()) - errors.begin();
	W_edge* me = validW_edges[minEI];

	// There must be exactly two joint neighbour vertices
	if (me->start->countJointNeighbourVertices(me->end) != 2) {
		return false;
	} else if (me->detectFoldOver()) {
		return false;
	} else {
		mcdCollapse(me);
		return true;
	}
}

// Collapse w_edge
void MeshMcd::mcdCollapse(W_edge* w_edge) {
	Vector3f vt = w_edge->getTargetV().head(3);
	Vertex* v1 = w_edge->start;
	Vertex* v2 = w_edge->end;
	Vertex* v3 = w_edge->right_next->end;
	Vertex* v4 = w_edge->left_next->end;
	Face* f1 = w_edge->right;
	Face* f2 = w_edge->left;
	W_edge* e1 = w_edge->right_prev->leftW_edge();
	W_edge* e2 = w_edge->right_next->leftW_edge();
	W_edge* e3 = w_edge->left_next->leftW_edge();
	W_edge* e4 = w_edge->left_prev->leftW_edge();
	
	// for all v1Edges, update v1 to v2
	for (auto e : v1->getAllW_edges()) {
		if (e->start == v1)
			e->start = v2;
		else if (e->end == v1)
		    e->end = v2;
		else
		    throw "not a W_edge of vertex v1";
	}

	// Re-pairing e1 with e2, and e3 with e4
	e1->PairLeftW_edge(e2);
	e3->PairLeftW_edge(e4);

	// Set W_edges on f1 and f2 to null
	for (auto e : f1->getW_edges()) {
		e->toNull();
	}
	for (auto e : f2->getW_edges()) {
		e->toNull();
	}

	// Udpate v1 and v2
	v2->p = vt;
	v2->edge = e2;
	v2->q = v2->q + v1->q;

	v1->p = Vector3f::Zero();
	v1->edge = NULL;

	v3->edge = e1;
	v4->edge = e4;

	// Update f1 and f2
	f1->edge = NULL;
	f2->edge = NULL;
}