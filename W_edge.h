#include <Eigen/Core>
#include <string>
#include <vector>

using namespace std;
using Eigen::Vector3f;
using Eigen::Vector3i;
using Eigen::Vector4f;
using Eigen::Matrix4f;
using Eigen::MatrixXf;

#ifndef W_EDGE_H
#define W_EDGE_H

struct W_edge
{
	struct Vertex* start; Vertex* end;
	struct Face* left; Face* right;
	W_edge* left_prev; W_edge* left_next;
	W_edge* right_prev; W_edge* right_next;

	// The left W_edge
	W_edge* leftW_edge() {
        return left_prev->right_next;
    }
	void PairLeftW_edge(W_edge * leftW_edge);
};

struct Vertex
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Vector3f p;
	W_edge *edge;
	// Used in decimation
	Matrix4f q;

    vector<Face*> getFaces();
    vector<W_edge*> getAllW_edges();
	vector<W_edge*> getAllW_edgesStart();
	int countJointNeighbourVertices(Vertex* v2);
    int countFaces();
};

struct Face
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	W_edge *edge;

	vector<W_edge*> getW_edges();
	vector<Vertex*> getVertices();
	Vector3f getNormal();
};

#endif //W_EDGE_H
