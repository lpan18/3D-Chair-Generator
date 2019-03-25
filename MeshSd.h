#include <nanogui/common.h>
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

#ifndef MESHSD_H
#define MESHSD_H

class MeshSd : public Mesh {
public:
	MeshSd(string fileName)
    : Mesh(fileName) {
	}

	MeshSd(ObjBuffer buffer)
    : Mesh(buffer) {
	}

    // Loop subdivision
	ObjBuffer sdLoop();
	// Butterfly subdivision
	ObjBuffer sdBtfl();
};

#endif // MESHSD_H