#include <algorithm>
#include <bits/stdc++.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <Eigen/Geometry>

#include "Mesh.h"

using namespace std;

#define RAND  (rand() % 100) / 100
#define WIDTH 112
#define HEIGHT 112
#define DENSITY 8



// Function for sorting w_edges
bool sortByStartVertexThenByEndVertex(const W_edge* edge1, const W_edge* edge2) {
	if (edge1->start < edge2->start)
		return true;
	else if (edge1->start > edge2->start)
		return false;
	else
		return edge1->end < edge2->end;
}

// Get mesh vertex positions
MatrixXf Mesh::getPositions() {
	MatrixXf positions = MatrixXf(3, mFaces * 9);
	for (int i = 0; i < mFaces; i++) {
		positions.col(i * 3) << (faces[i].edge->end->p - center) * scale;
		positions.col(i * 3 + 1) << (faces[i].edge->start->p - center) * scale;
		positions.col(i * 3 + 2) << (faces[i].edge->right_prev->start->p - center) * scale;

		positions.col(mFaces * 3 + i * 6) << positions.col(i * 3) * 1.005;
		positions.col(mFaces * 3 + i * 6 + 1) << positions.col(i * 3 + 1) * 1.005;
		positions.col(mFaces * 3 + i * 6 + 2) << positions.col(i * 3 + 1) * 1.005;
		positions.col(mFaces * 3 + i * 6 + 3) << positions.col(i * 3 + 2) * 1.005;
		positions.col(mFaces * 3 + i * 6 + 4) << positions.col(i * 3 + 2) * 1.005;
		positions.col(mFaces * 3 + i * 6 + 5) << positions.col(i * 3) * 1.005;
	}
	return positions;
}

// Get normals for flat shading
MatrixXf Mesh::getNormals(MatrixXf* positions) {
	MatrixXf normals = MatrixXf(3, mFaces * 9);

	for (int i = 0; i < mFaces; i++) {
		Vector3f e1 = positions->col(i * 3 + 1) - positions->col(i * 3);
		Vector3f e2 = positions->col(i * 3 + 2) - positions->col(i * 3 + 1);
		Vector3f normal = (e1.cross(e2)).normalized();

		normals.col(i * 3) = normals.col(i * 3 + 1) = normals.col(i * 3 + 2) = normal;
		normals.col(mFaces * 3 + i * 6) = normals.col(mFaces * 3 + i * 6 + 1) = normals.col(mFaces * 3 + i * 6 + 2)
		= normals.col(mFaces * 3 + i * 6 + 3) = normals.col(mFaces * 3 + i * 6 + 4) = normals.col(mFaces * 3 + i * 6 + 5)
		= normal;
	}
	return normals;
}

// Get normals for smooth shading
MatrixXf Mesh::getSmoothNormals(MatrixXf* normals) {
	MatrixXf smoothNormals = MatrixXf(3, mFaces * 9);
	for (int i = 0; i < mFaces; i++) {
		Vertex* v1 = faces[i].edge->end;
		Vertex* v2 = faces[i].edge->start;
		Vertex* v3 = faces[i].edge->right_prev->start;

		smoothNormals.col(i * 3) << getVertexSN(v1, normals);
		smoothNormals.col(i * 3 + 1) << getVertexSN(v2, normals);
		smoothNormals.col(i * 3 + 2) << getVertexSN(v3, normals);
	}
	for (int i = mFaces * 3; i < mFaces * 9; i++) {
		smoothNormals.col(i) << normals->col(i);
	}
	return smoothNormals;
}

MatrixXf Mesh::getColors() {
	MatrixXf colors = MatrixXf(3, mFaces * 9);
	for (int i = 0; i < mFaces * 3; i++) {
		colors.col(i) << 1, 0, 0;
	}
	for (int i = mFaces * 3; i < mFaces * 9; i++) {
		colors.col(i) << 0, 0, 0;
	}
	return colors;
}

