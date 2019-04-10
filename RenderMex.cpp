/*

This code is to render a Mesh given a 3x4 camera matrix with an image resolution widthxheight. The rendering result is an ID map for facets, edges and vertices. This can usually used for occlusion testing in texture mapping a model from an image, such as the texture mapping in the following two papers.

--Jianxiong Xiao http://mit.edu/jxiao/

Citation:

[1] J. Xiao, T. Fang, P. Zhao, M. Lhuillier, and L. Quan
Image-based Street-side City Modeling
ACM Transaction on Graphics (TOG), Volume 28, Number 5
Proceedings of ACM SIGGRAPH Asia 2009

[2] J. Xiao, T. Fang, P. Tan, P. Zhao, E. Ofek, and L. Quan
Image-based Facade Modeling
ACM Transaction on Graphics (TOG), Volume 27, Number 5
Proceedings of ACM SIGGRAPH Asia 2008

*/

#include "windows.h"
#include "psapi.h"
#include <GL/osmesa.h>
#include <GL/glu.h>
#include <vector>
#include <math.h>
#include <fstream>
#include <iomanip>
#include <time.h>
#include "matrix.h"  
#include <gl/Gl.h>
#include <gl/glut.h>
#include <direct.h> 
#include <io.h>
#include <highgui.h>
#include <cv.h>
using namespace std;
//using namespace math;
typedef matrix<double> Matrix;
#pragma comment(lib,"psapi.lib")

#define PI  3.1415926535897
#define RAND  (rand()%100)/100

#define WIDTH 112
#define HEIGHT 112
#define DENSITY 8

void OutputMatrix(Matrix matrix, const string& filename)
{
	ofstream outFile(filename.c_str());
	for (int i = 0; i < matrix.ColNo(); i++)
	{
		for (int j = 0; j < matrix.RowNo(); j++)
		{
			outFile << matrix(j,i) << "  ";
		}
		outFile << endl;
	}
	outFile.close();
}

void OutputPointer(unsigned int* pointer, int num)
{
	ofstream outFile("pointer.txt");
	outFile << num << endl;
	for (int i = 0; i < num; i++)
	{
//		outFile << *(pointer + i) << endl;
		outFile << pointer[i] << endl;
	}
	outFile.close();
}


void uint2uchar(unsigned int in, unsigned char* out){
	out[0] = (in & 0x00ff0000) >> 16;
	out[1] = (in & 0x0000ff00) >> 8;
	out[2] =  in & 0x000000ff;
}

unsigned int uchar2uint(unsigned char* in){
	unsigned int out = (((unsigned int)(in[0])) << 16) + (((unsigned int)(in[1])) << 8) + ((unsigned int)(in[2]));
	return out;
}

