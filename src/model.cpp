#include "pch.h"

Object objects[NUM_OBJECTS] =
{
    {  -11.0f,    1.0f,    0.0f,    1.0f,    0,    0},
    {   12.0f,    2.0f,    0.0f,    1.0f,    1,    0},
    {   45.0f,    1.0f,    0.0f,    1.0f,    3,    0},
    {  -30.0f,    1.0f,   21.0f,    1.0f,    2,    0},
    {  -30.0f,    1.0f,  -21.0f,    1.0f,    3,    0},
    {   12.0f,    1.0f,  -27.0f,    1.0f,    0,    0},
    {    4.0f,    1.0f,   16.0f,    1.0f,    0,    0},
};

Mesh walls;
Mesh rotatingMesh;
Mesh doorList[4];

void initGeometry(const char* szFileName, Mesh& mesh)
{
    FMOD_RESULT result;

    FILE* file;
    errno_t err = fopen_s(&file, szFileName, "rb");
    if (err != 0) {
        printf("The file %s was not opened\n", szFileName);
        return;
    } else {
        // read vertices
        fread(&mesh.numVertices, sizeof (mesh.numVertices), 1, file);
        mesh.vertices = new FMOD_VECTOR[mesh.numVertices];
        mesh.texcoords = new float[mesh.numVertices][2];
        fread(mesh.vertices, sizeof (float) * 3, mesh.numVertices, file);
        fread(mesh.texcoords, sizeof (float) * 2, mesh.numVertices, file);

        // read 
        fread(&mesh.numIndices, sizeof (mesh.numIndices), 1, file);
        mesh.indices = new int[mesh.numIndices];
        fread(mesh.indices, sizeof (int), mesh.numIndices, file);

        // read
        fread(&mesh.numPolygons, sizeof (mesh.numPolygons), 1, file);
        mesh.polygons = new Poly[mesh.numPolygons];

        // read polygons
        for (int poly = 0; poly < mesh.numPolygons; poly++)
        {
            Poly* polygon = &mesh.polygons[poly];

            fread(&polygon->numVertices, sizeof (polygon->numVertices), 1, file);
            fread(&polygon->indicesOffset, sizeof (polygon->indicesOffset), 1, file);
            fread(&polygon->directOcclusion, sizeof (polygon->directOcclusion), 1, file);
            fread(&polygon->reverbOcclusion, sizeof (polygon->reverbOcclusion), 1, file);

            int* indices = &mesh.indices[polygon->indicesOffset];

            // calculate polygon normal
            float xN = 0.0f;
            float yN = 0.0f;
            float zN = 0.0f;
            // todo: return an error if a polygon has less then 3 vertices.
            for (int vertex = 0; vertex < polygon->numVertices - 2; vertex++) {
                float xA = mesh.vertices[indices[vertex + 1]].x - mesh.vertices[indices[0]].x;
                float yA = mesh.vertices[indices[vertex + 1]].y - mesh.vertices[indices[0]].y;
                float zA = mesh.vertices[indices[vertex + 1]].z - mesh.vertices[indices[0]].z;
                float xB = mesh.vertices[indices[vertex + 2]].x - mesh.vertices[indices[0]].x;
                float yB = mesh.vertices[indices[vertex + 2]].y - mesh.vertices[indices[0]].y;
                float zB = mesh.vertices[indices[vertex + 2]].z - mesh.vertices[indices[0]].z;
                // cross product
                xN += yA * zB - zA * yB;
                yN += zA * xB - xA * zB;
                zN += xA * yB - yA * xB;
            }
            float fMagnidued = (float)sqrt(xN * xN + yN * yN + zN * zN);
            if (fMagnidued > 0.0f) { // a tollerance here might be called for
                xN /= fMagnidued;
                yN /= fMagnidued;
                zN /= fMagnidued;
            }
            polygon->normal.x = xN;
            polygon->normal.y = yN;
            polygon->normal.z = zN;
        }
        fclose(file);
    }

    result = fmodSystem->createGeometry(mesh.numPolygons, mesh.numIndices, &mesh.geometry);
    ERRCHECK(result);

    for (int poly = 0; poly < mesh.numPolygons; poly++)
    {
        Poly* polygon = &mesh.polygons[poly];
        FMOD_VECTOR vertices[16];
        int i;

        for (i = 0; i < polygon->numVertices; i++)
            vertices[i] = mesh.vertices[mesh.indices[polygon->indicesOffset + i]];

        int polygonIndex = 0;

        result = mesh.geometry->addPolygon(polygon->directOcclusion, polygon->reverbOcclusion, false, // single sided
            polygon->numVertices, vertices, &polygonIndex);
        ERRCHECK(result);
    }
}

void modelInit()
{
    // load objects
    initGeometry("../share/media/walls.bin", walls);
    initGeometry("../share/media/center.bin", rotatingMesh);
    initGeometry("../share/media/door.bin", doorList[0]);
    initGeometry("../share/media/door.bin", doorList[1]);
    initGeometry("../share/media/door.bin", doorList[2]);
    initGeometry("../share/media/door.bin", doorList[3]);
}

void freeGeometry(Mesh& mesh)
{
    mesh.geometry->release();

    delete [] mesh.vertices;
    delete [] mesh.texcoords;
    delete [] mesh.polygons;
    delete [] mesh.indices;
}