// Write mesh to an obj file
void Mesh::writeObj(string fileName) {
	stringstream ss;
	ss << "# " << nVertices << " " << mFaces << endl;
	for (int i = 0; i < nVertices; i++) {
		ss << "v " << vertices[i].p.x() << " " << vertices[i].p.y() << " " << vertices[i].p.z() << endl;
	}
	for (int i = 0; i < mFaces; i++) {
		ss << "f " << faces[i].edge->end - vertices + 1 << " " << faces[i].edge->start - vertices + 1 << " " << faces[i].edge->right_prev->start - vertices + 1 << endl;
	}

	ofstream outputFile(fileName);
	if (outputFile.is_open())
	{
		outputFile << ss.str();
		outputFile.close();
	}
}

// Read obj buffer
void Mesh::readObjBuffer(ObjBuffer buffer) {
	nVertices = buffer.nVertices;
	mFaces = buffer.mFaces;
	lW_edges = buffer.mFaces * 3;
	
	vertices = new Vertex[nVertices];
	faces = new Face[mFaces];
	w_edges = new W_edge[lW_edges];

	center = buffer.center;
	scale = buffer.scale;

	for (int vertexi = 0; vertexi < nVertices; vertexi++) {
		vertices[vertexi].p = buffer.vertices[vertexi];
	}

	int w_edgei = 0;
	int vns [3] = {0, 0, 0};
	for (int facei = 0; facei < mFaces; facei++) {
		int start_w_edgei = w_edgei;
		int start_vn = 0;
		vns[0] = buffer.faces[facei].x();
		vns[1] = buffer.faces[facei].y();
		vns[2] = buffer.faces[facei].z();

		for (auto vn : vns) {
			// obj file is counterclockwise, while winged-edge structure is clock wise
			w_edges[w_edgei].end = vertices + vn - 1;
			w_edges[w_edgei].right = faces + facei;
			vertices[vn - 1].edge = w_edges + w_edgei;

			if (start_vn == 0) {
				start_vn = vn;
			} else {
				w_edges[w_edgei - 1].start = vertices + vn - 1;
				w_edges[w_edgei].right_next = w_edges + w_edgei - 1;
				w_edges[w_edgei - 1].right_prev = w_edges + w_edgei;
			}

			w_edgei++;
		}
		
		w_edges[w_edgei - 1].start = vertices + start_vn - 1;
		w_edges[start_w_edgei].right_next = w_edges + w_edgei - 1;
		w_edges[w_edgei - 1].right_prev = w_edges + start_w_edgei;

		faces[facei].edge = w_edges + start_w_edgei;
	}
}

// Fill in left parameters (left_prev, left_next, and left) of W_edge
void Mesh::constructLeft() {
	W_edge** w_edgeP = new W_edge*[lW_edges];
	for (int i = 0; i < lW_edges; i++) {
		w_edgeP[i] = w_edges + i;
	}

	sort(w_edgeP, w_edgeP + lW_edges, sortByStartVertexThenByEndVertex);

	int bi = 0;
	for (int i = 0; i < lW_edges; i++) {
		if (i == lW_edges - 1 || w_edgeP[i + 1]->start != w_edgeP[i]->start) {
			for (int j = bi; j < i + 1; j++) {
				for (int k = bi; k < i + 1; k++) {
					if (w_edgeP[j]->start == w_edgeP[k]->right_prev->end && w_edgeP[j]->end == w_edgeP[k]->right_prev->start) {
						// This is a match
						w_edgeP[j]->PairLeftW_edge(w_edgeP[k]->right_prev);
					}
				}
			}
			bi = i + 1;
		}
	}
	delete w_edgeP;
}

// Get vertex normals for smooth shading
Vector3f Mesh::getVertexSN(Vertex* v, MatrixXf* normals) {
    Vector3f vec(0, 0, 0);
	vector<Face*> vfs = v->getFaces();
	int facei = -1;

	for (auto f : vfs) {
		facei = f - faces;
		vec += normals->col(facei * 3);
	}

    return vec.normalized();
}



// ======================= render depth map =======================