//////////////////////////////////////////////////////////////////////////
// render depth image here
void mexFunction(Matrix P, int width, int height, Matrix vertices, Matrix faces,unsigned int* result, int &resultNum, vector<float> &depth)
{ 
	float m_near = 0.01;
	float m_far = 1e8;
	int m_level = 0;
	int m_linewidth = 1;
	int m_pointsize = 1;

	double* projection = new double[P.RowNo() * P.ColNo()]; // 3x4 matrix
	unsigned int count = 0;

	for (int i = 0; i < P.RowNo(); i++)
	{
		for (int j = 0; j < P.ColNo(); j++)
		{
			*(projection + count) = P(i,j);
			count++;
		}
	}

	double* vertex = new double[vertices.RowNo() * vertices.ColNo()]; // 3xn double vertices matrix
	count = 0;
	for (int i = 0; i < vertices.ColNo(); i++)
	{
		for (int j = 0; j < vertices.RowNo(); j++)
		{
			*(vertex + count) = vertices(j,i);
			count++;
		}
	}

	unsigned int* face = new unsigned int[faces.RowNo() * faces.ColNo()]; // 4xn uint32 face matrix
	count = 0;
	for (int i = 0; i < faces.ColNo(); i++)
	{
		for (int j = 0; j < faces.RowNo(); j++)
		{
			*(face + count) = (unsigned int)faces(j,i);
			count++;
		}
	}

	int m_width = width;
	int m_height = height;
	unsigned int  num_vertex = vertices.ColNo();
	unsigned int  num_face = faces.ColNo();

	// Step 1: setup off-screen mesa's binding 
	OSMesaContext ctx;
	ctx = OSMesaCreateContextExt(OSMESA_RGB, 32, 0, 0, NULL );
	unsigned char * pbuffer = new unsigned char [3 * m_width * m_height];

	// Bind the buffer to the context and make it current
	if (!OSMesaMakeCurrent(ctx, (void*)pbuffer, GL_UNSIGNED_BYTE, m_width, m_height)) {
		printf("OSMesaMakeCurrent failed!: \n");
	}
	OSMesaPixelStore(OSMESA_Y_UP, 0);

	// Step 2: Setup basic OpenGL setting
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], 1.0f); // this line seems useless
	glViewport(0, 0, m_width, m_height);

	// Step 3: Set projection matrices
	double scale = (0x0001) << m_level;
	double final_matrix[16];

	// new way: faster way by reuse computation and symbolic derive. See sym_derive.m to check the math.
	double inv_width_scale  = 1.0/(m_width*scale);
	double inv_height_scale = 1.0/(m_height*scale);
	double inv_width_scale_1 =inv_width_scale - 1.0;
	double inv_height_scale_1_s = -(inv_height_scale - 1.0);
	double inv_width_scale_2 = inv_width_scale*2.0;
	double inv_height_scale_2_s = -inv_height_scale*2.0;
	double m_far_a_m_near = m_far + m_near;
	double m_far_s_m_near = m_far - m_near;
	double m_far_d_m_near = m_far_a_m_near/m_far_s_m_near;
	final_matrix[ 0]= projection[2+0*3]*inv_width_scale_1 + projection[0+0*3]*inv_width_scale_2;
	final_matrix[ 1]= projection[2+0*3]*inv_height_scale_1_s + projection[1+0*3]*inv_height_scale_2_s;
	final_matrix[ 2]= projection[2+0*3]*m_far_d_m_near;
	final_matrix[ 3]= projection[2+0*3];
	final_matrix[ 4]= projection[2+1*3]*inv_width_scale_1 + projection[0+1*3]*inv_width_scale_2;
	final_matrix[ 5]= projection[2+1*3]*inv_height_scale_1_s + projection[1+1*3]*inv_height_scale_2_s; 
	final_matrix[ 6]= projection[2+1*3]*m_far_d_m_near;    
	final_matrix[ 7]= projection[2+1*3];
	final_matrix[ 8]= projection[2+2*3]*inv_width_scale_1 + projection[0+2*3]*inv_width_scale_2; 
	final_matrix[ 9]= projection[2+2*3]*inv_height_scale_1_s + projection[1+2*3]*inv_height_scale_2_s;
	final_matrix[10]= projection[2+2*3]*m_far_d_m_near;
	final_matrix[11]= projection[2+2*3];
	final_matrix[12]= projection[2+3*3]*inv_width_scale_1 + projection[0+3*3]*inv_width_scale_2;
	final_matrix[13]= projection[2+3*3]*inv_height_scale_1_s + projection[1+3*3]*inv_height_scale_2_s;  
	final_matrix[14]= projection[2+3*3]*m_far_d_m_near - (2*m_far*m_near)/m_far_s_m_near;
	final_matrix[15]= projection[2+3*3];

	// matrix is ready. use it
	glMatrixMode(GL_PROJECTION);
	//  glLoadMatrixd(final_matrix);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Step 3: render the mesh with encoded color from their ID
	unsigned char colorBytes[3];
	unsigned int base_offset;

	base_offset = 1;
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < num_face; ++i) {
		uint2uchar(base_offset+i,colorBytes);
		glColor3ubv(colorBytes);

		glVertex3dv(vertex+3*(*face++));
		glVertex3dv(vertex+3*(*face++));
		glVertex3dv(vertex+3*(*face++));
		*face++; 
	}
	glEnd();
	glFinish();


	unsigned int* pDepthBuffer;
	GLint outWidth, outHeight, bitPerDepth;
	OSMesaGetDepthBuffer(ctx, &outWidth, &outHeight, &bitPerDepth, (void**)&pDepthBuffer);
	resultNum = (int)outWidth * (int)outHeight;

	for (int i = 0; i < resultNum; i++)
	{
		unsigned int temp = pDepthBuffer[i];
		depth.push_back(temp);
	}

	for (int i = 0; i < depth.size(); i++)
	{
		depth[i] = 0.3 / (1 - float(depth[i])/pow((float)2,32));
		depth[i] = (depth[i] - 0.2)/0.6;


		if (depth[i] < 0)
		{
			depth[i] = 0;
		}
		if (depth[i] > 1)
		{
			depth[i] = 1;
		}
	}

