#ifndef MODELRECONSTRUCT_H
#define MODELRECONSTRUCT_H

#include <gl\glut.h>
#include "FbxParser.h"

class ModelReconstruct;
static ModelReconstruct *currModelRec;

class ModelReconstruct{
public:
	ModelReconstruct(FbxParser *parser, int argc, char **argv);
	~ModelReconstruct();
	
	ModelReconstruct(const ModelReconstruct&) = delete;
	ModelReconstruct& operator=(const ModelReconstruct&) = delete;
	
	
	void displayModel();
	void display(); 
	void loop();

private:
	int argc;
	char **argv;
	FbxParser *parser;
	void initModelSpace();
	void caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal);
};



#endif