void Mesh::renderDepthImage() {
    // originally in main
	CvSize imgSize;
	imgSize.width = WIDTH * 2;
	imgSize.height = HEIGHT * 2;
	IplImage* image = cvCreateImage(imgSize, 8, 1);

    renderOneModel("chair", image);
}

void Mesh::renderOneModel(string filename, IplImage* image) {
    // Read model in
    MatrixXf vMatrix = getPositions();
	MatrixXf fMatrix = MatrixXf(3, mFaces); 
	faceToMatrix(fMatrix);
	cout << "size of vMatrix " << vMatrix.rows() << " " << vMatrix.cols() << endl;
	// cout << "size of fMatrix " << fMatrix.rows() << " " << fMatrix.cols() << endl;

	int count0 = 0;
	double xzRot, xyRot;
	xzRot = (2.0 * M_PI) / (DENSITY / 2.0) * 0.0;
	xyRot = (2.0 * M_PI) / (DENSITY / 2.0) * 0.0;

	string out;
	out = std::to_string(count0); 
	cout << "In write image, count 0 - " << count0 << endl;  

	renderOneDepth(filename, out, vMatrix, fMatrix, xzRot, xyRot, image);
	count0++;

	// for (int i = 1; i < DENSITY / 2; i++) {
	// 	for (int j = 0; j < DENSITY; j++) {
	// 		xzRot = (2.0 * M_PI) / DENSITY * i;
	// 		xyRot = (2.0 * M_PI) / DENSITY * j;

	// 		out.clear();
	// 		out = std::to_string(count0); 
	// 		cout << "In write image, count 0 - " << count0 << endl;   

	// 		renderOneDepth(filename, out, vMatrix, fMatrix, xzRot, xyRot, image);
	// 		count0++;
	// 	}
	// }

	// xzRot = (2.0 * M_PI) / (DENSITY / 2.0) * 4.0;
	// xyRot = (2.0 * M_PI) / (DENSITY / 2.0) * 0.0;

	// out.clear();
    // out = std::to_string(count0); 
	// renderOneDepth(filename, out, vMatrix, fMatrix, xzRot, xyRot, image);
}

void Mesh::renderOneDepth(std::string filename, string& depthIndex, 
						  MatrixXf vMatrix, MatrixXf fMatrix,
						  double xzRot, double xyRot, 
						  IplImage* image) {
	string str = "ChairImages/" + depthIndex + "-depth.jpg";

	double tilt = 0;
	double objz = 1;  
	double objx = 1;
	double ratio = 2;

	double fx_rgb = 5.1885790117450188e+02 * ratio;
	double fy_rgb = 5.1946961112127485e+02 * ratio;
	double cx_rgb = 3.2558244941119034e+02 * ratio;
	double cy_rgb = 2.5373616633400465e+02 * ratio;

	MatrixXf K(3,3);
	K(0,0) = fx_rgb; K(0,1) =  0;		K(0,2) =  cx_rgb; 
	K(1,0) =  0;	 K(1,1) =  fy_rgb;	K(1,2) =  cy_rgb; 
	K(2,0) =  0;	 K(2,1) =  0;		K(2,2) =  1; 

	double imw = WIDTH * ratio; 
	double imh = HEIGHT * ratio;
	MatrixXf C(3,1);
	C(0,0) =  0; C(1,0) = 1.7; C(2,0) =  0; 

	double z_near = 0.3;
	double z_far_ratio = 1.2;
	MatrixXf Ryzswi(3,3);
	Ryzswi(0,0) =  1; Ryzswi(0,1) =  0; Ryzswi(0,2) =  0; 
	Ryzswi(1,0) =  0; Ryzswi(1,1) =  0; Ryzswi(1,2) =  1; 
	Ryzswi(2,0) =  0; Ryzswi(2,1) =  1; Ryzswi(2,2) =  0;

	vMatrix = Ryzswi * vMatrix;

	MatrixXf Robj = genRotMat(xzRot, xyRot);
	MatrixXf Rcam = genTiltMat(tilt);

	MatrixXf Cadd(3,4);
	Cadd(0,0) =  1; Cadd(0,1) =  0; Cadd(0,2) =  0; Cadd(0,3) =  0; 
	Cadd(1,0) =  0; Cadd(1,1) =  1; Cadd(1,2) =  0; Cadd(1,3) =  -1; 
	Cadd(2,0) =  0; Cadd(2,1) =  0; Cadd(2,2) =  1; Cadd(2,3) =  0;

	MatrixXf P, Ptemp;
	Ptemp = K * Rcam;
	P = Ptemp * Cadd;

	MatrixXf p1, p2(3,1), p3(3,1);
	p1 = Robj * vMatrix;
	p2(0,0) =  objx;
	p2(1,0) =  1;
	p2(2,0) =  objz;
	p3(0,0) =  2;
	p3(1,0) =  2;
	p3(2,0) =  2;

	MatrixXf vmat = scalePoints(p1, p2, p3);

	unsigned int* result;
	int resultNum;
	vector<float> depth;
	vector<int> depth_255;
	// mexFunction(P, imw, imh, vmat, faces, result, resultNum, depth);
}

