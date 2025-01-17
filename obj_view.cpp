/*
    Sample code by Wallace Lira <http://www.sfu.ca/~wpintoli/> based on
    the four Nanogui examples and also on the sample code provided in
          https://github.com/darrenmothersele/nanogui-test
    
    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

////// add while windows//////////
#define _CRT_SECURE_NO_DEPRECATE

#include <GL/glew.h>
///////////////////////////////

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/glcanvas.h>
#include <iostream>
#include <string>
// List files
#include <experimental/filesystem>
#include <cstdlib>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <filesystem>


// Includes for the GLTexture class.
#include <algorithm>
#include <numeric>      // std::iota
#include <cstdint>
#include <memory>
#include <utility>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

#include "ChairMixer.h"
#include "Mesh.h"

using namespace std;

using nanogui::Screen;
using nanogui::Window;
using nanogui::GroupLayout;
using nanogui::Button;
using nanogui::CheckBox;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::Vector3f;
using nanogui::MatrixXu;
using nanogui::MatrixXf;
using nanogui::Label;
using nanogui::Arcball;

class MyGLCanvas : public nanogui::GLCanvas {
public:
    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent) {

        using namespace nanogui;
        
        mShader.initFromFiles("a_smooth_shader", "StandardShading.vertexshader", "StandardShading.fragmentshader");
        mArcball.setSize({500,500});

        // After binding the shader to the current context we can send data to opengl that will be handled
        // by the vertex shader and then by the fragment shader, in that order.
        // if you want to know more about modern opengl pipeline take a look at this link
        // https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview
        mShader.bind();

        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);
        mShader.uploadAttrib("vertexNormal_modelspace", normals);

        //ProjectionMatrixID
        float fovy = 90.0f, aspect = 1.0f;
        float near_ = 0.1f, far_ = 100.0f;
        float top = near_ * tan(fovy * PI / 360);
        float bottom = -top;
        float right = top * aspect;
        float left = -right;
        Matrix4f P = frustum(left, right, bottom, top, near_, far_);

        mShader.setUniform("P", P);

        // ViewMatrixID
        Matrix4f V = lookAt(Vector3f(0,0,3), Vector3f(0,0,0), Vector3f(0,1,0));
        mShader.setUniform("V", V);

        mTranslation = Vector3f(0, 0, 0);
        mZoom = Vector3f(4, 4, 4);

        // This the light origin position in your environment, which is totally arbitrary
        // however it is better if it is behind the observer
		mShader.setUniform("LightPosition_worldspace", Vector3f(2,6,4)); //2,-6,-4

        string path = "ChairModels";
        mMixer.readFolder(path);
    }

    //flush data on call
    ~MyGLCanvas() {
        delete mMesh;
        mMixer.free();
        mShader.free();
    }

    // method to load obj
    void loadObj(string fileName) {
        delete mMesh;
        mMesh = new Mesh(fileName);
        positions = mMesh->getPositions();
        normals = mMesh->getNormals(&positions);
        colors = mMesh->getColors();
    }

    // Temp test method
    void tempTest(string fileName) {
        ObjBuffer mixed = mMixer.tempTest();

        delete mMesh;
        mMesh = new Mesh(mixed);
        mMesh->writeObjFromMesh(fileName);

        positions = mMesh->getPositions();
        normals = mMesh->getNormals(&positions);
        colors = mMesh->getColors();
    }

    // Initialize method
    void initialize(string fileName, int idx, int n_to_show) {
        ObjBuffer mixed = mMixer.initialize(idx, n_to_show);

        delete mMesh;
        mMesh = new Mesh(mixed);
        mMesh->writeObjFromMesh(fileName);

        // positions = mMesh->getPositions();
        // normals = mMesh->getNormals(&positions);
        // colors = mMesh->getColors();
    }

    // Evolve method
    void evolve(string fileName, int level, int idx) {
        ObjBuffer mixed = mMixer.evolve(level, idx);

        delete mMesh;
        mMesh = new Mesh(mixed);
        mMesh->writeObjFromMesh(fileName);

        // positions = mMesh->getPositions();
        // normals = mMesh->getNormals(&positions);
        // colors = mMesh->getColors();
    }

    void writeObj(string fileName) {
        if (mMesh != NULL) {
            mMesh->writeObj(fileName);
        } else {
            cout << "No object in scene" << endl;
        }
    }

    // Method to set shading mode
    void setShadingMode(int fShadingMode) {
        mShadingMode = fShadingMode;
    }
    //Method to update the rotation on each axis
    void setRotation(nanogui::Matrix4f matRotation) {
        mRotation = matRotation;
    }

    void setZoom(nanogui::Vector3f vZoom) {
        mZoom = vZoom;
    }

    void setTranslation(nanogui::Vector3f vTranslation) {
        mTranslation = vTranslation;
    }

    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override {
        if (button == GLFW_MOUSE_BUTTON_2) {    
            mArcball.button(p, down);
            return true;
        }
        return false;
    }

    // right click => rotation
    bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override {
        if (button == GLFW_MOUSE_BUTTON_3 ) {   
            mArcball.motion(p);
            return true;
        }
        return false;
    }

    // left click => translation
    bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override {
        if (button == GLFW_MOUSE_BUTTON_2) { 
            setTranslation(mTranslation + Eigen::Vector3f(rel.x()/100.0f, -rel.y()/100.0f, 0.0f));
            return true;
        }
        return false;
    }

    // scroll => zooming
    bool scrollEvent(const Vector2i &p, const Vector2f &rel) override {
        float scaleFactor = rel.y()/5.0f;
        if(mZoom.x() + scaleFactor <= 0 || mZoom.y() + scaleFactor <= 0 || mZoom.z() + scaleFactor <= 0)
        {
            return false;
        }
        setZoom(mZoom + Eigen::Vector3f(scaleFactor, scaleFactor, scaleFactor));
        return true;
    }

    //OpenGL calls this method constantly to update the screen.
    virtual void drawGL() override {
        using namespace nanogui;
        if (mMesh == NULL) return;
        
	    //refer to the previous explanation of mShader.bind();
        mShader.bind();

	    //dates the positions matrix,color and indices matrices
        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);
	    mShader.uploadAttrib("vertexNormal_modelspace", normals);

        //ModelMatrixID
        setRotation(mArcball.matrix());
        Matrix4f M = translate(mTranslation) * mRotation * scale(mZoom);
        mShader.setUniform("M", M);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE_EXT);
        glPointSize(5);

        if (mShadingMode != 1 && mShadingMode != 3) {
            mShader.drawArray(GL_TRIANGLES, 0, positions.cols() / 3);
        }
        if (mShadingMode != 0 && mShadingMode != 3) {
            mShader.drawArray(GL_LINES, positions.cols() / 3, positions.cols() / 3 * 2);
            mShader.drawArray(GL_POINTS, positions.cols() / 3, positions.cols() / 3 * 2);
        }
        // Point Cloud
        if (mShadingMode == 3){
            mShader.drawArray(GL_POINTS, positions.cols() / 3, positions.cols() / 3 * 2);
        }
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_PROGRAM_POINT_SIZE_EXT);

    }

public: 
    ChairMixer mMixer;

//Instantiation of the variables that can be acessed outside of this class to interact with the interface
//Need to be updated if a interface element is interacting with something that is inside the scope of MyGLCanvas
private:
    nanogui::GLShader mShader;
    Eigen::Matrix4f mRotation;
    Eigen::Vector3f mZoom;
    Eigen::Vector3f mTranslation;
    nanogui::Arcball mArcball;
    Mesh *mMesh = NULL;
    MatrixXf positions;
    MatrixXf normals;
    MatrixXf colors;
    int mShadingMode = 0;
};

class ObjViewApp : public nanogui::Screen {
public:
    ObjViewApp() : nanogui::Screen(Eigen::Vector2i(1000, 1000), "Chair Modeling", false) {
        using namespace nanogui;

	    // Create a window context in which we will render the OpenGL canvas
        Window *window = new Window(this, "Obj Viewer");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

		///////////////// add while windows ///////////////////
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();

		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
		///////////////////////////////////////////////////

	    // OpenGL canvas initialization
        mCanvas = new MyGLCanvas(window);
        mCanvas->setBackgroundColor({100, 100, 100, 255});
        mCanvas->setSize({500, 500});

    	// Create widgets window and insert widgets into it
	    Window *widgets = new Window(this, "Widgets");
        widgets->setPosition(Vector2i(560, 15));
        widgets->setLayout(new GroupLayout());
	    widgets->setFixedSize(Vector2i(200, 1000));

	    // Open obj file
        new Label(widgets, "File dialog", "sans-bold", 20);
        Button *openBtn  = new Button(widgets, "Open");
        openBtn->setCallback([&] {
            string fileName = file_dialog({ {"obj", "obj file"} }, false);
            ObjViewApp::fileName = fileName;
            mCanvas->loadObj(fileName);
        });

        // Save obj file
        Button *saveBtn = new Button(widgets, "Save");
        saveBtn->setCallback([&] {
            string fileName = file_dialog({ {"obj", "obj file"} }, true);
            mCanvas->writeObj(fileName);
        });

        // Test button
        Button *testBtn = new Button(widgets, "Evolve");

        // Evolve buttons
        Button *initBtn = new Button(widgets, "Initialize");
        Button *legBtn = new Button(widgets, "Swap Legs");
        Button *armBtn = new Button(widgets, "Swap Arms");
        Button *backBtn = new Button(widgets, "Swap Back");

        Label *chairslabel = new Label(widgets, "Chair Models", "sans-bold", 20);
        Label *scorelabel = new Label(widgets, "Score:  ", "sans-bold", 20);
        chairslabel->setVisible(false);
        scorelabel->setVisible(false);

        // Generate chair obj buttons
        vector<Button*> objs;
        
        // create output directory 
        string folder = "Completion"; 
        struct stat sb;
        if (!(stat(folder.c_str(), &sb) == 0)) // && S_ISDIR(sb.st_mode)
        {
            const int dir_err = system("mkdir -p Completion");
            if (-1 == dir_err)
            {
                printf("Error creating directory!n");
                exit(1);
            }
        }

        size_t n_to_show = 0; 
        // count existing files in folder
        // for (const auto & entry : experimental::filesystem::directory_iterator(folder)){
            // cout << entry.path() << endl;
            // n++;
        // }
        if (n_to_show < 5) n_to_show = 5; // generate 5 objs in one test
        for (size_t i = 0; i < n_to_show; i++) {
            Button *obj = new Button(widgets, "chair " + to_string(i) + ".obj");
            obj->setVisible(false);  
            objs.push_back(obj);  
        }  
        
        // call back function for testBtn 
        testBtn->setCallback([this, objs, chairslabel, scorelabel, n_to_show, folder]() {
            // bool wasVisible = objs[0]->visible();
            emptyCompletion();
            emptyRendered();
            
            for(size_t idx = 0; idx < n_to_show; idx++)
            {  
                string objname = folder + "/"  + to_string(idx) + ".obj";
                mCanvas->tempTest(objname);

                chairslabel->setVisible(true);
                scorelabel->setVisible(true);
                objs[idx]->setVisible(true);

                objs[idx]->setCallback([this, objname, scorelabel, idx] {
                    ObjViewApp::fileName = objname;
                    mCanvas->loadObj(fileName);
                    vector<float> scores; 
                    loadScores(scores);
                    scorelabel->setCaption("Score:  " + to_string(scores[idx]));
                });
            }

            createDepthMapAndScore();
            performLayout();           
        });  


        // call back function for testBtn 
        // initBtn->setCallback([this, objs, chairslabel, scorelabel, n_to_show, &selected_idx, folder]() {
        initBtn->setCallback([&, objs, chairslabel, scorelabel, n_to_show, folder]() {
            emptyCompletion();
            emptyRendered();

            int n_to_make = mCanvas->mMixer.chairs.size();

            for(size_t idx = 0; idx < n_to_make; idx++)
            {  
                string objname = folder + "/init-"  + to_string(idx) + ".obj";
                mCanvas->initialize(objname, idx, n_to_show);

                chairslabel->setVisible(true);
                scorelabel->setVisible(true);
                if (idx < n_to_show) {
                    objs[idx]->setVisible(true);
                }
            }

            createDepthMapAndScore();

            // get scores
            vector<float> scores; 
            loadScores(scores);
            vector<int> scores_idx(scores.size());
            sortScores(scores, scores_idx);

            // show models according to score
            for (size_t idx = 0; idx < n_to_show; idx++) {
                int which_score = scores_idx[idx];
                float curr_score = scores[which_score];
                ObjViewApp::selected_idx.push_back(which_score);
                cout << "n " << idx << " to show | which score" << which_score << endl;

                string objname = folder + "/init-"  + to_string(which_score) + ".obj";
                objs[idx]->setCallback([this, objname, scorelabel, curr_score] {
                    ObjViewApp::fileName = objname;
                    mCanvas->loadObj(fileName);
                    scorelabel->setCaption("Score:  " + to_string(curr_score));
                });
            }

            // record the selections
            mCanvas->mMixer.updateRecord(0, ObjViewApp::selected_idx);
            ObjViewApp::selected_idx.clear();

            performLayout();           
        }); 

        // call back function for leg  
        legBtn->setCallback([&, objs, chairslabel, scorelabel, n_to_show, folder]() {
            emptyCompletion();
            emptyRendered();

            int n_to_make = mCanvas->mMixer.chairs.size() * n_to_show;

            for(size_t idx = 0; idx < n_to_make; idx++) {  
                string objname = folder + "/leg-"  + to_string(idx) + ".obj";
                mCanvas->evolve(objname, 1, idx);
            }

            createDepthMapAndScore();

            // load and sort scores
            vector<float> scores; 
            loadScores(scores);
            vector<int> scores_idx(scores.size());
            sortScores(scores, scores_idx);

            // show models according to score
            for (size_t idx = 0; idx < n_to_show; idx++) {
                int which_score = scores_idx[idx];
                float curr_score = scores[which_score];
                ObjViewApp::selected_idx.push_back(which_score);
                cout << "n " << idx << " to show | which score " << which_score << endl;

                string objname = folder + "/leg-"  + to_string(which_score) + ".obj";
                objs[idx]->setCallback([this, objname, scorelabel, curr_score] {
                    ObjViewApp::fileName = objname;
                    mCanvas->loadObj(fileName);
                    scorelabel->setCaption("Score:  " + to_string(curr_score));
                });
            }

            // record the selections
            mCanvas->mMixer.updateRecord(1, ObjViewApp::selected_idx);
            ObjViewApp::selected_idx.clear();

            performLayout();
        }); 

        // call back function for arm  
        armBtn->setCallback([&, objs, chairslabel, scorelabel, n_to_show, folder]() {
            emptyCompletion();
            emptyRendered();

            int n_to_make = mCanvas->mMixer.chairs.size() * n_to_show;

            for(size_t idx = 0; idx < n_to_make; idx++) {  
                string objname = folder + "/arm-"  + to_string(idx) + ".obj";
                mCanvas->evolve(objname, 2, idx);
            }

            createDepthMapAndScore();

            // load and sort scores
            vector<float> scores; 
            loadScores(scores);
            vector<int> scores_idx(scores.size());
            sortScores(scores, scores_idx);

            // show models according to score
            for (size_t idx = 0; idx < n_to_show; idx++) {
                int which_score = scores_idx[idx];
                float curr_score = scores[which_score];
                ObjViewApp::selected_idx.push_back(which_score);
                cout << "n " << idx << " to show | which score " << which_score << endl;

                string objname = folder + "/arm-"  + to_string(which_score) + ".obj";
                objs[idx]->setCallback([this, objname, scorelabel, curr_score] {
                    ObjViewApp::fileName = objname;
                    mCanvas->loadObj(fileName);
                    scorelabel->setCaption("Score:  " + to_string(curr_score));
                });
            }

            // record the selections
            mCanvas->mMixer.updateRecord(2, ObjViewApp::selected_idx);
            ObjViewApp::selected_idx.clear();

            performLayout();
        }); 


        // call back function for arm  
        backBtn->setCallback([&, objs, chairslabel, scorelabel, n_to_show, folder]() {
            emptyCompletion();
            emptyRendered();

            int n_to_make = mCanvas->mMixer.chairs.size() * n_to_show;

            for(size_t idx = 0; idx < n_to_make; idx++) {  
                string objname = folder + "/back-"  + to_string(idx) + ".obj";
                mCanvas->evolve(objname, 3, idx);
            }

            createDepthMapAndScore();

            // load and sort scores
            vector<float> scores; 
            loadScores(scores);
            vector<int> scores_idx(scores.size());
            sortScores(scores, scores_idx);

            // show models according to score
            for (size_t idx = 0; idx < n_to_show; idx++) {
                int which_score = scores_idx[idx];
                float curr_score = scores[which_score];
                ObjViewApp::selected_idx.push_back(which_score);
                cout << "n " << idx << " to show | which score " << which_score << endl;

                string objname = folder + "/back-"  + to_string(which_score) + ".obj";
                objs[idx]->setCallback([this, objname, scorelabel, curr_score] {
                    ObjViewApp::fileName = objname;
                    mCanvas->loadObj(fileName);
                    scorelabel->setCaption("Score:  " + to_string(curr_score));
                });
            }

            // record the selections
            mCanvas->mMixer.updateRecord(3, ObjViewApp::selected_idx);
            ObjViewApp::selected_idx.clear();

            performLayout();
        }); 


        
    	// Shading mode
        new Label(widgets, "Shading Mode", "sans-bold", 20);
        Widget *panelCombo = new Widget(widgets);
        panelCombo->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));

        ComboBox *combo = new ComboBox(widgets, { "Flat", "Wireframe", "Flat+Wireframe", "Point Cloud"} );
        combo->setCallback([&](int value) {
            mCanvas->setShadingMode(value);
        });

        // Quit button
        new Label(widgets, "Quit", "sans-bold", 20);
        Button *quitBtn = new Button(widgets, "Quit");
        quitBtn->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Quit", "Are you sure to shut down?", "No", "Yes", true);
            dlg->setCallback([](int result) { 
                if(result == 1){
                    nanogui::shutdown();
                }});
        });

	    //Method to assemble the interface defined before it is called
        performLayout();
    }

    virtual void drawContents() override {
        // ... put your rotation code here if you use dragging the mouse, updating either your model points, the mvp matrix or the V matrix, depending on the approach used
    }

    virtual void draw(NVGcontext *ctx) {
        /* Draw the user interface */
        Screen::draw(ctx);
    }

    void emptyCompletion() {
		if (system("del /q C:\\Project\\A2\\A2\\Completion\\*") == 0) { //system("exec rm -r ./Completion/*") 
            cout << "Successfully emptied Completion" << endl; 
        } else {
            cout << "Error emptying Completion" << endl; 
        }
    }

    void emptyRendered() {
        if (system("del /q C:\\Project\\A2\\A2\\Rendered\\*") == 0) { //system("exec rm -r ./Rendered/*")
            cout << "Successfully emptied Rendered" << endl; 
        } else {
            cout << "Error emptying Rendered" << endl; 
        }
    }

    void createDepthMapAndScore() {

		if (system("D:/ProgramData/Anaconda3/python get_eval_list.py") == 0) {
			cout << "Successfully created test list" << endl;
			// if(system("/usr/bin/python3 test.py ") == 0){
			if (system("D:/ProgramData/Anaconda3/python evaluate.py") == 0) {
				cout << "Successfully scored chairs" << endl;
			}
			else {
				cout << "Error scoring chairs" << endl;
			}
		}
		else {
			cout << "Error creating test list" << endl;
		}


        //if (system("/usr/bin/blender example.blend --background --python render.py") == 0) {
        //    cout << "Successfully created depth map" << endl; 
        //    // if(system("/usr/bin/python3 test.py ") == 0){
        //    if (system("/opt/anaconda3/bin/python test.py") == 0) {
        //        cout << "Successfully scored chairs" << endl; 
        //    } else {
        //        cout << "Error scoring chairs" << endl; 
        //    }
        //} else {
        //    cout << "Error creating depth map" << endl; 
        //}
    }

    void loadScores(vector<float>& scores) {
        ifstream file;
        file.open("score.txt");
        if (!file) {
            cout << "Unable to open file";
            exit(1); 
        }
        string line;
        while (getline(file, line)) {
            scores.push_back(strtof((line).c_str(),0));
        }
        file.close();
    }

    void sortScores(vector<float>& scores, vector<int>& scores_idx) {
        iota(scores_idx.begin(), scores_idx.end(), 0);
        sort(scores_idx.begin(), scores_idx.end(),
        [&scores](int i1, int i2) {return scores[i1] > scores[i2];});
    }

    void printIntVector(vector<int> v, string name) {
        cout << name << " -";
        for (auto i : v) {
            cout << " " << i;
        }
        cout << endl;
    }

    void printFloatVector(vector<float> v, string name) {
        cout << name << " -";
        for (auto i : v) {
            cout << " " << i;
        }
        cout << endl;
    }

private:
    MyGLCanvas *mCanvas;
    vector<int> selected_idx;
    string fileName;
};

int main(int /* argc */, char ** /* argv */) {

    try {
        srand (56843658);
        nanogui::init();

        /* scoped variables */ {
        nanogui::ref<ObjViewApp> app = new ObjViewApp();
        app->drawAll();
        app->setVisible(true);
        nanogui::mainloop();
    }
        nanogui::shutdown();

    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    } 
    return 0;
}
