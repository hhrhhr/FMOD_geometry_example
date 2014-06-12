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
		FMOD_VECTOR vel = {  0.0f,  0.0f, 0.0f };

		result = fmodSystem->playSound(FMOD_CHANNEL_FREE, sounds[objects[i].sound], true, &objects[i].channel);
		ERRCHECK(result);
		result = objects[i].channel->set3DAttributes(&pos, &vel);
		ERRCHECK(result);
		result = objects[i].channel->set3DSpread(90.0f);
		ERRCHECK(result);
		result = objects[i].channel->setPaused(false);
		ERRCHECK(result);
	}
}

void audioInit()
{
    FMOD_RESULT      result;
    //bool             listenerflag = true;
    //FMOD_VECTOR      listenerpos  = { 0.0f, 0.0f, 0.0f };
    unsigned int     version;
    FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS        caps;
    char             name[256];
    int              numdrivers;

    printf("==================================================================\n");
    printf("Geometry example.  Copyright (c) Firelight Technologies 2004-2014.\n");
    printf("==================================================================\n\n");

    // Create a System object and initialize.
    result = FMOD::System_Create(&fmodSystem);
    ERRCHECK(result);
    
    result = fmodSystem->getVersion(&version);
    ERRCHECK(result);
    printf("FMOD ver. %08x\n", version);

    if (version < FMOD_VERSION) {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        exit(-1);
    }

    result = fmodSystem->getNumDrivers(&numdrivers);
    ERRCHECK(result);
    printf("founded %d drivers\n", numdrivers);

    if (numdrivers == 0) {
        result = fmodSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
        printf("no sound!");
    } else {
        result = fmodSystem->getDriverCaps(0, &caps, 0, &speakermode);
        ERRCHECK(result);

        result = fmodSystem->setSpeakerMode(speakermode);       /* Set the user selected speaker mode. */
        ERRCHECK(result);

        if (caps & FMOD_CAPS_HARDWARE_EMULATED) {            /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
            result = fmodSystem->setDSPBufferSize(1024, 10); /* You might want to warn the user about this. */
            ERRCHECK(result);
        }

        FMOD_GUID guid;
        result = fmodSystem->getDriverInfo(0, name, 256, &guid);
        ERRCHECK(result);
        printf("used driver: %s, guid: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", 
            name, guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
            guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        if (strstr(name, "SigmaTel")) { /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
            result = fmodSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }

        int samplerate;
        FMOD_SOUND_FORMAT format;
        int numoutputchannels;
        int maxinputchannels;
        FMOD_DSP_RESAMPLER resamplemethod;
        int bits;
        result = fmodSystem->getSoftwareFormat(&samplerate, &format, &numoutputchannels, &maxinputchannels, &resamplemethod, &bits);
        printf("sample rate: %i, format: %i, outputs: %i, inputs: %i, resample: %i, bits: %i\n",
            samplerate, format, numoutputchannels, maxinputchannels, resamplemethod, bits);

    }
#ifdef FMOD_DEBUG
#   define __PROFILE FMOD_INIT_ENABLE_PROFILE |
#else
#   define __PROFILE
#endif
    result = fmodSystem->init(100, FMOD_INIT_3D_RIGHTHANDED     | FMOD_INIT_OCCLUSION_LOWPASS | __PROFILE
                                   FMOD_INIT_DISTANCE_FILTERING | FMOD_INIT_HRTF_LOWPASS  , 0);
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER) /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
    {
        result = fmodSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);
                
        result = fmodSystem->init(100, FMOD_INIT_3D_RIGHTHANDED     | FMOD_INIT_OCCLUSION_LOWPASS |
                                       FMOD_INIT_DISTANCE_FILTERING | FMOD_INIT_HRTF_LOWPASS, 0);/* ... and re-init. */
        ERRCHECK(result);
    };
