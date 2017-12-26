#ifndef MODELRECONSTRUCT_H
#define MODELRECONSTRUCT_H

#include <gl\glut.h>
#include "FbxParser.h"

class ModelReconstruct;
static ModelReconstruct *currModelRec;

class ModelReconstruct{
public:
	ModelReconstruct(FbxParser *parser, int argc, char **argv);		// argc, argv is the parameters of main function, which will be used as the paras of glutInit() function
	~ModelReconstruct();		//don't know what destructor should do yet
	
	//copy constructor/operator= is forbidden
	ModelReconstruct(const ModelReconstruct&) = delete;
	ModelReconstruct& operator=(const ModelReconstruct&) = delete;
	
	
	void displayModel();	//used for display the model
	void display();		//called by a ModelReconstruct is not recommended since it's shall be called by displayModel
	void loop();			//used for glutMainLoop()

private:
	int argc;
	char **argv;
	FbxParser *parser;

	void initModelSpace();
	void caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal);
};



#endif
