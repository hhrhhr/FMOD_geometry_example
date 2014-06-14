#ifndef STDAFX_H
#define STDAFX_H

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <math.h>

#include "fmod.hpp"
#include "fmod_errors.h"
#include <GL/freeglut.h>

#include "model.h"
#include "audio.h"
#include "video.h"


extern float interfaceUpdatTime;

const int NUM_OBJECTS = 7;
const int NUM_SOUNDS = 4;

// window params
extern int width;
extern int height;
extern int window_pos_x;
extern int window_pos_y;

// keyboard control
extern bool moveForward;
extern bool moveBackward;
extern bool moveLeft;
extern bool moveRight;
extern bool moveUp;
extern bool moveDown;
extern bool isRun;

// mesh
extern Mesh walls;
extern Mesh rotatingMesh;
extern Mesh doorList[];

// listener orientation
extern float xRotation;
extern float yRotation;

// listerer position
extern float xListenerPos;
extern float yListenerPos;
extern float zListenerPos;

extern float accumulatedTime;

//debug text
extern unsigned int useFont;

#endif // STDAFX_H