void Mesh::faceToMatrix(MatrixXf& fMatrix) {
	for (int i = 0; i < mFaces; i++) {
		fMatrix.col(i) << faces[i].edge->end - vertices + 1, faces[i].edge->start - vertices + 1, faces[i].edge->right_prev->start - vertices + 1;
	}
}

void Mesh::getDepth(double v, double vmin, double vmax, int &depth) {
	if (v < vmin) {
		v = vmin;
	}
	if (v > vmax) {
		v = vmax;
	}
		
	double dv = vmax - vmin;
	depth = (v - vmin) / dv * 255;
}

MatrixXf Mesh::genRotMat(double theta0, double theta1) {
	MatrixXf R, R0(3,3), R1(3,3);
	R0(0,0) = cos(theta0);	R0(0,1) =  0;		R0(0,2) =  -sin(theta0); 
	R0(1,0) =  0;			R0(1,1) =  1;		R0(1,2) =  0; 
	R0(2,0) =  sin(theta0);	R0(2,1) =  0;		R0(2,2) =  cos(theta0); 

	R1(0,0) = cos(theta1);	R1(0,1) =  -sin(theta1);	R1(0,2) =  0; 
	R1(1,0) = sin(theta1);	R1(1,1) =  cos(theta1);		R1(1,2) =  0; 
	R1(2,0) = 0;			R1(2,1) =  0;				R1(2,2) =  1; 

	R = R0 * R1;
	return R;
}

MatrixXf Mesh::genTiltMat(double theta) {
	MatrixXf R(3,3);
	R(0,0) =  1;	R(0,1) =  0;				R(0,2) =  0; 
	R(1,0) =  0;	R(1,1) =  cos(theta);		R(1,2) = -sin(theta); 
	R(2,0) =  0;	R(2,1) =  sin(theta);		R(2,2) =  cos(theta); 
	return R;
}

MatrixXf Mesh::scalePoints(MatrixXf coor, MatrixXf center, MatrixXf size) {
	MatrixXf coornew(coor.rows(), coor.cols());
	
	MatrixXf minv(coor.rows(), 1);
	MatrixXf maxv(coor.rows(), 1);
	for (int i = 0; i < coor.rows(); i++) {
		double maxTemp = -99999;
		double minTemp = 99999;
		for (int j = 0; j < coor.cols(); j++) {
			if (coor(i,j) > maxTemp) {
				maxTemp = coor(i,j);
			}
			if (coor(i,j) < minTemp) {
				minTemp = coor(i,j);
			}
		}
		maxv(i,0) = maxTemp;
		minv(i,0) = minTemp;
	}

	MatrixXf oldCenter;
	oldCenter = minv + maxv;
	for (int i = 0; i < oldCenter.rows(); i++) {
		for (int j = 0; j < oldCenter.cols(); j++) {
			oldCenter(i,j) = oldCenter(i,j)/2;
		}
	}

	MatrixXf oldSize = maxv - minv;

	// MatrixXf scaleM();
	double scale = 9999999;
	for (int i = 0; i < oldSize.rows(); i++) {
		for (int j = 0; j < oldSize.cols(); j++) {
			double temp = (double) 1 / oldSize(i,j);			
			if (scale > temp) {
				scale = temp;
			}
		}
	}

	scale = scale * 1.5;

	MatrixXf p1, p2;
	p1 = scale * coor;
	p2 = center - scale * oldCenter;
	for (int i = 0; i < p1.rows(); i++) {
		for (int j = 0; j < p1.cols(); j++) {
			coornew(i,j) = p1(i,j) * p2(i, 0);
		}
	}

	return coornew;
}

