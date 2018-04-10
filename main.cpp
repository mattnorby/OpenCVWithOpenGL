/*
 * OpenCV with OpenGL
 * by Matt Norby, April 2018
 *
 * This program uses OpenCV to load an image from disk.
 * The image file name should be the first command-line argument to the program.
 * The image is converted to an OpenGL texture, and applied to a rectangle.
 * An untextured 3D object (a gem) is positioned in front of the rectangle.
 * The camera moves in and out, to show perspective.
 *
 * Press Esc to stop the animation.
 *
 * While the animation itself is not very interesting, it proves a
 * couple of concepts:
 * 1. It is possible to load an image from OpenCV and use it as an OpenGL texture.
 * 2. It is possible to position the image in a window, and display content on top of it.
 *
 * What is the ultimate goal of this experiment?  Augmented reality.
 * Suppose we already have a calibrated video camera.
 * Imagine capturing a frame of video using OpenCV.
 * Detect a marker (or other reference points) and determine orientation in 3D.
 * Use the frame as an OpenGL texture for a rectangle that fills the screen.
 * Place a 3D object (possibly animated) on the marker, oriented correctly in 3D.
 *
 * Imagine substituting a live video frame for the image, and then animating
 * a 3D object on top of it, oriented properly to match a detected marker in
 * the video.  That would be augmented reality.
 */

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/opengl.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
using namespace cv;
using namespace std;

String imageFile;
int width, height;
GLfloat zOffset = -5.0;
GLfloat zDelta = -0.003125;
GLuint texName, startList;

/**
 * Perform initial setup for the application:
 * 1. Load an image from OpenCV.
 * 2. Create an OpenGL texture from the image.
 * 3. Assign material properties and set up lighting
 * 3. Prepare display lists with all primitives needed for rendering.
 */
