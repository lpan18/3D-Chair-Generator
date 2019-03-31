// #include <GL/osmesa.h>
#include <GL/glu.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <GL/gl.h>
#include <GL/glut.h>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <Eigen/Core>
#include <Eigen/Geometry>


using namespace std;
using namespace Eigen;

#define RAND  (rand() % 100) / 100

#define WIDTH 112
#define HEIGHT 112
#define DENSITY 8




void getDepth(double v,double vmin,double vmax,int &depth) {
	if (v < vmin) {
		v = vmin;
	}
	if (v > vmax) {
		v = vmax;
	}
		
	double dv = vmax - vmin;
	depth = (v - vmin) / dv * 255;
}



void renderOneDepth(std::string filename, string& depthIndex, 
						  double xzRot, double xyRot, 
						  IplImage* image) {
	// string strhead = "\\depth\\";
	// string strend = "depth.jpg";
	string str = "depth_images/" + depthIndex + "-depth.jpg";

	// double tilt = 0;
	// double objz = 1;  
	// double objx = 1;
	// double ratio = 2;

	// double fx_rgb = 5.1885790117450188e+02 * ratio;
	// double fy_rgb = 5.1946961112127485e+02 * ratio;
	// double cx_rgb = 3.2558244941119034e+02 * ratio;
	// double cy_rgb = 2.5373616633400465e+02 * ratio;

	// Matrix	K(3,3);
	// K(0,0) = fx_rgb; K(0,1) =  0;		K(0,2) =  cx_rgb; 
	// K(1,0) =  0;	 K(1,1) =  fy_rgb;	K(1,2) =  cy_rgb; 
	// K(2,0) =  0;	 K(2,1) =  0;		K(2,2) =  1; 

	// double imw = WIDTH * ratio; 
	// double imh = HEIGHT * ratio;
	// Matrix	C(3,1);
	// C(0,0) =  0; C(1,0) = 1.7; C(2,0) =  0; 

	// double z_near = 0.3;
	// double z_far_ratio = 1.2;
	// Matrix Ryzswi(3,3);
	// Ryzswi(0,0) =  1; Ryzswi(0,1) =  0; Ryzswi(0,2) =  0; 
	// Ryzswi(1,0) =  0; Ryzswi(1,1) =  0; Ryzswi(1,2) =  1; 
	// Ryzswi(2,0) =  0; Ryzswi(2,1) =  1; Ryzswi(2,2) =  0;

	// vertices = Ryzswi * vertices;

	// Matrix Robj = genRotMat(xzRot, xyRot);
	// Matrix Rcam = genTiltMat(tilt);

	// Matrix Cadd(3,4);
	// Cadd(0,0) =  1; Cadd(0,1) =  0; Cadd(0,2) =  0; Cadd(0,3) =  0; 
	// Cadd(1,0) =  0; Cadd(1,1) =  1; Cadd(1,2) =  0; Cadd(1,3) =  -1; 
	// Cadd(2,0) =  0; Cadd(2,1) =  0; Cadd(2,2) =  1; Cadd(2,3) =  0; 

	// Matrix P, Ptemp;
	// Ptemp = K * Rcam;
	// P = Ptemp * Cadd;

	// Matrix p1, p2(3,1), p3(3,1);
	// p1 = Robj * vertices;
	// p2(0,0) =  objx;
	// p2(1,0) =  1;
	// p2(2,0) =  objz;
	// p3(0,0) =  2;
	// p3(1,0) =  2;
	// p3(2,0) =  2;

	// Matrix vmat = scalePoints(p1, p2, p3);

	// unsigned int* result;
	// int resultNum;
	// vector<float> depth;
	// vector<int> depth_255;
	// mexFunction(P, imw, imh, vmat, faces, result, resultNum, depth);

	// delete[] result;
	// for (int i = 0; i < depth.size(); i++)
	// {
	// 	int depthInt;
	// 	GetDepth(depth[i], 0, 1, depthInt);
	// 	depth_255.push_back(depthInt);
	// }

	// CvSize imgSize;
	// imgSize.width = WIDTH * 2;
	// imgSize.height = HEIGHT * 2;

	// for(int i = 0;i < imgSize.width; i++) {
	// 	for(int j = 0;j < imgSize.height; j++) {
	// 		((uchar*)(image->imageData+image->widthStep*j))[i] = (char)depth_255[i * WIDTH * 2 + j];
	// 	}
	// }

	// cvSaveImage(str.c_str(), image);

	// ~K;
	// ~C;
	// ~Ryzswi;
	// ~Robj;
	// ~Rcam;
	// ~Cadd;
	// ~P;
	// ~Ptemp;
	// ~p1;
	// ~p2;
	// ~p3;
	// ~vmat;
	// depth.clear();
	// depth_255.clear();
}



// Write mesh to an obj file
void renderOneModel(string filename, IplImage* image) {
    // Read model in
    Matrix vertices,faces; 
	ReadOffFile(p, vertices, faces); 

	int count0 = 0;
	double xzRot, xyRot;
	xzRot = (2.0 * M_PI) / (DENSITY / 2.0) * 0.0;
	xyRot = (2.0 * M_PI) / (DENSITY / 2.0) * 0.0;

	string out;
	out = std::to_string(count0); 
	cout << "In write image, count 0 - " << count0 << endl;  

	renderOneDepth(filename, out, xzRot, xyRot, image);
	count0++;

	for (int i = 1; i < DENSITY / 2; i++) {
		for (int j = 0; j < DENSITY; j++) {
			xzRot = (2.0 * M_PI) / DENSITY * i;
			xyRot = (2.0 * M_PI) / DENSITY * j;

			out.clear();
			out = std::to_string(count0); 
			cout << "In write image, count 0 - " << count0 << endl;   

			renderOneDepth(filename, out, xzRot, xyRot, image);
			count0++;
		}
	}

	xzRot = (2.0 * M_PI) / (DENSITY / 2.0) * 4.0;
	xyRot = (2.0 * M_PI) / (DENSITY / 2.0) * 0.0;

	out.clear();
    out = std::to_string(count0); 
	renderOneDepth(filename, out, xzRot, xyRot, image);
}




void main(int argc, char **argv) {
    // originally in main
	CvSize imgSize;
	imgSize.width = WIDTH * 2;
	imgSize.height = HEIGHT * 2;
	IplImage* image = cvCreateImage(imgSize, 8, 1);

    renderOneModel("chair", image);
    return 0;
}