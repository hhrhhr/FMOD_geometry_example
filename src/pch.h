#ifndef STDAFX_H
#define STDAFX_H

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include "fmod.hpp"
#include "fmod_errors.h"
#include <GL/freeglut.h>

#include "model.h"
#include "video.h"
#include "audio.h"

extern float INTERFACE_UPDATETIME;

const int NUM_OBJECTS = 7;
const int NUM_SOUNDS = 4;

// window size
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
