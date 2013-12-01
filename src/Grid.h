#ifndef GRID_H_
#define GRID_H_

#include "gameplay.h"
#include <cstdarg>
#define PI 3.1415926535

using namespace gameplay;

static const unsigned int DEFAULT_LINE_COUNT = 81;

/**
 * Creates a new grid mesh.
 * 
 * @param lineCount The number of lines in the grid. (Rows or columns). Should be odd.
 * 
 * @return A new grid mesh or NULL if there was an error.
 */
Mesh* createGridMesh(unsigned int lineCount = DEFAULT_LINE_COUNT);

/**
 * Creates a model that contains a new grid mesh.
 * 
 * @param lineCount The number of lines in the grid. (Rows or columns). Should be odd.
 * 
 * @return A new model containing a grid mesh or NULL if there was an error.
 */
Model* createGridModel(unsigned int lineCount = DEFAULT_LINE_COUNT);

//box
Mesh* createBoxMesh(float width, float height, float depth, Node* node = NULL);
Model* createBoxModel(float width, float height, float depth, Node* node = NULL);
//cylinder
Mesh* createCylinderMesh(float radius, float height, int segments = -1, Node* node = NULL);
Model* createCylinderModel(float radius, float height, int segments = -1, Node* node = NULL);
//cone
Mesh* createConeMesh(float radius, float height, int segments = -1, Node* node = NULL);
Model* createConeModel(float radius, float height, int segments = -1, Node* node = NULL);
//sphere
Mesh* createSphereMesh(float radius, float height, int segments = -1, Node* node = NULL);
Model* createSphereModel(float radius, float height, int segments = -1, Node* node = NULL);

void setEdges(Node* node, const char* type, ...);

//any data associated with a node - currently just the list of vertex index pairs forming the edges
typedef struct {
	unsigned short numVertices, numEdges;
	float *vertices;
	unsigned short *edges;
	
} nodeData;

#endif
