#ifndef AUDIO_H
#define AUDIO_H

// fmod sounds structures
extern FMOD::System *fmodSystem;
extern FMOD::Geometry *geometry;

struct Sound
{
    const char *name;
    FMOD_MODE mode;
    float minDist;
    float maxDist;
    FMOD::Sound *snd;
};
extern Sound sound[];

void ERRCHECK(FMOD_RESULT result);
void initObjects();
void audioInit();
void audioDoorInit();
void updateObjectSoundPos(Object* object);
void doGeometryMovement();
void doSoundMovement();
void doListenerMovement();

#endif