void Mesh::mexFunction(MatrixXf P, int width, int height, 
					   MatrixXf vMat, MatrixXf fMatrix, 
					   unsigned int* result, int &resultNum, 
					   vector<float> &depth) { 
	float m_near = 0.01;
	float m_far = 1e8;
	int m_level = 0;
	int m_linewidth = 1;
	int m_pointsize = 1;

	double* projection = new double[P.rows() * P.cols()]; // 3x4 matrix
	unsigned int count = 0;

	for (int i = 0; i < P.rows(); i++) {
		for (int j = 0; j < P.cols(); j++) {
			*(projection + count) = P(i, j);
			count++;
		}
	}

	double* vertex = new double[vMat.rows() * vMat.cols()]; // 3xn double vertices matrix
	count = 0;
	for (int i = 0; i < vMat.cols(); i++) {
		for (int j = 0; j < vMat.rows(); j++) {
			*(vertex + count) = vMat(j,i);
			count++;
		}
	}

	unsigned int* face = new unsigned int[fMatrix.rows() * fMatrix.cols()]; // 4xn uint32 face matrix
	count = 0;
	for (int i = 0; i < fMatrix.cols(); i++) {
		for (int j = 0; j < fMatrix.rows(); j++) {
			*(face + count) = (unsigned int) fMatrix(j, i);
			count++;
		}
	}

	int m_width = width;
	int m_height = height;
	unsigned int  num_vertex = vMat.cols();
	unsigned int  num_face = fMatrix.cols();

// 	// Step 1: setup off-screen mesa's binding 
// 	OSMesaContext ctx;
// 	ctx = OSMesaCreateContextExt(OSMESA_RGB, 32, 0, 0, NULL );
// 	unsigned char * pbuffer = new unsigned char [3 * m_width * m_height];

// 	// Bind the buffer to the context and make it current
// 	if (!OSMesaMakeCurrent(ctx, (void*)pbuffer, GL_UNSIGNED_BYTE, m_width, m_height)) {
// 		printf("OSMesaMakeCurrent failed!: \n");
// 	}
// 	OSMesaPixelStore(OSMESA_Y_UP, 0);

// 	// Step 2: Setup basic OpenGL setting
// 	glEnable(GL_DEPTH_TEST);
// 	glDisable(GL_LIGHTING);
// 	glDisable(GL_CULL_FACE);
// 	//glEnable(GL_CULL_FACE);
// 	//glCullFace(GL_BACK);
// 	glPolygonMode(GL_FRONT, GL_FILL);
// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 	//glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], 1.0f); // this line seems useless
// 	glViewport(0, 0, m_width, m_height);

// 	// Step 3: Set projection matrices
// 	double scale = (0x0001) << m_level;
// 	double final_matrix[16];

// 	// new way: faster way by reuse computation and symbolic derive. See sym_derive.m to check the math.
// 	double inv_width_scale  = 1.0 / (m_width * scale);
// 	double inv_height_scale = 1.0 / (m_height * scale);
// 	double inv_width_scale_1 = inv_width_scale - 1.0;
// 	double inv_height_scale_1_s = -(inv_height_scale - 1.0);
// 	double inv_width_scale_2 = inv_width_scale * 2.0;
// 	double inv_height_scale_2_s = -inv_height_scale * 2.0;
// 	double m_far_a_m_near = m_far + m_near;
// 	double m_far_s_m_near = m_far - m_near;
// 	double m_far_d_m_near = m_far_a_m_near / m_far_s_m_near;
// 	final_matrix[ 0] = projection[2 + 0 * 3] * inv_width_scale_1 + projection[0 + 0 * 3] * inv_width_scale_2;
// 	final_matrix[ 1] = projection[2 + 0 * 3] * inv_height_scale_1_s + projection[1 + 0 * 3] * inv_height_scale_2_s;
// 	final_matrix[ 2] = projection[2 + 0 * 3] * m_far_d_m_near;
// 	final_matrix[ 3] = projection[2 + 0 * 3];
// 	final_matrix[ 4] = projection[2 + 1 * 3] * inv_width_scale_1 + projection[0 + 1 * 3] * inv_width_scale_2;
// 	final_matrix[ 5] = projection[2 + 1 * 3] * inv_height_scale_1_s + projection[1 + 1 * 3] * inv_height_scale_2_s; 
// 	final_matrix[ 6] = projection[2 + 1 * 3] * m_far_d_m_near;    
// 	final_matrix[ 7] = projection[2 + 1 * 3];
// 	final_matrix[ 8] = projection[2 + 2 * 3] * inv_width_scale_1 + projection[0 + 2 * 3] * inv_width_scale_2; 
// 	final_matrix[ 9] = projection[2 + 2 * 3] * inv_height_scale_1_s + projection[1 + 2 * 3] * inv_height_scale_2_s;
// 	final_matrix[10] = projection[2 + 2 * 3] * m_far_d_m_near;
// 	final_matrix[11] = projection[2 + 2 * 3];
// 	final_matrix[12] = projection[2 + 3 * 3] * inv_width_scale_1 + projection[0 + 3 * 3] * inv_width_scale_2;
// 	final_matrix[13] = projection[2 + 3 * 3] * inv_height_scale_1_s + projection[1 + 3 * 3] * inv_height_scale_2_s;  
// 	final_matrix[14] = projection[2 + 3 * 3] * m_far_d_m_near - (2 * m_far * m_near) / m_far_s_m_near;
// 	final_matrix[15] = projection[2 + 3 * 3];

// 	// matrix is ready. use it
// 	glMatrixMode(GL_PROJECTION);
// 	glLoadIdentity();
// 	glMatrixMode(GL_MODELVIEW);
// 	glLoadIdentity();

// 	// Step 3: render the mesh with encoded color from their ID
// 	unsigned char colorBytes[3];
// 	unsigned int base_offset;

// 	base_offset = 1;
// 	glBegin(GL_TRIANGLES);
// 	for (unsigned int i = 0; i < num_face; ++i) {
// 		uint2uchar(base_offset+i,colorBytes);
// 		glColor3ubv(colorBytes);

// 		glVertex3dv(vertex + 3 * (*face++));
// 		glVertex3dv(vertex + 3 * (*face++));
// 		glVertex3dv(vertex + 3 * (*face++));
// 		*face++; 
// 	}
// 	glEnd();
// 	glFinish();

// 	unsigned int* pDepthBuffer;
// 	GLint outWidth, outHeight, bitPerDepth;
// 	OSMesaGetDepthBuffer(ctx, &outWidth, &outHeight, &bitPerDepth, (void**)&pDepthBuffer);
// 	resultNum = (int)outWidth * (int)outHeight;

// 	for (int i = 0; i < resultNum; i++) {
// 		unsigned int temp = pDepthBuffer[i];
// 		depth.push_back(temp);
// 	}

// 	for (int i = 0; i < depth.size(); i++) {
// 		depth[i] = 0.3 / (1 - float(depth[i]) / pow((float)2,32));
// 		depth[i] = (depth[i] - 0.2) / 0.6;

// 		if (depth[i] < 0) {
// 			depth[i] = 0;
// 		}
// 		if (depth[i] > 1) {
// 			depth[i] = 1;
// 		}
// 	}

// //  delete [] pDepthBuffer;
// // 	delete [] pbuffer;
// // 	delete [] result;
// // 	delete [] projection;
// // 	delete [] face;
// // 	delete [] vertex;

// 	OSMesaDestroyContext(ctx);
} 