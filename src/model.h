#ifndef MODEL_H
#define MODEL_H

const int NUM_OBJECTS = 7;

// sounds placement
struct Object
{
	float xPos;
	float yPos;
	float zPos;
	float intensity;
	int sound;
	FMOD::Channel *channel;
};

Object objects[];

// geometry structers for loading and drawing
struct Poly
{
	int numVertices;
	int indicesOffset;
	float directOcclusion;
	float reverbOcclusion;
	FMOD_VECTOR normal;
};

struct Mesh
{
	int numVertices;
	FMOD_VECTOR *vertices;
	float (*texcoords)[2];
	int numPolygons;
	Poly* polygons;
	int numIndices;
	int *indices;
	FMOD::Geometry *geometry;
};

extern Mesh walls;
extern Mesh rotatingMesh;
extern Mesh doorList[];

void initGeometry(const char* szFileName, Mesh& mesh);
void modelInit();
void freeGeometry(Mesh& mesh);

#endif