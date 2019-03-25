#include <nanogui/common.h>
#include <iostream>
#include <string>
#include <vector>
#include "W_edge.h"
#include "Mesh.h"

using namespace std;
using nanogui::Vector3f;
using nanogui::Vector3i;
using nanogui::Vector4f;
using nanogui::Matrix4f;
using nanogui::MatrixXf;

#ifndef MESHMCD_H
#define MESHMCD_H

class MeshMcd : public Mesh {
public:
	MeshMcd(string fileName)
    : Mesh(fileName) {
	}
    
	MeshMcd(ObjBuffer buffer)
    : Mesh(buffer) {
	}

    // Multiple choices decimation
	ObjBuffer mcd(int k, int countCollapse);
private:
	// One step of multiple choices decimation
	// return true if successful, false if not
	bool mcdOneStep(int k, vector<W_edge*>& validW_edges);
	// Collapse a w_edge
	void mcdCollapse(W_edge* w_edge);
};

#endif // MESHMCD_H