//     delete [] pDepthBuffer;
// 	delete [] pbuffer;
// 	delete [] result;
// 	delete [] projection;
// 	delete [] face;
// 	delete [] vertex;

	OSMesaDestroyContext(ctx);
} 


void ReadOffFile(const string& filename, Matrix& vertices, Matrix& faces)
{
	ifstream in(filename.c_str());
	char buf[256];
	in.getline(buf, sizeof buf);
	int vertNum, faceNum, edgeNum;
	in >> vertNum >> faceNum >> edgeNum;
	vertices.SetSize(3, vertNum);
	faces.SetSize(4, faceNum);
	std::cout << "vertNum: " << vertNum << endl;
	std::cout << "faceNum: " << faceNum << endl;
	for (int i = 0; i < vertNum; ++i)
	{
		double x, y, z;
		in >> x;
		in >> y;
		in >> z;
		vertices(0, i) = x;
		vertices(1, i) = y;
		vertices(2, i) = z;

//		std::cout << "x: " << x << "  y: " << y << "  z: " << z << endl;
	}

	int degree;
	for (int i = 0; i < faceNum; ++i)
	{
		in >> degree;

		int first, second, third;
		in >> first >> second >> third;
		faces(0, i) = first;
		faces(1, i) = second;
		faces(2, i) = third;
		faces(3, i) = first;

//		std::cout << first << "  " << second << "  " << third << endl;
	}

	in.close();
}

Matrix genRotMat(double theta0, double theta1)
{
	Matrix R, R0(3,3), R1(3,3);
	R0(0,0) = cos(theta0);	R0(0,1) =  0;		R0(0,2) =  -sin(theta0); 
	R0(1,0) =  0;			R0(1,1) =  1;		R0(1,2) =  0; 
	R0(2,0) =  sin(theta0);	R0(2,1) =  0;		R0(2,2) =  cos(theta0); 

	R1(0,0) = cos(theta1);	R1(0,1) =  -sin(theta1);	R1(0,2) =  0; 
	R1(1,0) = sin(theta1);	R1(1,1) =  cos(theta1);		R1(1,2) =  0; 
	R1(2,0) = 0;			R1(2,1) =  0;				R1(2,2) =  1; 

	R = R0 * R1;
	~R0;
	~R1;
	return R;
}

Matrix genTiltMat(double theta)
{
	Matrix R(3,3);
	R(0,0) =  1;	R(0,1) =  0;				R(0,2) =  0; 
	R(1,0) =  0;	R(1,1) =  cos(theta);		R(1,2) = -sin(theta); 
	R(2,0) =  0;	R(2,1) =  sin(theta);		R(2,2) =  cos(theta); 
	return R;
}

