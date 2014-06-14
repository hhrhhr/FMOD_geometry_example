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
FMOD::Sound    *sounds[4]  = {0, 0, 0, 0};
FMOD::Geometry *geometry   = 0;

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
    FMOD_RESULT result;
    int i;

    for (i = 0; i < NUM_OBJECTS; i++) {
        // play object sounds
        FMOD_VECTOR pos = { objects[i].xPos, objects[i].yPos, objects[i].zPos };
        FMOD_VECTOR vel = { 0.0f,  0.0f, 0.0f };

        result = fmodSystem->playSound(sounds[objects[i].sound], 0, false, &objects[i].channel);
        ERRCHECK(result);
        result = objects[i].channel->set3DAttributes(&pos, &vel);
        ERRCHECK(result);
        result = objects[i].channel->set3DSpread(0.0f); //TODO: check this
        ERRCHECK(result);
    }
}

void audioInit()
{
    FMOD_RESULT      result;
    unsigned int     version;

    printf("==================================================================\n");
    printf("Geometry example.  Copyright (c) Firelight Technologies 2004-2014.\n");
    printf("==================================================================\n\n");

    // Create a System object and initialize.
    result = FMOD::System_Create(&fmodSystem);
    ERRCHECK(result);

    result = fmodSystem->getVersion(&version);
    ERRCHECK(result);
    if (version < FMOD_VERSION) {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        exit(-1);
    }

#ifdef FMOD_DEBUG
#   define __PROFILE FMOD_INIT_PROFILE_ENABLE | FMOD_INIT_PROFILE_METER_ALL |
#else
#   define __PROFILE
#endif
    result = fmodSystem->init(512, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED | __PROFILE
        FMOD_INIT_CHANNEL_LOWPASS | FMOD_INIT_CHANNEL_DISTANCEFILTER, 0);
    ERRCHECK(result);
#undef __PROFILE

    // Load sounds
    result = fmodSystem->createSound("../share/media/emi_idle.ogg", FMOD_DEFAULT, 0, &sounds[0]);
    ERRCHECK(result);
    result = sounds[0]->set3DMinMaxDistance(5.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[0]->setMode(FMOD_LOOP_NORMAL | FMOD_3D);
    ERRCHECK(result);

    result = fmodSystem->createSound("../share/media/magnitofon_2.ogg", FMOD_DEFAULT, 0, &sounds[1]);
    ERRCHECK(result);
    result = sounds[1]->set3DMinMaxDistance(2.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[1]->setMode(FMOD_LOOP_NORMAL | FMOD_3D);
    ERRCHECK(result);

    result = fmodSystem->createSound("../share/media/tushkano_idle_2.ogg", FMOD_DEFAULT, 0, &sounds[2]);
    ERRCHECK(result);
    result = sounds[2]->set3DMinMaxDistance(1.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[2]->setMode(FMOD_LOOP_NORMAL | FMOD_3D);
    ERRCHECK(result);

    result = fmodSystem->createSound("../share/media/radar_idle.ogg", FMOD_DEFAULT, 0, &sounds[3]);
    ERRCHECK(result);
    result = sounds[3]->set3DMinMaxDistance(5.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[3]->setMode(FMOD_LOOP_NORMAL | FMOD_3D);
    ERRCHECK(result);

    initObjects();

    result = fmodSystem->setGeometrySettings(200.0f);
    ERRCHECK(result);
}

void audioDoorInit()
{
    // place doors in desired orientatins
    FMOD_VECTOR up      = { 0.0f, 1.0f, 0.0f };
    FMOD_VECTOR forward = { 1.0f, 0.0f, 0.0f };
    FMOD_RESULT      result;
    result = doorList[1].geometry->setRotation(&forward, &up);
    ERRCHECK(result);
    result = doorList[2].geometry->setRotation(&forward, &up);
    ERRCHECK(result);
    result = doorList[3].geometry->setRotation(&forward, &up);
    ERRCHECK(result);
}

void updateObjectSoundPos(Object* object)
{
    FMOD_RESULT result;
    FMOD_VECTOR pos = { object->xPos, object->yPos, object->zPos };
    FMOD_VECTOR oldPos;
    object->channel->get3DAttributes(&oldPos, 0);

    FMOD_VECTOR vel;
    vel.x = (pos.x - oldPos.x) *  (1000.0f / INTERFACE_UPDATETIME);
    vel.y = (pos.y - oldPos.y) *  (1000.0f / INTERFACE_UPDATETIME);
    vel.z = (pos.z - oldPos.z) *  (1000.0f / INTERFACE_UPDATETIME);
    result = object->channel->set3DAttributes(&pos, &vel);
    ERRCHECK(result);
}

void doGeometryMovement()
{
    FMOD_RESULT result;

    // example of moving individual polygon vertices
    float xGeometryWarpPos = -30.0f;
    float zGeometryWarpPos = -21.0f;
    float dx = xListenerPos - xGeometryWarpPos;
    float dz = zListenerPos - zGeometryWarpPos;
    if (dx * dx + dz * dz < 30.0f * 30.0f)
    {
        if (sin(accumulatedTime * 1.0f) > 0.0f)
        {
            static FMOD_VECTOR lastOffset = { 0.0f, 0.0f, 0.0f };
            FMOD_VECTOR offset = { sin(accumulatedTime * 2.0f), 0.0f, cos(accumulatedTime * 2.0f) };
            for (int poly = 0; poly < walls.numPolygons; poly++)
            {
                Poly* polygon = &walls.polygons[poly];
                for (int i = 0; i < polygon->numVertices; i++)
                {
                    FMOD_VECTOR& vertex = walls.vertices[walls.indices[polygon->indicesOffset + i]];

                    dx = vertex.x - xGeometryWarpPos;
                    dz = vertex.z - zGeometryWarpPos;
                    if (dx * dx + dz * dz > 90.0f)
                        continue;
                    vertex.x -= lastOffset.x;
                    vertex.y -= lastOffset.y;
                    vertex.z -= lastOffset.z;

                    vertex.x += offset.x;
                    vertex.y += offset.y;
                    vertex.z += offset.z;
                    result = walls.geometry->setPolygonVertex(poly, i, &vertex);
                    ERRCHECK(result);
                }
            }
            lastOffset = offset;
        }
    }

    // example of rotation and a geometry object
    FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };
    FMOD_VECTOR forward = { (float)sin(accumulatedTime * 0.5f), 0.0f, (float)cos(accumulatedTime * 0.5f) };
    result = rotatingMesh.geometry->setRotation(&forward, &up);
    ERRCHECK(result);
    FMOD_VECTOR pos;
    pos.x = 12.0f;
    pos.y = (float)sin(accumulatedTime) * 0.4f + 0.1f;
    pos.z = 0.0f;
    result = rotatingMesh.geometry->setPosition(&pos);
    ERRCHECK(result);

    // example of moving doors
    // door 1
    pos.x = 3.25f;
    pos.y = ((float)sin(accumulatedTime)) * 2.0f + 1.0f;
    if (pos.y < 0.0f)
        pos.y = 0;
    if (pos.y > 2.0f)
        pos.y = 2.0f;
    pos.z = 11.5f;
    result = doorList[0].geometry->setPosition(&pos);
    ERRCHECK(result);

    // door 2
    pos.x = 0.75f;
    pos.y = ((float)sin(accumulatedTime)) * 2.0f + 1.0f;
    if (pos.y < 0.0f)
        pos.y = 0;
    if (pos.y > 2.0f)
        pos.y = 2.0f;
    pos.z = 14.75f;
    result = doorList[1].geometry->setPosition(&pos);
    ERRCHECK(result);

    // door 3
    pos.x = 8.25f;
    pos.y = ((float)sin(accumulatedTime)) * 2.0f + 1.0f;
    if (pos.y < 0.0f)
        pos.y = 0;
    if (pos.y > 2.0f)
        pos.y = 2.0f;
    pos.z = 14.75f;
    result = doorList[2].geometry->setPosition(&pos);
    ERRCHECK(result);

    // door 4
    pos.x = 33.0f;
    pos.y = ((float)sin(accumulatedTime)) * 2.0f + 1.0f;
    if (pos.y < 0.0f)
        pos.y = 0;
    if (pos.y > 2.0f)
        pos.y = 2.0f;
    pos.z = -0.75f;
    result = doorList[3].geometry->setPosition(&pos);
    ERRCHECK(result);
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

    float speed = INTERFACE_UPDATETIME / 1000.0f * 1.4f;
    if (isRun) speed *= 5.0f;

    if (moveForward)
        forward += speed;
    else if (moveBackward)
        forward -= speed;

    if (moveRight)
        right += speed;
    else if (moveLeft)
        right -= speed;

    if (moveUp)
        up += speed;
    else if (moveDown)
        up -= speed;

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

    if (yListenerPos < 1.0f)
        yListenerPos = 1.0f;

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

        FMOD_RESULT result = fmodSystem->set3DListenerAttributes(0, &listenerVector, &vel, &forward, &up);
        ERRCHECK(result);
    }
}
