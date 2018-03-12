#ifndef MODELRECONSTRUCT_H
#define MODELRECONSTRUCT_H

#include <gl\glew.h>
#include <gl\GLAux.h>
#include <gl\glut.h>

#include "FbxParser.h"
#include "RGBImgStructure.h"


class ModelReconstruct;
static ModelReconstruct *currModelRec;

//rotate factor
static GLfloat xRot = 0.0f;
static GLfloat zRot = 0.0f;

//scale factor
static GLfloat xScale = 1.0f;
static GLfloat yScale = 1.0f;
static GLfloat zScale = 1.0f;

//angle of rotation for the camera direction
static GLfloat angle = 0.0f;

//actual vector representing the camera's direction
static GLfloat cameraRotX = 0.0f;
static GLfloat cameraRotY = 1.0f;

//XZ offset of the camera
static GLfloat cameraOffX = 0.0f;
static GLfloat cameraOffY = 0.0f;
static GLfloat cameraOffZ = 0.0f;

//position of camera
static GLfloat cameraPosX = 0.0;
static GLfloat cameraPosY = -60.0f;
static GLfloat cameraPosZ = 100.0f;

// the key states. These variables will be zero
//when no key is being pressed
static float deltaAngle = 0.0f;
static float deltaMove = 0.0f;
static int xOrigin = -1;

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
	
	void activateMouseFunc();
	void mouseFunc(int button, int state, int x, int y);	

	void activateMotionFunc();
	void motionFunc(int x, int y);
	
	void activeKeyUpFunc();
	void keyUpFunc(unsigned char key, int x, int y);

	void activeIdleFunc();
	void idlefunc();

	void activeReshapFunc();
	void reshapeFunc(int w, int h);

	void drawGrid(const FbxAMatrix & pTransform);
	void loop();			//used for glutMainLoop()

private:
	int argc;
	char **argv;
	FbxParser *parser;

	bool loadGLTextures();

	void computePos(GLfloat deltaMode);
	//void textureMapping();
	void initModelSpace();
	void resetTransformFactor();
};



#endif