Matrix scalePoints(Matrix coor, Matrix center, Matrix size)
{
	Matrix coornew(coor.RowNo(), coor.ColNo());
	
	Matrix minv(coor.RowNo(), 1);
	Matrix maxv(coor.RowNo(), 1);
	for (int i = 0; i < coor.RowNo(); i++)
	{
		double maxTemp = -99999;
		double minTemp = 99999;
		for (int j = 0; j < coor.ColNo(); j++)
		{
			if (coor(i,j) > maxTemp)
			{
				maxTemp = coor(i,j);
			}
			if (coor(i,j) < minTemp)
			{
				minTemp = coor(i,j);
			}
		}
		maxv(i,0) = maxTemp;
		minv(i,0) = minTemp;
	}

// 	OutputMatrix(maxv, "maxv.txt");
// 	OutputMatrix(minv, "minv.txt");

	Matrix oldCenter;
	oldCenter = minv + maxv;
	for (int i = 0; i < oldCenter.RowNo(); i++)
	{
		for (int j = 0; j < oldCenter.ColNo(); j++)
		{
			oldCenter(i,j) = oldCenter(i,j)/2;
		}
	}

//	OutputMatrix(oldCenter, "oldCenter.txt");
	Matrix oldSize = maxv - minv;
//	OutputMatrix(oldSize, "oldSize.txt");

	Matrix scaleM();
	double scale = 9999999;
	for (int i = 0; i < oldSize.RowNo(); i++)
	{
		for (int j = 0; j < oldSize.ColNo(); j++)
		{
			double temp = (double)1 / oldSize(i,j);
//			cout << "oldSize(i,j): " << oldSize(i,j) << endl;
			
			if (scale > temp)
			{
//				cout << "temp: " << temp << endl;
				scale = temp;
			}
		}
	}
//	cout << "scale: " << scale << endl;
	scale = scale * 1.5;

	Matrix p1,p2;
	p1 = scale * coor;
	p2 = center - scale * oldCenter;
// 	OutputMatrix(p1, "p1z.txt");
// 	OutputMatrix(p2, "p2z.txt");
	for (int i = 0; i < p1.RowNo(); i++)
	{
		for (int j = 0; j < p1.ColNo(); j++)
		{
			coornew(i,j) = p1(i,j) * p2(i, 0);
		}
	}
//	OutputMatrix(coornew, "coornew.txt");

	~coornew;
	~minv;
	~maxv;
	~oldCenter;
	~oldSize;
//	~scaleM;
	~p1;
	~p2;
	return coornew;
}

void IntToString (std::string& out, int value)  
{  
	char buf[32];  
	itoa (value, buf, 10); // #include <stdlib.h>  
	out.append (buf); 
	delete[] buf;
}  


void GetDepth(double v,double vmin,double vmax,int &depth)
{
	if (v < vmin)
		v = vmin;
	if (v > vmax)
		v = vmax;
	double dv = vmax - vmin;

	depth = (v - vmin) / dv * 255;
}

void showMem()
{
	/*
	MEMORYSTATUS memstatus;
	memset(&memstatus,0,sizeof(MEMORYSTATUS));
	memstatus.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&memstatus);
	DWORD mem = memstatus.dwAvailPhys;
	cout << "Free Memory is: " << mem <<"Bytes!" << endl;
	*/

	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));
	cout << "�ڴ�ʹ�ã� "<< pmc.WorkingSetSize/1000 << "K/" << pmc.PagefileUsage << endl;
}

