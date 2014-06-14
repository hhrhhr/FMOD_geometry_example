#include "pch.h"

// listener orientation
float xRotation = 0.0f;
float yRotation = -90.0f;

// listerer position
float xListenerPos = 30.0f;
float yListenerPos = 1.3f;
float zListenerPos = 0.4f;

const float PI = 3.14159265f;

// fmod sounds structures
FMOD::System   *fmodSystem = 0;
//FMOD::Sound    *sounds[4]  = {0, 0, 0, 0};
FMOD::Geometry *geometry   = 0;

const int DEFAULT_MODE = FMOD_DEFAULT | FMOD_LOOP_NORMAL | FMOD_3D;

Sound sound[NUM_SOUNDS] = {
    { "../share/media/emi_idle.ogg",        DEFAULT_MODE, 5.0f, 10000.0f, 0 },
    { "../share/media/magnitofon_2.ogg",    DEFAULT_MODE, 1.0f, 10000.0f, 0 },
    { "../share/media/tushkano_idle_2.ogg", DEFAULT_MODE, 1.0f, 10000.0f, 0 },
    { "../share/media/radar_idle.ogg",      DEFAULT_MODE, 5.0f, 10000.0f, 0 },
};

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK) {
        const char *err = FMOD_ErrorString(result);
        printf("FMOD error! (%d) %s\n", result, err);
        exit(-1);
    }
}

void initObjects()
{
    int i;

    for (i = 0; i < NUM_OBJECTS; i++) {
        // play object sounds
        FMOD_VECTOR pos = { objects[i].xPos, objects[i].yPos, objects[i].zPos };
        FMOD_VECTOR vel = { 0.0f,  0.0f, 0.0f };

        ERRCHECK(fmodSystem->playSound(sound[objects[i].sound].snd, 0, false, &objects[i].channel));
        ERRCHECK(objects[i].channel->set3DAttributes(&pos, &vel));
        ERRCHECK(objects[i].channel->set3DSpread(0.0f)); //TODO: check this
    }
}

void audioInit()
{
    printf("==================================================================\n");
    printf("Geometry example.  Copyright (c) Firelight Technologies 2004-2014.\n");
    printf("==================================================================\n\n");

    // Create a System object and initialize.
    ERRCHECK(FMOD::System_Create(&fmodSystem));

    unsigned int     version;
    ERRCHECK(fmodSystem->getVersion(&version));
    if (version < FMOD_VERSION) {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        exit(-1);
    }

#ifdef FMOD_DEBUG
#   define __PROFILE FMOD_INIT_PROFILE_ENABLE | FMOD_INIT_PROFILE_METER_ALL |
#else
#   define __PROFILE
#endif
    ERRCHECK(fmodSystem->init(512, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED | __PROFILE
        FMOD_INIT_CHANNEL_LOWPASS | FMOD_INIT_CHANNEL_DISTANCEFILTER, 0));
    
#undef __PROFILE


    // Load sounds
    for(int i = 0; i < NUM_SOUNDS; ++i) {
        Sound &s = sound[i];
        ERRCHECK(fmodSystem->createSound(s.name, s.mode, 0, &s.snd));
        ERRCHECK(s.snd->set3DMinMaxDistance(s.minDist, s.maxDist));
    }

    initObjects();

    ERRCHECK(fmodSystem->setGeometrySettings(200.0f));
    
}

void audioDoorInit()
{
    // place doors in desired orientatins
    FMOD_VECTOR up      = { 0.0f, 1.0f, 0.0f };
    FMOD_VECTOR forward = { 1.0f, 0.0f, 0.0f };
    ERRCHECK(doorList[1].geometry->setRotation(&forward, &up));
    ERRCHECK(doorList[2].geometry->setRotation(&forward, &up));
    ERRCHECK(doorList[3].geometry->setRotation(&forward, &up));
}

void updateObjectSoundPos(Object* object)
{
    FMOD_VECTOR pos = { object->xPos, object->yPos, object->zPos };
    FMOD_VECTOR oldPos;
    object->channel->get3DAttributes(&oldPos, 0);

    FMOD_VECTOR vel;
    vel.x = (pos.x - oldPos.x) * (1000.0f / INTERFACE_UPDATETIME);
    vel.y = (pos.y - oldPos.y) * (1000.0f / INTERFACE_UPDATETIME);
    vel.z = (pos.z - oldPos.z) * (1000.0f / INTERFACE_UPDATETIME);
    ERRCHECK(object->channel->set3DAttributes(&pos, &vel));
}

