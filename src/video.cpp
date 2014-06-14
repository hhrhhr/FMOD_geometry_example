#include "pch.h"

GLuint texture;

GLuint loadTexture(const char * filename)
{
    GLuint texture;
    int width;
    int height;
    unsigned char *data;
    FILE *file;

    // open texture data
    errno_t err = fopen_s(&file, filename, "rb");
    if (err != 0)
        return 0;

    width = 128;
    height = 128;
    data = (unsigned char*)malloc(width * height * 3);

    fread(data, width * height * 3, 1, file );
    fclose(file);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    free(data);

    return texture;
}

void videoInit()
{
    texture = loadTexture("../share/media/texture.img");

    // setup lighting
    GLfloat lightDiffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightPosition[] = {300.0, 1000.0, 400.0, 0.0};
    GLfloat lightAmbient[] = {1.25, 1.25, 1.25, 1.0};

    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawGeometry(Mesh& mesh)
{
    FMOD_RESULT result;

    FMOD_VECTOR pos;
    result = mesh.geometry->getPosition(&pos);
    ERRCHECK(result);

    glPushMatrix();
    // create matrix and set gl transformation for geometry
    glTranslatef(pos.x, pos.y, pos.z);
    FMOD_VECTOR forward;
    FMOD_VECTOR up;
    result = mesh.geometry->getRotation(&forward, &up);
    ERRCHECK(result);
    float matrix[16] = {
        up.y * forward.z - up.z * forward.y, up.x, forward.x, 0.0f,
        up.z * forward.x - up.x * forward.z, up.y, forward.y, 0.0f,
        up.x * forward.y - up.y * forward.x, up.z, forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    glMultMatrixf(matrix);

    // draw all polygons in object
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    for (int poly = 0; poly < mesh.numPolygons; poly++) {
        Poly* polygon = &mesh.polygons[poly];
        if (polygon->directOcclusion == 0.0f)
            continue; // don't draw because it is an open door way

        glBegin(GL_TRIANGLE_FAN);
        glNormal3fv(&polygon->normal.x);

        for (int i = 0; i < polygon->numVertices; i++) {
            int index = mesh.indices[polygon->indicesOffset + i];
            glTexCoord2f(mesh.texcoords[index][0], mesh.texcoords[index][1]);
            glVertex3fv(&mesh.vertices[index].x);
        }
        glEnd();
    }
    glPopMatrix();
}

static void glPrintF (int row, int col, const char *fmt, ...)
{
    if (useFont == 0) return;

    const float scale = 0.09f;

    static char buf[128];
    void *strokeFont = GLUT_STROKE_MONO_ROMAN;
    void *bitmapFont = GLUT_BITMAP_9_BY_15;

    static float charWidth = glutStrokeWidth(strokeFont, 35) * scale;
    static float charHeight = glutStrokeHeight(strokeFont) * scale;

    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buf, 128, fmt, args);
    va_end(args);

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 0.0);
    switch(useFont) {
    case 1:
        //StrokeString
        glTranslatef(charWidth * col, height - charHeight * row, 0.0);
        glScalef(scale, scale, 1);
        glutStrokeString(strokeFont, (const unsigned char*)buf);
        break;
    case 2:
        //BitmapString
        glRasterPos2i(glutBitmapWidth(bitmapFont, ' ') * col, height - glutBitmapHeight(bitmapFont) * row);
        glutBitmapString (bitmapFont, (const unsigned char*)buf);
        break;
    default:
        break;
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void idle()
{
    static unsigned int fpsCounter = 0;
    static float fps = 0.0f;
    static float prevTime = (float)glutGet(GLUT_ELAPSED_TIME); //(float)clock();

    fpsCounter++;

    float currTime = (float)glutGet(GLUT_ELAPSED_TIME); //(float)clock();
    accumulatedTime = currTime / 1000.0f;
    float delta = currTime - prevTime;

    if (delta > 1000.0f) {
        INTERFACE_UPDATETIME = delta / (float)fpsCounter;
        fps = 1000.0f / INTERFACE_UPDATETIME;
        prevTime = currTime;
        fpsCounter = 0;
    }

    doGeometryMovement();
    doSoundMovement();
    doListenerMovement();

    ERRCHECK(fmodSystem->update());


    float dsp, stream, geom, update, total;
    int channels;

    fmodSystem->getCPUUsage(&dsp, &stream, &geom, &update, &total);
    fmodSystem->getChannelsPlaying(&channels);

    glPrintF(1, 1, "FPS : %02.1f", fps);
    glPrintF(2, 1, "FMOD CPU load : %.3f", total);
    glPrintF(3, 1, " | DSP : %.3f", dsp);
    glPrintF(4, 1, " | stream : %.3f", stream);
    glPrintF(5, 1, " | geometry: %.3f", geom);
    glPrintF(6, 1, " | update : %.3f", update);
    glPrintF(7, 1, "active channels : %i", channels);

    glutSwapBuffers();
    glutPostRedisplay();
}

void display(void)
{
    // update view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)width / (float)height, 0.1, 500.0);

    glRotatef(xRotation, 1.0f, 0.0f, 0.0f);
    glRotatef(yRotation, 0.0f, 1.0f, 0.0f);
    glTranslatef(-xListenerPos, -yListenerPos, -zListenerPos);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.4, 0.6f, 1.0f, 0.0f);

    // draw geometry
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    drawGeometry(walls);
    drawGeometry(rotatingMesh);
    drawGeometry(doorList[0]);
    drawGeometry(doorList[1]);
    drawGeometry(doorList[2]);
    drawGeometry(doorList[3]);

    glDisable(GL_TEXTURE_2D);

    // draw sound objects
    for (int object = 0; object < NUM_OBJECTS; object++) {
        float audibility = 1.0f;
        objects[object].channel->getAudibility(&audibility);

        glPrintF(8 + object, 1, "sound[%i] VOL: %f", object, audibility);

        glPolygonMode(GL_FRONT, GL_FILL);
        glPushMatrix();
        glTranslatef(objects[object].xPos, objects[object].yPos, objects[object].zPos);

        glDisable(GL_LIGHTING);
        glColor3f(1.0f - audibility, audibility, 0.0f);
        glRotatef(accumulatedTime * 200.0f, 1.0f, 1.0f, 0.0f);
        glutSolidTetrahedron();

        //glPopAttrib();
        glPopMatrix();
    }

    // finish in idle()
    //glutSwapBuffers();
    //glutPostWindowRedisplay(0);
}

void reshapeFunc(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

GlutCloseClass::~GlutCloseClass()
{
    glDeleteTextures(1, &texture);

    freeGeometry(walls);
    freeGeometry(rotatingMesh);
    freeGeometry(doorList[0]);
    freeGeometry(doorList[1]);
    freeGeometry(doorList[2]);
    freeGeometry(doorList[3]);

    sounds[0]->release();
    sounds[1]->release();
    sounds[2]->release();
    sounds[3]->release();

    fmodSystem->release();
}
