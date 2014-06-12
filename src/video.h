#ifndef VIDEO_H
#define VIDEO_H

class GlutCloseClass
{
  public:
    GlutCloseClass() {};
   ~GlutCloseClass();
};

GlutCloseClass gCloseObject;

GLuint loadTexture(const char * filename);
void videoInit();
void drawGeometry(Mesh& mesh);
void idle();
void timerFunc(int nValue);
void draw_debug_text();
void display(void);
void reshapeFunc(int w, int h);

#endif // VIDEO_H