void doGeometryMovement()
{
    // example of rotation and a geometry object
    FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };
    FMOD_VECTOR forward = { (float)sin(accumulatedTime * 0.5f), 0.0f, (float)cos(accumulatedTime * 0.5f) };
    ERRCHECK(rotatingMesh.geometry->setRotation(&forward, &up));

    FMOD_VECTOR pos;

    pos.x = 12.0f;
    pos.y = 0.5f;
    pos.z = 0.0f;
    ERRCHECK(rotatingMesh.geometry->setPosition(&pos));

    // example of moving doors
    static const float d[4][2] = 
    {   //x,     z
        { 3.25f, 11.5f },
        { 0.75f, 14.75f },
        { 8.25f, 14.75f },
        { 33.0f, -0.75f },
    };

    for (int i = 0; i < 4; ++i) {
        pos.x = d[i][0];
        pos.y = ((float)sin(accumulatedTime)) * 2.0f + 1.0f;
        if (pos.y < 0.0f) pos.y = 0;
        if (pos.y > 2.0f) pos.y = 2.0f;
        pos.z = d[i][1];
        ERRCHECK(doorList[i].geometry->setPosition(&pos));
    }
}

void doSoundMovement()
{
    objects[0].zPos = 10.0f * sin(accumulatedTime);
    updateObjectSoundPos(&objects[0]);
    objects[5].zPos = -22 + 8.0f * sin(accumulatedTime);
    updateObjectSoundPos(&objects[5]);
}

void doListenerMovement()
{
    // Update user movement
    float forward = 0.0f;
    float right = 0.0f;
    float up = 0.0f;

    float speed = INTERFACE_UPDATETIME / 1000.0f * 1.4f;    // 1.4 m/s is walk speed
    if (isRun) speed *= 5.0f;

    if      (moveForward)  forward += speed;
    else if (moveBackward) forward -= speed;

    if      (moveRight)    right += speed;
    else if (moveLeft)     right -= speed;

    if      (moveUp)       up += speed;
    else if (moveDown)     up -= speed;

    float xRight = (float)cos(yRotation * (PI / 180.0f));
    float yRight = 0.0f;
    float zRight = (float)sin(yRotation * (PI / 180.0f));

    xListenerPos += xRight * right;
    yListenerPos += yRight * right;
    zListenerPos += zRight * right;

    float xForward =  (float)sin(yRotation * (PI / 180.0f)) * cos(xRotation  * (PI / 180.0f));
    float yForward = -(float)sin(xRotation * (PI / 180.0f));
    float zForward = -(float)cos(yRotation * (PI / 180.0f)) * cos(xRotation  * (PI / 180.0f));

    xListenerPos += xForward * forward;
    yListenerPos += yForward * forward;
    zListenerPos += zForward * forward;

    yListenerPos += up;

    if (yListenerPos < 1.7f)
        yListenerPos = 1.7f;

    // cross product
    float xUp = yRight * zForward - zRight * yForward;
    float yUp = zRight * xForward - xRight * zForward;
    float zUp = xRight * yForward - yRight * xForward;

    // Update listener
    {
        FMOD_VECTOR listenerVector;
        listenerVector.x = xListenerPos;
        listenerVector.y = yListenerPos;
        listenerVector.z = zListenerPos;

        static FMOD_VECTOR lastpos = { 0.0f, 0.0f, 0.0f };
        static bool bFirst = true;
        FMOD_VECTOR forward;
        FMOD_VECTOR up;
        FMOD_VECTOR vel;

        forward.x = xForward;
        forward.y = yForward;
        forward.z = zForward;
        up.x = xUp;
        up.y = yUp;
        up.z = zUp;

        // ********* NOTE ******* READ NEXT COMMENT!!!!!
        // vel = how far we moved last FRAME (m/f), then time compensate it to SECONDS (m/s).
        vel.x = (listenerVector.x - lastpos.x) * (1000.0f / INTERFACE_UPDATETIME);
        vel.y = (listenerVector.y - lastpos.y) * (1000.0f / INTERFACE_UPDATETIME);
        vel.z = (listenerVector.z - lastpos.z) * (1000.0f / INTERFACE_UPDATETIME);
        if (bFirst) {
            bFirst = false;
            vel.x = 0;
            vel.y = 0;
            vel.z = 0;
        }

        static FMOD_VECTOR lastVel = { 0.0f, 0.0f, 0.0f };

        // store pos for next time
        lastpos = listenerVector;
        lastVel = vel;

        ERRCHECK(fmodSystem->set3DListenerAttributes(0, &listenerVector, &vel, &forward, &up));
    }
}
