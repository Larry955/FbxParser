#ifndef MODELRECONSTRUCT_H
#define MODELRECONSTRUCT_H

#include <gl\glut.h>
#include "FbxParser.h"

class ModelReconstruct;
static ModelReconstruct *currModelRec;

//rotate factor
static GLfloat xRot = 0.0f;
static GLfloat zRot = 0.0f;

//scale factor
static GLfloat xScale = 1.0f;
static GLfloat yScale = 1.0f;
static GLfloat zScale = 1.0f;

class ModelReconstruct{
public:
	ModelReconstruct(FbxParser *parser, int argc, char **argv);		// argc, argv is the parameters of main function, which will be used as the paras of glutInit() function
	~ModelReconstruct();		//don't know what destructor should do yet
	
	//copy constructor/operator= is forbidden
	ModelReconstruct(const ModelReconstruct&) = delete;
	ModelReconstruct& operator=(const ModelReconstruct&) = delete;
	
	
	void displayModel();	//used for display the model
	void display();		//not recommend to be called by an object since it's should be called by displayModel function

	void activateKeyBoard();	//activate keyboard to rotate the model
	void keyFunc(unsigned char key, int x, int y);		//not recommend to be called by an object since it's should be called by activateKeys function
	
	void loop();			//used for glutMainLoop()

private:
	int argc;
	char **argv;
	FbxParser *parser;

	void initModelSpace();
	void resetTransformFactor();
	void caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal);
};



#endif
