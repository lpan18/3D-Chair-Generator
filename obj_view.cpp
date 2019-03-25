/*
    Sample code by Wallace Lira <http://www.sfu.ca/~wpintoli/> based on
    the four Nanogui examples and also on the sample code provided in
          https://github.com/darrenmothersele/nanogui-test
    
    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

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

// Includes for the GLTexture class.
#include <algorithm>
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

#include "Mesh.h"
#include "MeshSd.h"
#include "MeshMcd.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

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
        
        // After binding the shader to the current context we can send data to opengl that will be handled
        // by the vertex shader and then by the fragment shader, in that order.
        // if you want to know more about modern opengl pipeline take a look at this link
        // https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview
        mShader.bind();

        // ViewMatrixID
        // change your rotation to work on the camera instead of rotating the entire world with the MVP matrix
        Matrix4f V;
        V.setIdentity();
        mShader.setUniform("V", V);
        
        //ModelMatrixID
        Matrix4f M;
        M.setIdentity();
        mShader.setUniform("M", M);
        
        // This the light origin position in your environment, which is totally arbitrary
        // however it is better if it is behind the observer
        mShader.setUniform("LightPosition_worldspace", Vector3f(2,-6,-4));
    }

    //flush data on call
    ~MyGLCanvas() {
        mShader.free();
        delete mMesh;
    }

    // method to load obj
    void loadObj(string fileName) {
        delete mMesh;
        mMesh = new Mesh(fileName);

        positions = mMesh->getPositions();
        normals = mMesh->getNormals(&positions);
        smoothNormals = mMesh->getSmoothNormals(&normals);
        colors = mMesh->getColors();
    }

    // method to load obj (sdLevel is the step of subdivision)
    void loadObjSd(string fileName, int sdLevel = 0, int sdMode = -1) {
        delete mMesh;
        mMesh = new Mesh(fileName);

        if(sdLevel != 0){
            if (sdMode == 0){
                // Loop subdivision
                for (int i = 0; i < sdLevel; i++) {
                    ObjBuffer buffer = ((MeshSd*)mMesh)->sdLoop();
                    delete mMesh;
                    mMesh = new Mesh(buffer);
                }
            } else if(sdMode == 1){
                // Butterfly subdivision
                for (int i = 0; i < sdLevel; i++) {
                    ObjBuffer buffer = ((MeshSd*)mMesh)->sdBtfl();
                    delete mMesh;
                    mMesh = new Mesh(buffer);
                }
            }
        }

        positions = mMesh->getPositions();
        normals = mMesh->getNormals(&positions);
        smoothNormals = mMesh->getSmoothNormals(&normals);
        colors = mMesh->getColors();
    }

    // method to load obj (k is the number of multiple choices; countCollapse is the
    // total count of edge collapses)
    void loadObjMcd(string fileName, int k, int countCollapse) {
        delete mMesh;
        mMesh = new Mesh(fileName);
        
        if (k > mMesh->getVertexCount()) {
            cout << "The value of k " << k << " is too big." << endl;
        } else if (countCollapse > mMesh->getVertexCount()) {
            cout << "The number of edges to collapse " << countCollapse << " is too big." << endl;
        } else {
            ObjBuffer buffer = ((MeshMcd*)mMesh)->mcd(k, countCollapse);
            delete mMesh;
            mMesh = new Mesh(buffer);
        }

        positions = mMesh->getPositions();
        normals = mMesh->getNormals(&positions);
        smoothNormals = mMesh->getSmoothNormals(&normals);
        colors = mMesh->getColors();
    }

    void writeObj(string fileName) {
        if (mMesh != NULL) {
            mMesh->writeObj(fileName);
        } else {
            cout << "No object in scene" << endl;
        }
    }

    // Method to update the rotation on each axis
    void setRotation(nanogui::Vector3f vRotation) {
        mRotation = vRotation;
    }

    // Method to update the rotation on each axis
    void setTranslation(nanogui::Vector3f vTranslation) {
        mTranslation = vTranslation;
    }
    
    // Method to set the zooming 
    void setZooming(float fZooming) {
        mZooming = fZooming;
    }
    // Method to set shading mode
    void setShadingMode(float fShadingMode) {
        mShadingMode = fShadingMode;
    }

    //OpenGL calls this method constantly to update the screen.
    virtual void drawGL() override {
        using namespace nanogui;
        if (mMesh == NULL) return;
        
	    //refer to the previous explanation of mShader.bind();
        mShader.bind();

        MatrixXf shadingNormal = (mShadingMode == 1 || mShadingMode == 4)  ? smoothNormals : normals;
	    //this simple command updates the positions matrix. You need to do the same for color and indices matrices too
        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);
	    mShader.uploadAttrib("vertexNormal_modelspace", shadingNormal);
	
        //This is a way to perform a simple rotation using a 4x4 rotation matrix represented by rmat
	    //mvp stands for ModelViewProjection matrix
        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3,3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mRotation[0], Vector3f::UnitX()) *
                                                   Eigen::AngleAxisf(mRotation[1],  Vector3f::UnitY()) *
                                                   Eigen::AngleAxisf(mRotation[2], Vector3f::UnitZ())) * mZooming;
        mvp.topRightCorner<3,1>() = Eigen::Vector3f(mTranslation[0],mTranslation[1],mTranslation[2])* 0.25f;
       
        mShader.setUniform("MVP", mvp);

	    // If enabled, does depth comparisons and update the depth buffer.
	    // Avoid changing if you are unsure of what this means.
        glEnable(GL_DEPTH_TEST);

        if (mShadingMode != 2) {
            mShader.drawArray(GL_TRIANGLES, 0, positions.cols() / 3);
        }
        if (mShadingMode != 0 && mShadingMode != 1) {
            mShader.drawArray(GL_LINES, positions.cols() / 3, positions.cols() / 3 * 2);
        }

        glDisable(GL_DEPTH_TEST);
    }

//Instantiation of the variables that can be acessed outside of this class to interact with the interface
//Need to be updated if a interface element is interacting with something that is inside the scope of MyGLCanvas
private:
    Mesh *mMesh = NULL;
    MatrixXf positions;
    MatrixXf normals;
    MatrixXf smoothNormals;
    MatrixXf colors;
    nanogui::GLShader mShader;
    Eigen::Vector3f mRotation;
    Eigen::Vector3f mTranslation;
    float mZooming = 0.5f;
    int mShadingMode = 0;
};


class ObjViewApp : public nanogui::Screen {
public:
    ObjViewApp() : nanogui::Screen(Eigen::Vector2i(1100, 700), "Assignment 2", false) {
        using namespace nanogui;

	    // Create a window context in which we will render the OpenGL canvas
        Window *window = new Window(this, "Obj Viewer");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());
	
	    // OpenGL canvas initialization
        mCanvas = new MyGLCanvas(window);
        mCanvas->setBackgroundColor({100, 100, 100, 255});
        mCanvas->setSize({400, 400});

    	// Create another window and insert widgets into it
	    Window *anotherWindow = new Window(this, "Widgets");
        anotherWindow->setPosition(Vector2i(485, 15));
        anotherWindow->setLayout(new GroupLayout());

	    // Open and save obj file
        new Label(anotherWindow, "File dialog", "sans-bold", 20);
        Button *openBtn  = new Button(anotherWindow, "Open");
        openBtn->setCallback([&] {
            string fileName = file_dialog({ {"obj", "obj file"} }, false);
            ObjViewApp::fileName = fileName;
            mCanvas->loadObj(fileName);
        });
        Button *saveBtn = new Button(anotherWindow, "Save");
        saveBtn->setCallback([&] {
            string fileName = file_dialog({ {"obj", "obj file"} }, true);
            mCanvas->writeObj(fileName);
        });

        // Subdivision
        new Label(anotherWindow, "Subdivision Option", "sans-bold", 20);
        ComboBox *comboSd = new ComboBox(anotherWindow, { "Loop", "Butterfly" } );
        comboSd->setCallback([&](int value) {
            sdMode = value;
        });

        // Subdivision panel
	    Widget *panelSdLevel = new Widget(anotherWindow);
        panelSdLevel->setLayout(new BoxLayout(Orientation::Horizontal,
                                        Alignment::Middle, 0, 2));
        // Initiate subdivision slider
        Slider *sldSdLevel = new Slider(panelSdLevel);
        sldSdLevel->setValue(0.0f);
        sldSdLevel->setFixedWidth(220);
        TextBox *sldSdLevelTxt = new TextBox(panelSdLevel);
        sldSdLevelTxt->setFixedSize(Vector2i(60, 25));
        sldSdLevelTxt->setValue("0");
        sldSdLevelTxt->setUnits(" Step");
        sldSdLevelTxt->setFixedSize(Vector2i(60,25));
        sldSdLevelTxt->setFontSize(20);
        sldSdLevelTxt->setAlignment(TextBox::Alignment::Right);

        sldSdLevel->setCallback([&, sldSdLevelTxt](float value) {
            sdLevel = (int)round(value * 5);
            sldSdLevelTxt->setValue(std::to_string(sdLevel));
        });

        Button *btnRunSd = new Button(anotherWindow, "Run Subdivision");
        btnRunSd->setCallback([&] {
            mCanvas->loadObjSd(fileName, sdLevel, sdMode);
        });

        // Rotation panel
        new Label(anotherWindow, "Rotation", "sans-bold", 20);
	    Widget *panelRot = new Widget(anotherWindow);
        panelRot->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));

        // Initiate rotation sliders
	    Slider *rotSlider_X = new Slider(panelRot);
	    new Label(panelRot, "X");
        Slider *rotSlider_Y = new Slider(panelRot);
	    new Label(panelRot, "Y");
	    Slider *rotSlider_Z = new Slider(panelRot);
	    new Label(panelRot, "Z");

        // Rotation along X axis
        rotSlider_X->setValue(0.5f);
        rotSlider_X->setFixedWidth(80);
        rotSlider_X->setCallback([&, rotSlider_Y, rotSlider_Z](float value) {
	        float radians_X = (value - 0.5f)*2*2*M_PI;
            float radians_Y = (rotSlider_Y->value() - 0.5f)*2*2*M_PI;
            float radians_Z = (rotSlider_Z->value() - 0.5f)*2*2*M_PI;          
	        mCanvas->setRotation(nanogui::Vector3f(radians_X, radians_Y, radians_Z));
        });

	    // Rotation along Y axis
        rotSlider_Y->setValue(0.5f);
        rotSlider_Y->setFixedWidth(80);
        rotSlider_Y->setCallback([&, rotSlider_X, rotSlider_Z](float value) {
            float radians_X = (rotSlider_X->value() - 0.5f)*2*2*M_PI;
	        float radians_Y = (value - 0.5f)*2*2*M_PI;
            float radians_Z = (rotSlider_Z->value() - 0.5f)*2*2*M_PI;
	        mCanvas->setRotation(nanogui::Vector3f(radians_X, radians_Y, radians_Z));
        });

	    // Rotation along Z axis
        rotSlider_Z->setValue(0.5f);
        rotSlider_Z->setFixedWidth(80);
        rotSlider_Z->setCallback([&, rotSlider_X, rotSlider_Y](float value) {
            float radians_X = (rotSlider_X->value() - 0.5f)*2*2*M_PI;
            float radians_Y = (rotSlider_Y->value() - 0.5f)*2*2*M_PI;
	        float radians_Z = (value - 0.5f)*2*2*M_PI;
	        mCanvas->setRotation(nanogui::Vector3f(radians_X, radians_Y, radians_Z));
        });

        // Translation panel
        new Label(anotherWindow, "Translation", "sans-bold", 20);
	    Widget *panelTrans = new Widget(anotherWindow);
        panelTrans->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));

        // Initiate translation sliders
	    Slider *tranSlider_X = new Slider(panelTrans);
	    new Label(panelTrans, "X");
        Slider *tranSlider_Y = new Slider(panelTrans);
	    new Label(panelTrans, "Y");
	    Slider *tranSlider_Z = new Slider(panelTrans);
	    new Label(panelTrans, "Z");

        //Translation along X axis
        tranSlider_X->setValue(0.5f);
        tranSlider_X->setFixedWidth(80);
        tranSlider_X->setCallback([&, tranSlider_Y, tranSlider_Z](float value) {
            float trans_X = (value - 0.5f)*2*4;
            float trans_Y = (tranSlider_Y->value() - 0.5f)*2*4;
            float trans_Z = (tranSlider_Z->value() - 0.5f)*2*4;          
	        mCanvas->setTranslation(nanogui::Vector3f(trans_X, trans_Y, trans_Z));
        });

        //Translation along Y axis
        tranSlider_Y->setValue(0.5f);
        tranSlider_Y->setFixedWidth(80);
        tranSlider_Y->setCallback([&, tranSlider_X, tranSlider_Z](float value) {
            float trans_X = (tranSlider_X->value() - 0.5f)*2*4;
            float trans_Y = (value - 0.5f)*2*4;
            float trans_Z = (tranSlider_Z->value() - 0.5f)*2*4;          
	        mCanvas->setTranslation(nanogui::Vector3f(trans_X, trans_Y, trans_Z));
        });

        //Translation along Z axis
        tranSlider_Z->setValue(0.5f);
        tranSlider_Z->setFixedWidth(80);
        tranSlider_Z->setCallback([&, tranSlider_X, tranSlider_Y](float value) {
            float trans_X = (tranSlider_X->value() - 0.5f)*2*4; 
            float trans_Y = (tranSlider_Y->value() - 0.5f)*2*4;
            float trans_Z = (value - 0.5f)*2*4;        
	        mCanvas->setTranslation(nanogui::Vector3f(trans_X, trans_Y, trans_Z));
        });

        mCanvas->setTranslation(nanogui::Vector3f(0, 0, 0));

        // Zomming panel
	    new Label(anotherWindow, "Zoom", "sans-bold", 20);
	    Widget *panelZoom = new Widget(anotherWindow);
        panelZoom->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));

        // Initiate zooming slider
        Slider *zoom = new Slider(panelZoom);
        zoom->setValue(0.5f);
        zoom->setFixedWidth(150);
        TextBox *zoomTxt = new TextBox(panelZoom);
        zoomTxt->setFixedSize(Vector2i(60, 25));
        zoomTxt->setValue("50");
        zoomTxt->setUnits("%");
        zoom->setCallback([&, zoomTxt](float value) {
	        mCanvas->setZooming(value);
            zoomTxt->setValue(std::to_string((int) (value * 100)));
        });
        zoomTxt->setFixedSize(Vector2i(60,25));
        zoomTxt->setFontSize(20);
        zoomTxt->setAlignment(TextBox::Alignment::Right);

    	// Shading mode
        new Label(anotherWindow, "Shading Mode", "sans-bold", 20);
        Widget *panelCombo = new Widget(anotherWindow);
        panelCombo->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));

        ComboBox *combo = new ComboBox(anotherWindow, { "Flat", "Smooth", "Wireframe", "Flat+Wireframe", "Smooth+Wireframe"} );
        combo->setCallback([&](int value) {
            mCanvas->setShadingMode(value);
        });

        // Quit button
        new Label(anotherWindow, "Quit", "sans-bold", 20);
        Button *quitBtn = new Button(anotherWindow, "Quit");
        quitBtn->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Quit", "Are you sure to shut down?", "No", "Yes", true);
            dlg->setCallback([](int result) { 
                if(result == 1){
                    nanogui::shutdown();
                }});
        });

        // Create another window and insert widgets into it
	    Window *mcdWindow = new Window(this, "Mesh Decimation");
        mcdWindow->setPosition(Vector2i(865, 15));
        mcdWindow->setLayout(new GroupLayout());

	    Widget *panelK = new Widget(mcdWindow);
        panelK->setLayout(new BoxLayout(Orientation::Horizontal,
                                        Alignment::Middle, 0, 10));
        Label *lbK = new Label(panelK, "k");
        lbK->setFixedSize(Vector2i(75, 20));
        TextBox *txtK = new TextBox(panelK);
        txtK->setFixedSize(Vector2i(90, 25));
        txtK->setEditable(true);
        txtK->setValue("8");

        Widget *panelN = new Widget(mcdWindow);
        panelN->setLayout(new BoxLayout(Orientation::Horizontal,
                                        Alignment::Middle, 0, 10));
        Label *lbN = new Label(panelN, "N to Collapse");
        lbN->setFixedSize(Vector2i(75, 20));
        TextBox *txtN = new TextBox(panelN);
        txtN->setFixedSize(Vector2i(90, 25));
        txtN->setEditable(true);
        txtN->setValue("100");

        // Edge collapses
        Button *mcdButton  = new Button(mcdWindow, "Decimate");
        mcdButton->setCallback([&, txtK, txtN] {
            int k = stoi(txtK->value());
            int n = stoi(txtN->value());
            mCanvas->loadObjMcd(fileName, k, n);
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


private:
    MyGLCanvas *mCanvas;
    string fileName;
    int sdMode = 0;
    int sdLevel = 0;
};

int main(int /* argc */, char ** /* argv */) {
    try {
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