/*
    char dbgLog[256];
    FMOD_ADVANCEDSETTINGS adv;
    adv.maxMPEGcodecs = 0;              // [r/w] Optional. Specify 0 to ignore. For use with FMOD_CREATECOMPRESSEDSAMPLE only.  Mpeg  codecs consume 21,684 bytes per instance and this number will determine how many mpeg channels can be played simultaneously.   Default = 32.    
    adv.maxADPCMcodecs = 0;             // [r/w] Optional. Specify 0 to ignore. For use with FMOD_CREATECOMPRESSEDSAMPLE only.  ADPCM codecs consume  2,136 bytes per instance and this number will determine how many ADPCM channels can be played simultaneously.  Default = 32.
    adv.maxXMAcodecs = 0;               // [r/w] Optional. Specify 0 to ignore. For use with FMOD_CREATECOMPRESSEDSAMPLE only.  XMA   codecs consume 14,836 bytes per instance and this number will determine how many XMA channels can be played simultaneously.    Default = 32.
    adv.maxCELTcodecs = 0;              // [r/w] Optional. Specify 0 to ignore. For use with FMOD_CREATECOMPRESSEDSAMPLE only.  CELT  codecs consume 11,500 bytes per instance and this number will determine how many CELT channels can be played simultaneously.   Default = 32.
    adv.maxVORBIScodecs = 0;            // [r/w] Optional. Specify 0 to ignore. For use with FMOD_CREATECOMPRESSEDSAMPLE only.  Vorbis codecs consume 12,000 bytes per instance and this number will determine how many Vorbis channels can be played simultaneously. Default = 32.
    adv.maxAT9Codecs = 0;               // [r/w] Optional. Specify 0 to ignore. For use with FMOD_CREATECOMPRESSEDSAMPLE only.  AT9 codecs consume  8,720 bytes per instance and this number will determine how many AT9 channels can be played simultaneously. Default = 32.
    adv.maxPCMcodecs = 0;               // [r/w] Optional. Specify 0 to ignore. For use with PS3 only.                          PCM   codecs consume 12,672 bytes per instance and this number will determine how many streams and PCM voices can be played simultaneously. Default = 16.
    adv.ASIONumChannels = 0;            // [r/w] Optional. Specify 0 to ignore. Number of channels available on the ASIO device.
    adv.ASIOChannelList = 0;            // [r/w] Optional. Specify 0 to ignore. Pointer to an array of strings (number of entries defined by ASIONumChannels) with ASIO channel names.
    adv.ASIOSpeakerList = 0;            // [r/w] Optional. Specify 0 to ignore. Pointer to a list of speakers that the ASIO channels map to.  This can be called after System::init to remap ASIO output.
    adv.max3DReverbDSPs = 0;            // [r/w] Optional. Specify 0 to ignore. The max number of 3d reverb DSP's in the system. (NOTE: CURRENTLY DISABLED / UNUSED)
    adv.HRTFMinAngle = 0;               // [r/w] Optional.                      For use with FMOD_INIT_HRTF_LOWPASS.  The angle range (0-360) of a 3D sound in relation to the listener, at which the HRTF function begins to have an effect. 0 = in front of the listener. 180 = from 90 degrees to the left of the listener to 90 degrees to the right. 360 = behind the listener. Default = 180.0.
    adv.HRTFMaxAngle = 0;               // [r/w] Optional.                      For use with FMOD_INIT_HRTF_LOWPASS.  The angle range (0-360) of a 3D sound in relation to the listener, at which the HRTF function has maximum effect. 0 = front of the listener. 180 = from 90 degrees to the left of the listener to 90 degrees to the right. 360 = behind the listener. Default = 360.0.
    adv.HRTFFreq = 0;                   // [r/w] Optional. Specify 0 to ignore. For use with FMOD_INIT_HRTF_LOWPASS.  The cutoff frequency of the HRTF's lowpass filter function when at maximum effect. (i.e. at HRTFMaxAngle).  Default = 4000.0.
    adv.vol0virtualvol = 0;             // [r/w] Optional. Specify 0 to ignore. For use with FMOD_INIT_VOL0_BECOMES_VIRTUAL.  If this flag is used, and the volume is 0.0, then the sound will become virtual.  Use this value to raise the threshold to a different point where a sound goes virtual.
    adv.eventqueuesize = 0;             // [r/w] Optional. Specify 0 to ignore. For use with FMOD Event system only.  Specifies the number of slots available for simultaneous non blocking loads, across all threads.  Default = 32.
    adv.defaultDecodeBufferSize = 0;    // [r/w] Optional. Specify 0 to ignore. For streams. This determines the default size of the double buffer (in milliseconds) that a stream uses.  Default = 400ms
    adv.debugLogFilename = dbgLog;      // [r/w] Optional. Specify 0 to ignore. Gives fmod's logging system a path/filename.  Normally the log is placed in the same directory as the executable and called fmod.log. When using System::getAdvancedSettings, provide at least 256 bytes of memory to copy into.
    adv.profileport = 0;                // [r/w] Optional. Specify 0 to ignore. For use with FMOD_INIT_ENABLE_PROFILE.  Specify the port to listen on for connections by the profiler application.
    adv.geometryMaxFadeTime = 0;        // [r/w] Optional. Specify 0 to ignore. The maximum time in miliseconds it takes for a channel to fade to the new level when its occlusion changes.
    adv.maxSpectrumWaveDataBuffers = 0; // [r/w] Optional. Specify 0 to ignore. Tells System::init to allocate a pool of wavedata/spectrum buffers to prevent memory fragmentation, any additional buffers will be allocated normally.
    adv.musicSystemCacheDelay = 0;      // [r/w] Optional. Specify 0 to ignore. The delay the music system should allow for loading a sample from disk (in milliseconds). Default = 400 ms.
    adv.distanceFilterCenterFreq = 0;   // [r/w] Optional. Specify 0 to ignore. For use with FMOD_INIT_DISTANCE_FILTERING.  The default center frequency in Hz for the distance filtering effect. Default = 1500.0.
    adv.stackSizeStream = 0;            // [r/w] Optional. Specify 0 to ignore. Specify the stack size for the FMOD Stream thread in bytes.  Useful for custom codecs that use excess stack.  Default 49,152 (48kb)
    adv.stackSizeNonBlocking = 0;       // [r/w] Optional. Specify 0 to ignore. Specify the stack size for the FMOD_NONBLOCKING loading thread.  Useful for custom codecs that use excess stack.  Default 65,536 (64kb)
    adv.stackSizeMixer = 0;             // [r/w] Optional. Specify 0 to ignore. Specify the stack size for the FMOD mixer thread.  Useful for custom dsps that use excess stack.  Default 49,152 (48kb)

    adv.cbsize = sizeof(adv);
    result = fmodSystem->getAdvancedSettings(&adv);
    ERRCHECK(result);

    adv.geometryMaxFadeTime = 100;
    result = fmodSystem->setAdvancedSettings(&adv);
    ERRCHECK(result);
*/
  
    // Load sounds
    result = fmodSystem->createSound("../share/media/emi_idle.ogg", FMOD_SOFTWARE | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_3D, 0, &sounds[0]);
    ERRCHECK(result);
    result = sounds[0]->set3DMinMaxDistance(5.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[0]->setMode(FMOD_LOOP_NORMAL);
    ERRCHECK(result);

    result = fmodSystem->createSound("../share/media/magnitofon_2.ogg", FMOD_SOFTWARE | FMOD_CREATECOMPRESSEDSAMPLE |  FMOD_3D, 0, &sounds[1]);
    ERRCHECK(result);
    result = sounds[1]->set3DMinMaxDistance(2.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[1]->setMode(FMOD_LOOP_NORMAL);
    ERRCHECK(result);

    result = fmodSystem->createSound("../share/media/tushkano_idle_2.ogg", FMOD_SOFTWARE | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_3D, 0, &sounds[2]);
    ERRCHECK(result);
    result = sounds[2]->set3DMinMaxDistance(1.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[2]->setMode(FMOD_LOOP_NORMAL);
    ERRCHECK(result);

    result = fmodSystem->createSound("../share/media/radar_idle.ogg", FMOD_SOFTWARE | FMOD_CREATECOMPRESSEDSAMPLE | FMOD_3D, 0, &sounds[3]);
    ERRCHECK(result);
    result = sounds[3]->set3DMinMaxDistance(5.0f, 10000.0f);
    ERRCHECK(result);
    result = sounds[3]->setMode(FMOD_LOOP_NORMAL);
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
