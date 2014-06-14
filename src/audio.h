#ifndef AUDIO_H
#define AUDIO_H

// fmod sounds structures
extern FMOD::System *fmodSystem;
extern FMOD::Sound *sounds[];
extern FMOD::Geometry *geometry;

void ERRCHECK(FMOD_RESULT result);
void initObjects();
void audioInit();
void audioDoorInit();
void updateObjectSoundPos(Object* object);
void doGeometryMovement();
void doSoundMovement();
void doListenerMovement();

#endif