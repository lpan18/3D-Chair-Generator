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
// List files
#include <experimental/filesystem>

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
        mArcball.setSize({400,400});

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
        float near = 0.1f, far = 100.0f;
        float top = near * tan(fovy * M_PI / 360);
        float bottom = -top;
        float right = top * aspect;
        float left = -right;
        Matrix4f P = frustum(left, right, bottom, top, near, far);

        mShader.setUniform("P", P);

        // ViewMatrixID
        Matrix4f V = lookAt(Vector3f(0,0,3), Vector3f(0,0,0), Vector3f(0,1,0));
        mShader.setUniform("V", V);

        mTranslation = Vector3f(0, 0, 0);
        mZoom = Vector3f(1.5, 1.5, 1.5);

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

        MatrixXf shadingNormal = (mShadingMode == 1 || mShadingMode == 4)  ? smoothNormals : normals;
	    //dates the positions matrix,color and indices matrices
        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);
	    mShader.uploadAttrib("vertexNormal_modelspace", shadingNormal);

        //ModelMatrixID
        setRotation(mArcball.matrix());
        Matrix4f M = translate(mTranslation) * mRotation * scale(mZoom);
        mShader.setUniform("M", M);

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
    Eigen::Matrix4f mRotation;
    Eigen::Vector3f mZoom;
    Eigen::Vector3f mTranslation;
    nanogui::Arcball mArcball;
    int mShadingMode = 0;
};


class ObjViewApp : public nanogui::Screen {
public:
    ObjViewApp() : nanogui::Screen(Eigen::Vector2i(800, 600), "Chair Modeling", false) {
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
	    Window *widgets = new Window(this, "Widgets");
        widgets->setPosition(Vector2i(485, 15));
        widgets->setLayout(new GroupLayout());

	    // Open and save obj file
        new Label(widgets, "File dialog", "sans-bold", 20);
        Button *openBtn  = new Button(widgets, "Open");
        openBtn->setCallback([&] {
            string fileName = file_dialog({ {"obj", "obj file"} }, false);
            ObjViewApp::fileName = fileName;
            mCanvas->loadObj(fileName);
        });
        Button *saveBtn = new Button(widgets, "Save");
        saveBtn->setCallback([&] {
            string fileName = file_dialog({ {"obj", "obj file"} }, true);
            mCanvas->writeObj(fileName);
        });
         
        Button *testBtn = new Button(widgets, "Test");
        testBtn->setCallback([&] {
            string folder = "Completion/";       
            for(size_t i = 1; i <= 5; i++)
            {  
                string filename1 = folder + to_string(i) + ".obj";      
                mCanvas->tempTest();
                mCanvas->writeObj(filename1);
            }
            for (const auto & entry : experimental::filesystem::directory_iterator(folder))
                cout << entry.path() << endl;
                // Button *file1 = new Button(widgets, "1.obj");
                // file1->setCallback([&] {
                //     ObjViewApp::fileName = "Completion/1.obj";
                //     mCanvas->loadObj(fileName);
                // });

        });            

        Button *file2 = new Button(widgets, "2.obj");
        file2->setCallback([&] {
            ObjViewApp::fileName = "Completion/2.obj";
            mCanvas->loadObj(fileName);
        });
        Button *file3 = new Button(widgets, "3.obj");
        file3->setCallback([&] {
            ObjViewApp::fileName = "Completion/3.obj";
            mCanvas->loadObj(fileName);
        });
        Button *file4 = new Button(widgets, "4.obj");
        file4->setCallback([&] {
            ObjViewApp::fileName = "Completion/4.obj";
            mCanvas->loadObj(fileName);
        });
        Button *file5 = new Button(widgets, "5.obj");
        file5->setCallback([&] {
            ObjViewApp::fileName = "Completion/5.obj";
            mCanvas->loadObj(fileName);
        });
        
        new Label(widgets, "Views", "sans-bold", 20);
        Widget *panelViews = new Widget(widgets);
        panelViews->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));
        
    	// Shading mode
        new Label(widgets, "Shading Mode", "sans-bold", 20);
        Widget *panelCombo = new Widget(widgets);
        panelCombo->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 2));

        ComboBox *combo = new ComboBox(widgets, { "Flat", "Smooth", "Wireframe", "Flat+Wireframe", "Smooth+Wireframe"} );
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


private:
    MyGLCanvas *mCanvas;
    string fileName;
    int sdMode = 0;
    int sdLevel = 0;
};

int main(int /* argc */, char ** /* argv */) {
    try {
        srand (time(NULL));
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
