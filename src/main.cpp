/*===============================================================================================
geometry.exe main.cpp
Copyright (c), Firelight Technologies Pty, Ltd 2005-2014.

Example to show occlusion
===============================================================================================*/
#include "pch.h"

// keyboard control
bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;
bool moveUp = false;
bool moveDown = false;
bool isRun = false;

// mouse control
bool doRotate = false;
int xMouse = 0;
int yMouse = 0;

// window size
int width = 768;
int height = 480;
int window_pos_x = 200;
int window_pos_y = 50;

float accumulatedTime = 0.0f;
float INTERFACE_UPDATETIME = 16.67f; // milliseconds

//debug text
unsigned int useFont = 1; // 0 - no, 1 - stroke, 2 - bitmap;


void mouseFunc(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN) {
            doRotate = true;
            xMouse = x;
            yMouse = y;
        } else if (state == GLUT_UP) {
            doRotate = false;
        }
        break;
    default:
        break;
    }
}

void motionFunc(int x, int y)
{
    int dx = x - xMouse;
    int dy = y - yMouse;

    // view rotation about y-axis
    yRotation += (float)dx * 0.5f;
    if (yRotation > 180.0f)
        yRotation -= 360.0f;
    else
        if (yRotation < -180.0f)
            yRotation += 360.0f;

    // view rotation about x-axis
    const float xExtent = 88.0f;
    xRotation += (float)dy * 0.5f;
    if (xRotation > xExtent)
        xRotation = xExtent;
    else
        if (xRotation < -xExtent)
            xRotation = -xExtent;

    xMouse = x;
    yMouse = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    //printf("'%c' pressed at (%i,%i) \n", key, x, y);
    switch(key) {
    case 'W':
    case 'w': moveForward = true; break;
    case 'S':
    case 's': moveBackward = true; break;
    case 'A':
    case 'a': moveLeft = true; break;
    case 'D':
    case 'd': moveRight = true; break;
    case ' ': moveUp = true; break;
    case 'C':
    case 'c': moveDown = true; break;
    case 'f': (useFont < 2) ? useFont++ : useFont = 0; break;
    }
}

void keyboardUpFunc(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    //printf("'%c' up at (%i,%i) \n", key, x, y);
    switch(key) {
    case 'W':
    case 'w': moveForward = false; break;
    case 'S':
    case 's': moveBackward = false; break;
    case 'A':
    case 'a': moveLeft = false; break;
    case 'D':
    case 'd': moveRight = false; break;
    case ' ': moveUp = false; break;
    case 'C':
    case 'c': moveDown = false; break;
    case 27 : glutLeaveMainLoop(); break;
    }
}

void specialFunc(int s, int x, int y)
{
    (void)x; (void)y;
    //printf("'%i' pressed at (%i,%i) \n", s, x, y);
    switch(s) {
    case 112: isRun = true; break;
    }
}

void specialFuncUp(int s, int x, int y)
{
    (void)x; (void)y;
    //printf("'%i' up at (%i,%i) \n", s, x, y);
    switch(s) {
    case 112: isRun = false; break;
    }
}


void init(void)
{
    audioInit();
    modelInit();
    audioDoorInit();
    videoInit();
}

bool enableVSync(int interval)
{
    const unsigned char *extensions = glGetString(GL_EXTENSIONS);
    if(strstr((const char *)extensions, "WGL_EXT_swap_control") != 0) {
        typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
        PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");
        if(wglSwapIntervalEXT) {
            wglSwapIntervalEXT(interval);
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    //glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); // ???

    glutInitWindowSize(width, height);
    glutInitWindowPosition(window_pos_x, window_pos_y);
    glutCreateWindow("FMOD Geometry example.");

    if (!enableVSync(1))
        printf("VSync not enabled\n");

    glutReshapeFunc(reshapeFunc);
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutKeyboardFunc(keyboardFunc);
    glutKeyboardUpFunc(keyboardUpFunc);
    glutSpecialFunc(specialFunc);
    glutSpecialUpFunc(specialFuncUp);

    init();

    glutMainLoop();

    return 0;
}

