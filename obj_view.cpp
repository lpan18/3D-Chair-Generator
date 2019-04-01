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

#include "ChairMixer.h"
#include "Mesh.h"

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

        setIsometricView();
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

    // Temp test method
    void tempTest() {
        string path = "ChairModels";
        ChairMixer mixer;
        mixer.readFolder(path);
        ObjBuffer mixed = mixer.tempTest();

        delete mMesh;
        mMesh = new Mesh(mixed);

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

    void setFrontView() {
        mMVP << -0.5, 0, 0, 0,
                0, 0, 0.5, 0,
                0, 0.5, 0, 0,
                0, 0, 0, 1;
    }

    void setSideView() {
        mMVP << 0, -0.5, 0, 0,
                0, 0, 0.5, 0,
                -0.5, 0, 0, 0,
                0, 0, 0, 1;
    }

    void setTopView() {
        mMVP << -0.5, 0, 0, 0,
                0, 0.5, 0, 0,
                0, 0, -0.5, 0,
                0, 0, 0, 1;
    }

    void setIsometricView() {
        mMVP << -0.379379, -0.325686, -1.49012e-08, 0,
                -0.0816341, 0.0950923, 0.484039, 0,
                -0.315289, 0.367268, -0.125326, 0,
                0, 0, 0, 1;
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

        // Reset MVP
        mShader.setUniform("MVP", mMVP);

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
    Matrix4f mMVP;
    MatrixXf positions;
    MatrixXf normals;
    MatrixXf smoothNormals;
    MatrixXf colors;
    nanogui::GLShader mShader;
    int mShadingMode = 0;
};

class ObjViewApp : public nanogui::Screen {
public:
    ObjViewApp() : nanogui::Screen(Eigen::Vector2i(1000, 600), "Chair Modeling", false) {
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
        Button *testBtn = new Button(anotherWindow, "Test");
        testBtn->setCallback([&] {
            mCanvas->tempTest();
        });

        new Label(anotherWindow, "Views", "sans-bold", 20);
        Widget *panelViews = new Widget(anotherWindow);
        panelViews->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));
        
        Button *frontView = new Button(panelViews, "Front");
        frontView->setCallback([&] {
            mCanvas->setFrontView();
        });

        Button *sideView = new Button(panelViews, "Side");
        sideView->setCallback([&] {
            mCanvas->setSideView();
        });

        Button *topView = new Button(panelViews, "Top");
        topView->setCallback([&] {
            mCanvas->setTopView();
        });

        Button *isometricView = new Button(panelViews, "Isometric");
        isometricView->setCallback([&] {
            mCanvas->setIsometricView();
        });

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