void init() {
    // Load an image, using OpenCV
    Mat img = imread(imageFile, CV_LOAD_IMAGE_COLOR);
    if (img.empty()) {
        cout << "Unable to read image: " << imageFile << endl;
        exit(-1);
    }
    width = img.cols;
    height = img.rows;

    // Create an OpenGL texture, using the data from the OpenCV image
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.ptr());

    // Material properties for the gem
    GLfloat mat_ambient[] = { 0.1, 0.1, 0.8, 1.0 };
    GLfloat mat_specular[] = { 0.8, 0.8, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };

    // Lighting properties
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

    glClearColor(0.0, 0.0, 0.0, 0.0);

    // Assign the material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // Assign the lighting properties, and enable lighting
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Tell OpenGL to check for occlusions
    glEnable(GL_DEPTH_TEST);

    // Compile two display lists, for later use
    startList = glGenLists(2);

    // List #1: Gem
    glNewList(startList, GL_COMPILE);
        glBegin(GL_TRIANGLES);
            glNormal3f(-0.22663, -0.84565, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // ABC (triangles)
            glVertex3f( 0.0,   -1.0,    1.75);
            glVertex3f(-0.5,   -0.866,  1.75);
            glNormal3f(-0.61907, -0.61907, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // ACD
            glVertex3f(-0.5,   -0.866,  1.75);
            glVertex3f(-0.866, -0.5,    1.75);
            glNormal3f(-0.84565, -0.22663, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // ADE
            glVertex3f(-0.866, -0.5,    1.75);
            glVertex3f(-1.0,    0.0,    1.75);
            glNormal3f(-0.84565, 0.22663, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AEF
            glVertex3f(-1.0,    0.0,    1.75);
            glVertex3f(-0.866,  0.5,    1.75);
            glNormal3f(-0.61907, 0.61907, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AFG
            glVertex3f(-0.866,  0.5,    1.75);
            glVertex3f(-0.5,    0.866,  1.75);
            glNormal3f(-0.22663, 0.84565, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AGH
            glVertex3f(-0.5,    0.866,  1.75);
            glVertex3f( 0.0,    1.0,    1.75);
            glNormal3f(0.22663, 0.84565, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AHI
            glVertex3f( 0.0,    1.0,    1.75);
            glVertex3f( 0.5,    0.866,  1.75);
            glNormal3f(0.61907, 0.61907, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AIJ
            glVertex3f( 0.5,    0.866,  1.75);
            glVertex3f( 0.866,  0.5,    1.75);
            glNormal3f(0.84565, 0.22663, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AJK
            glVertex3f( 0.866,  0.5,    1.75);
            glVertex3f( 1.0,    0.0,    1.75);
            glNormal3f(0.84565, -0.22663, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AKL
            glVertex3f( 1.0,    0.0,    1.75);
            glVertex3f( 0.866, -0.5,    1.75);
            glNormal3f(0.61907, -0.61907, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // ALM
            glVertex3f( 0.866, -0.5,    1.75);
            glVertex3f( 0.5,   -0.866,  1.75);
            glNormal3f(0.22663, -0.84565, -0.48323);
            glVertex3f( 0.0,    0.0,    0.0 ); // AMB
            glVertex3f( 0.5,   -0.866,  1.75);
            glVertex3f( 0.0,   -1.0,    1.75);
            glNormal3f(-0.18619, -0.69474, 0.69474);

            glVertex3f(-0.5,   -0.866,  1.75); // CBNO (quads, as triangles)
            glVertex3f( 0.0,   -1.0,    1.75);
            glVertex3f( 0.0,   -0.75,   2.0 );
            glVertex3f(-0.5,   -0.866,  1.75);
            glVertex3f( 0.0,   -0.75,   2.0 );
            glVertex3f(-0.375, -0.6495, 2.0 );
            glNormal3f(-0.50589, -0.50589, 0.69474);
            glVertex3f(-0.866, -0.5,    1.75); // DCOP
            glVertex3f(-0.5,   -0.866,  1.75);
            glVertex3f(-0.375, -0.6495, 2.0 );
            glVertex3f(-0.866, -0.5,    1.75);
            glVertex3f(-0.375, -0.6495, 2.0 );
            glVertex3f(-0.6495, -0.375, 2.0 );
            glNormal3f(-0.69474, -0.18619, 0.69474);
            glVertex3f(-1.0,    0.0,    1.75); // EDPQ
            glVertex3f(-0.866, -0.5,    1.75);
            glVertex3f(-0.6495, -0.375, 2.0 );
            glVertex3f(-1.0,    0.0,    1.75);
            glVertex3f(-0.6495, -0.375, 2.0 );
            glVertex3f(-0.75,   0.0,    2.0 );
            glNormal3f(-0.69474, 0.18619, 0.69474);
            glVertex3f(-0.866,  0.5,    1.75); // FEQR
            glVertex3f(-1.0,    0.0,    1.75);
            glVertex3f(-0.75,   0.0,    2.0 );
            glVertex3f(-0.866,  0.5,    1.75);
            glVertex3f(-0.75,   0.0,    2.0 );
            glVertex3f(-0.6495, 0.375,  2.0 );
            glNormal3f(-0.50589, 0.50589, 0.69474);
            glVertex3f(-0.5,    0.866,  1.75); // GFRS
            glVertex3f(-0.866,  0.5,    1.75);
            glVertex3f(-0.6495, 0.375,  2.0 );
            glVertex3f(-0.5,    0.866,  1.75);
            glVertex3f(-0.6495, 0.375,  2.0 );
            glVertex3f(-0.375,  0.6495, 2.0 );
            glNormal3f(-0.18619, 0.69474, 0.69474);
            glVertex3f( 0.0,    1.0,    1.75); // HGST
            glVertex3f(-0.5,    0.866,  1.75);
            glVertex3f(-0.375,  0.6495, 2.0 );
            glVertex3f( 0.0,    1.0,    1.75);
            glVertex3f(-0.375,  0.6495, 2.0 );
            glVertex3f( 0.0,    0.75,   2.0 );
            glNormal3f(0.18619, 0.69474, 0.69474);
            glVertex3f( 0.5,    0.866,  1.75); // IHTU
            glVertex3f( 0.0,    1.0,    1.75);
            glVertex3f( 0.0,    0.75,   2.0 );
            glVertex3f( 0.5,    0.866,  1.75);
            glVertex3f( 0.0,    0.75,   2.0 );
            glVertex3f( 0.375,  0.6495, 2.0 );
            glNormal3f(0.50589, 0.50589, 0.69474);
            glVertex3f( 0.866,  0.5,    1.75); // JIUV
            glVertex3f( 0.5,    0.866,  1.75);
            glVertex3f( 0.375,  0.6495, 2.0 );
            glVertex3f( 0.866,  0.5,    1.75);
            glVertex3f( 0.375,  0.6495, 2.0 );
            glVertex3f( 0.6495, 0.375,  2.0 );
            glNormal3f(0.69474, 0.18619, 0.69474);
            glVertex3f( 1.0,    0.0,    1.75); // KJVW
            glVertex3f( 0.866,  0.5,    1.75);
            glVertex3f( 0.6495, 0.375,  2.0 );
            glVertex3f( 1.0,    0.0,    1.75);
            glVertex3f( 0.6495, 0.375,  2.0 );
            glVertex3f( 0.75,   0.0,    2.0 );
            glNormal3f(0.69474, -0.18619, 0.69474);
            glVertex3f( 0.866, -0.5,    1.75); // LKWX
            glVertex3f( 1.0,    0.0,    1.75);
            glVertex3f( 0.75,   0.0,    2.0 );
            glVertex3f( 0.866, -0.5,    1.75);
            glVertex3f( 0.75,   0.0,    2.0 );
            glVertex3f( 0.6495, -0.375, 2.0 );
            glNormal3f(0.50589, -0.50589, 0.69474);
            glVertex3f( 0.5,   -0.866,  1.75); // MLXY
            glVertex3f( 0.866, -0.5,    1.75);
            glVertex3f( 0.6495, -0.375, 2.0 );
            glVertex3f( 0.5,   -0.866,  1.75);
            glVertex3f( 0.6495, -0.375, 2.0 );
            glVertex3f( 0.375, -0.6495, 2.0 );
            glNormal3f(0.18619, -0.69474, 0.69474);
            glVertex3f( 0.0,   -1.0,    1.75); // BMYN
            glVertex3f( 0.5,   -0.866,  1.75);
            glVertex3f( 0.375, -0.6495, 2.0 );
            glVertex3f( 0.0,   -1.0,    1.75);
            glVertex3f( 0.375, -0.6495, 2.0 );
            glVertex3f( 0.0,   -0.75,   2.0 );

            glNormal3f( 0.0,    0.0,    1.0 ); // polygon NOPQRSTUVWXY (as triangles)
            glVertex3f( 0.375, -0.65,   2.0 ); // YXW
            glVertex3f( 0.65,  -0.375,  2.0 );
            glVertex3f( 0.75,   0.0,    2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YWV
            glVertex3f( 0.75,   0.0,    2.0 );
            glVertex3f( 0.65,   0.375,  2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YVU
            glVertex3f( 0.65,   0.375,  2.0 );
            glVertex3f( 0.375,  0.65,   2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YUT
            glVertex3f( 0.375,  0.65,   2.0 );
            glVertex3f( 0.0,    0.75,   2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YTS
            glVertex3f( 0.0,    0.75,   2.0 );
            glVertex3f(-0.375,  0.65,   2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YSR
            glVertex3f(-0.375,  0.65,   2.0 );
            glVertex3f(-0.65,   0.375,  2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YRQ
            glVertex3f(-0.65,   0.375,  2.0 );
            glVertex3f(-0.75,   0.0,    2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YQP
            glVertex3f(-0.75,   0.0,    2.0 );
            glVertex3f(-0.65,  -0.375,  2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YPO
            glVertex3f(-0.65,  -0.375,  2.0 );
            glVertex3f(-0.375, -0.65,   2.0 );
            glVertex3f( 0.375, -0.65,   2.0 ); // YON
            glVertex3f(-0.375, -0.65,   2.0 );
            glVertex3f( 0.0,   -0.75,   2.0 );
        glEnd();
    glEndList();

    // List #2: Rectangle with texture
    glNewList(startList + 1, GL_COMPILE);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 0.0);
            glVertex3f(-2.0, 2.0, 0.0);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(-2.0, -2.0, 0.0);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(2.0, -2.0, 0.0);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(2.0, 2.0, 0.0);
        glEnd();
    glEndList();
}

/**
 * Set the projection matrix, when the OpenGL context window changes size.
 * This method is also called when the window is created.
 * @param w the new width of the window
 * @param h the new height of the window
 */
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,            // zoom factor
                   (double) w / h,  // aspect ratio
                   1.0,             // near clipping plane
                   100.0);          // far clipping plane

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
 * Render the 3D scene.  This function is called repeatedly by OpenGL.
 */
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the camera position
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, zOffset);

    // Update the camera position, for the next loop
    zOffset += zDelta;
    if (zOffset >= -5.0 && zDelta > 0) zDelta = -zDelta;
    if (zOffset <= -10.0 && zDelta < 0) zDelta = -zDelta;

    // Disable lighting and enable textures, then render the rectangle
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texName);
    glCallList(startList + 1);

    // Disable textures and enable lighting, then render the gem
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glCallList(startList);

    // Tell OpenGL that the window should be repainted
    glFlush();
    glutPostRedisplay();
}

/**
 * Keyboard callback - invoked when a key is pressed.
 * Exits the program when the user presses Esc.
 */
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[]) {
    // Get the image file name from the command line
    if (argc < 2) {
        cout << "Please specify the image file name as the first program argument" << endl;
        return -1;
    }
    imageFile = argv[1];

    // Initialize OpenGL, and create a context
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenCV with OpenGL");

    // Perform initial setup
    init();

    // Assign callback functions
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);

    // Tell OpenGL to start rendering
    glutMainLoop();

    return EXIT_SUCCESS;
}