void RenderOneDepth(const string& filename, string& depthIndex, Matrix vertices, Matrix faces, double xzRot, double xyRot, IplImage* image)
{
	string strhead = "\\depth\\";
	string strend = "depth.jpg";
	string str;

//	showMem();
	str = filename + strhead;
	str = str + depthIndex;
	str = str + strend;

	double tilt = 0;
	double objz = 1;  
	double objx = 1;
	double ratio = 2;

	double fx_rgb = 5.1885790117450188e+02 * ratio;
	double fy_rgb = 5.1946961112127485e+02 * ratio;
	double cx_rgb = 3.2558244941119034e+02 * ratio;
	double cy_rgb = 2.5373616633400465e+02 * ratio;

	Matrix	K(3,3);
	K(0,0) = fx_rgb; K(0,1) =  0;		K(0,2) =  cx_rgb; 
	K(1,0) =  0;	 K(1,1) =  fy_rgb;	K(1,2) =  cy_rgb; 
	K(2,0) =  0;	 K(2,1) =  0;		K(2,2) =  1; 

	double imw = WIDTH * ratio; 
	double imh = HEIGHT * ratio;
	Matrix	C(3,1);
	C(0,0) =  0; C(1,0) = 1.7; C(2,0) =  0; 

	double z_near = 0.3;
	double z_far_ratio = 1.2;
	Matrix Ryzswi(3,3);
	Ryzswi(0,0) =  1; Ryzswi(0,1) =  0; Ryzswi(0,2) =  0; 
	Ryzswi(1,0) =  0; Ryzswi(1,1) =  0; Ryzswi(1,2) =  1; 
	Ryzswi(2,0) =  0; Ryzswi(2,1) =  1; Ryzswi(2,2) =  0;

//	std::cout << "///////////1111111" << endl;
	vertices = Ryzswi * vertices;

	Matrix Robj = genRotMat(xzRot, xyRot);
	Matrix Rcam = genTiltMat(tilt);

	Matrix Cadd(3,4);
	Cadd(0,0) =  1; Cadd(0,1) =  0; Cadd(0,2) =  0; Cadd(0,3) =  0; 
	Cadd(1,0) =  0; Cadd(1,1) =  1; Cadd(1,2) =  0; Cadd(1,3) =  -1; 
	Cadd(2,0) =  0; Cadd(2,1) =  0; Cadd(2,2) =  1; Cadd(2,3) =  0; 

	Matrix P, Ptemp;
	Ptemp = K * Rcam;
	P = Ptemp * Cadd;

	Matrix p1, p2(3,1), p3(3,1);
	p1 = Robj * vertices;
	p2(0,0) =  objx;
	p2(1,0) =  1;
	p2(2,0) =  objz;
	p3(0,0) =  2;
	p3(1,0) =  2;
	p3(2,0) =  2;

	Matrix vmat = scalePoints(p1, p2, p3);

//	cout << "/////22222 "<< endl;
//	showMem();
	unsigned int* result;
	int resultNum;
	vector<float> depth;
	vector<int> depth_255;
	mexFunction(P, imw, imh, vmat, faces, result, resultNum, depth);

	delete[] result;
	for (int i = 0; i < depth.size(); i++)
	{
		int depthInt;
		GetDepth(depth[i], 0, 1, depthInt);
		depth_255.push_back(depthInt);
	}

//	showMem();

	CvSize imgSize;
	imgSize.width = WIDTH * 2;
	imgSize.height = HEIGHT * 2;
	
	for(int i = 0;i < imgSize.width; i++)
	{
		for(int j = 0;j < imgSize.height; j++)
		{
			((uchar*)(image->imageData+image->widthStep*j))[i] = (char)depth_255[i * WIDTH * 2 + j];
//			((uchar*)(image->imageData+image->widthStep*j))[i] = (char)1;
		}
	}
	cvSaveImage(str.c_str(), image);


// 	ofstream outFile(str.c_str());
// 	for (int i = 0; i < WIDTH * 2; i++)
// 	{
// 		for (int j = 0; j < HEIGHT * 2; j++)
// 		{
// 			outFile << setiosflags(ios::fixed) << depth[i*WIDTH*2 + j] << "  ";
// 		}
// 		outFile << endl;
// 	}
// 	outFile.close();

	// 	maxDepth = max(depth(abs(depth) < 100));
	// 	cropmask = (depth < z_near) | (depth > z_far_ratio * maxDepth);
	// 	crop = findCropRegion(~cropmask);
	// 	depth = depth(crop(1)+(1:crop(3)), crop(2)+(1:crop(4)));
	// 	depth(cropmask(crop(1)+(1:crop(3)), crop(2)+(1:crop(4)))) = z_far_ratio * maxDept
	~K;
	~C;
	~Ryzswi;
	~Robj;
	~Rcam;
	~Cadd;
	~P;
	~Ptemp;
	~p1;
	~p2;
	~p3;
	~vmat;
	depth.clear();
	depth_255.clear();

//	cout << "/////55555 "<< endl;
	showMem();
}

