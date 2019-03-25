#include <nanogui/common.h>
#include <string>
#include <vector>

using namespace std;
using nanogui::Vector3f;
using nanogui::Vector3i;
using nanogui::Vector4f;
using nanogui::Matrix4f;
using nanogui::MatrixXf;

#ifndef W_EDGE_H
#define W_EDGE_H

const float PI = 3.14159265359;
const float MAXVALUE = 99999999;

struct W_edge
{
	struct Vertex* start; Vertex* end;
	struct Face* left; Face* right;
	W_edge* left_prev; W_edge* left_next;
	W_edge* right_prev; W_edge* right_next;
	// Used in Subdivision
	Vector3f* edgeVertex = NULL;

	// The left W_edge
	W_edge* leftW_edge() {
        return left_prev->right_next;
    }
	void PairLeftW_edge(W_edge * leftW_edge);
	Matrix4f getQ();
	Vector4f getTargetV();
	void toNull();
	bool isNull();
	// Detect if fold over will occur by collapsing this edge
	bool detectFoldOver();
};

struct Vertex
{
	Vector3f p;
	W_edge *edge;
	// Used in decimation
	Matrix4f q;

    vector<Face*> getFaces();
    vector<W_edge*> getAllW_edges();
	vector<W_edge*> getAllW_edgesStart();
	int countJointNeighbourVertices(Vertex* v2);
    int countFaces();
	void setInitialQ();
};

struct Face
{
	W_edge *edge;

	vector<W_edge*> getW_edges();
	vector<Vertex*> getVertices();
	Vector3f getNormal();
	Vector3f getNewNormal(Vertex* v, Vector3f newP);
	bool detectFoldOver(Vertex* v, Vector3f newP);
	Matrix4f getK_p();
};

#endif //W_EDGE_H
