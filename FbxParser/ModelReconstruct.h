#ifndef MODELRECONSTRUCT_H
#define MODELRECONSTRUCT_H

#include <gl\glut.h>
#include "FbxParser.h"

class ModelReconstruct{
public:
	ModelReconstruct(FbxParser *parser);
	~ModelReconstruct();
	
	ModelReconstruct(const ModelReconstruct&) = delete;
	ModelReconstruct& operator=(const ModelReconstruct&) = delete;
	
	void initModelSpace();
	void displayModel();
	void display(); 

private:
	static int argc;
	static char **argv;
	FbxParser *parser;
	void caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal);
};



#endif