void getFiles( string path, vector<string>& folderDirs, vector<string>& folderNames, vector<string>& files, int level)  
{  
	//�ļ����  
	long   hFile   =   0;  
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;  
	string p;  
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1)  
	{  
		do  
		{  
			//�����Ŀ¼,����֮  
			//�������,�����б�  
			if((fileinfo.attrib &  _A_SUBDIR))  
			{  
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0 && strcmp(fileinfo.name,"images") != 0 
					&& strcmp(fileinfo.name,"models") != 0 && strcmp(fileinfo.name,"untitled") != 0 /*&& strcmp(fileinfo.name,"depth") != 0*/
					/*&& level == 1*/)
				{
//					getFiles( p.assign(path).append("\\").append(fileinfo.name), files, names, level++);  
					folderDirs.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
					folderNames.push_back(fileinfo.name);
				}
			}  
			else  
			{  
				files.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
			}  
		}while(_findnext(hFile, &fileinfo)  == 0);  
		_findclose(hFile);  
	}  
}
void RenderOneModel(const string& filename, IplImage* image)
{
	string strhead = "\\depth";
	string str;
	str = filename + strhead;
	mkdir(str.c_str());

	string p;
	p.assign(filename).append("\\").append("model.off");
	cout<<p.c_str()<<endl;  

	Matrix vertices,faces; 
	ReadOffFile(p, vertices, faces); 
	
	int count0 = 0;
	double xzRot, xyRot;
	xzRot = (2 * PI)/(DENSITY/2) * 0;
	xyRot = (2 * PI)/(DENSITY/2) * 0;
	string out;
	IntToString(out, count0);
	cout<<count0<<endl;  
	RenderOneDepth(filename, out, vertices, faces, xzRot, xyRot, image);
	count0++;

	for (int i = 1; i < DENSITY/2; i++)
	{
		for (int j = 0; j < DENSITY; j++)
		{
			xzRot = (2 * PI)/DENSITY * i;
			xyRot = (2 * PI)/DENSITY * j;
			out.clear();
			IntToString(out, count0);
			cout<<count0<<endl;  
			RenderOneDepth(filename, out, vertices, faces, xzRot, xyRot, image);
			count0++;
		}
	}

	xzRot = (2 * PI)/(DENSITY/2) * 4;
	xyRot = (2 * PI)/(DENSITY/2) * 0;
	out.clear();
	IntToString(out, count0);
	RenderOneDepth(filename, out, vertices, faces, xzRot, xyRot, image);
	
	~vertices;
	~faces; 
}

void RenderOneCategory(const string& filename, IplImage* image)
{
	int testedObjectCount = 0;
	const char * filePath;
	string front="E:\\siggraph2016_project\\ShapeNetCore2015v0\\current\\"; 
	string end = "\\test";
	string filePathString = front + filename;
	filePathString = filePathString + end;
	filePath = filePathString.c_str();
	cout<<filePathString.c_str()<<endl;  
	
	vector<string> folderDirs;
	vector<string> folderNames;
	vector<string> files;
	int level = 0;
	getFiles(filePath, folderDirs, folderNames, files, level);  
   
	for (int i = 0;i < folderDirs.size();i++)  
	{  
//		cout<<"name: "<<folderDirs[i].c_str()<<endl;  
	}  
// 	system("pause");
	

	char str[30];  
	for (int i = 0;i < folderDirs.size();i++)  
	{  
		bool haveDepthFlag = false;
		int depthFolder = -1;
		vector<string> localFolderDirs;
		vector<string> localFolderNames;
		vector<string> localFiles;
		getFiles(folderDirs[i], localFolderDirs, localFolderNames, localFiles, level);  

		for (int j = 0; j < localFolderNames.size(); j++)
		{
			if (localFolderNames[0] == "depth")
			{
				haveDepthFlag = true;
				depthFolder = j;
//				cout<<"depth: "<<j<<endl;
			}
		}
		
		if (!haveDepthFlag)
		{
			RenderOneModel(folderDirs[i], image);
			continue;
		}
		
		vector<string> depthFolderDirs;
		vector<string> depthFolderNames;
		vector<string> depthFiles;
		getFiles(localFolderDirs[depthFolder], depthFolderDirs, depthFolderNames, depthFiles, level);  
		
//		cout << depthFiles.size() << endl;

		if (depthFiles.size() != 26)
		{
			haveDepthFlag = false;
		}

// 		for (int j = 0; j < depthFiles.size(); j++)
// 		{
// 			cout<<"Depth: "<<depthFiles[j].c_str()<<endl;
// 		}
		
		if (!haveDepthFlag)
		{
			RenderOneModel(folderDirs[i], image);
			testedObjectCount++;
		}

		if (testedObjectCount > 80)
		{
			cout << "return" << endl;
			return;
		}
		
	}   

	delete[] filePath;
}

void main(int argc, char **argv)
{
	srand((unsigned)time(NULL));

	IplImage* image;
	CvSize imgSize;
	imgSize.width = WIDTH * 2;
	imgSize.height = HEIGHT * 2;
	image = cvCreateImage(imgSize, 8, 1);

	RenderOneCategory("chair", image);
}
