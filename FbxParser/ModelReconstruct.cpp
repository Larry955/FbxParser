#include "ModelReconstruct.h"

#include <vector>
#include <cstdio>
using std::vector;

#include <Magick++.h>
using namespace Magick;	//a third party library, used for resolving the images 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"	//a third part header,used for resolving the image


GLuint  textureArr[1];         // Storage For One Texture ( NEW )  

//it is recommend that only one object of ModelReconstruct be constructed in the memory
ModelReconstruct::ModelReconstruct(FbxParser *parser, int argc, char **argv) :parser(parser), argc(argc), argv(argv)
{
	initModelSpace();
}

ModelReconstruct::~ModelReconstruct()
{

}

void ModelReconstruct::initModelSpace()
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Reconstructed FBX Model");


	GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_position[] = { 1.0, 0.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	GLfloat light1_ambient[] = { 0.0, 0.2, 0.2, 1.0 };
	GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat light1_specular[] = { 1.0, 0.0, 0.6, 1.0 };
	GLfloat light1_position[] = { 1.0, 0.0, 1.0, 0.0 };
	GLfloat spot_direction[] = { 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, 1, 1, 80000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//FBXSDK_printf("cameraPosX: %f, cameraPosY: %f, cameraPosZ: %f\n\n", cameraPosX, cameraPosY, cameraPosZ);

	gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
		cameraPosX + cameraRotX, cameraPosY + cameraRotY, cameraPosZ,
		0, 0, 1);

	loadGLTextures();
}

void displayCallBack()
{
	currModelRec->display();
}

void ModelReconstruct::display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (deltaMove) {
		computePos(deltaMove);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, 1, 1, 80000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//FBXSDK_printf("cameraPosX: %f, cameraPosY: %f, cameraPosZ: %f\n\n", cameraPosX, cameraPosY, cameraPosZ);

	gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
		cameraPosX + cameraRotX, cameraPosY + cameraRotY, cameraPosZ,
		0, 0, 1);
	glPushMatrix();

	glRotatef(xRot, 1.0, 0.0, 0.0);
	glRotatef(zRot, 0.0, 0.0, 1.0);
	glScalef(xScale, yScale, zScale);

	glBindTexture(GL_TEXTURE_2D, textureArr[0]);

	//Polygon Points, Normals and UVs
	vector<FbxVector4> polygonPoints = parser->getPolygonPoints();
	vector<FbxVector4> normals = parser->getNormals();
	vector<FbxVector2> uvs = parser->getTextureUVs();

	//for (int i = 0; i != 24; ++i) {
	//	if (i % 3 == 0)
	//		FBXSDK_printf("\nuv: ");
	//	FBXSDK_printf("%.10lf, %.10lf\n", uvs[i].mData[0], uvs[i].mData[1]);
	//}

	//for (int i = 0; i != 24; ++i) {
	//	if (i % 3 == 0)
	//		FBXSDK_printf("\nnormal: ");
	//	FBXSDK_printf("%.10lf, %.10lf\n", normals[i].mData[0], normals[i].mData[1], normals[i].mData[2]);
	//}


	int polygonPointsIndex = 0;
	int polygonIndex = 0;
	while (polygonPointsIndex < polygonPoints.size()) {
		int polygonSize = parser->getFbxMesh()->GetPolygonSize(polygonIndex++);
		if (polygonSize == 3) {
			glEnable(GL_NORMALIZE);	//normalize
			//glColor3f(1.0f, 0.0f, 0.0f);
			
			glBegin(GL_TRIANGLES);
			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glEnd();
		}
		else if (polygonSize == 4) {
			glEnable(GL_NORMALIZE);	//normalize
			//glColor3f(1.0f, 1.0f, 0.0f);
			
			glBegin(GL_QUADS);
			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glEnd();
		}
		else {
			FBXSDK_printf("undefined polygon size: %d\n", polygonSize);
		}
	}

	//glDisable(GL_TEXTURE_2D);

	glPopMatrix();

	FbxAMatrix lDummyGlobalPosition;
	drawGrid(lDummyGlobalPosition);

	glFlush();
	glutSwapBuffers();
}

void ModelReconstruct::computePos(GLfloat deltaMove)
{
	cameraPosX += deltaMove * cameraRotX * 0.1f;
	cameraPosY += deltaMove * cameraRotY * 0.1f;
}

void ModelReconstruct::displayModel()
{
	currModelRec = this;
	glutDisplayFunc(::displayCallBack);
	
}

void ModelReconstruct::drawGrid(const FbxAMatrix & pTransform)
{
	

	glPushMatrix();
	glMultMatrixd(pTransform);

	// Draw a grid 500*500
	glDisable(GL_LIGHTING);	//disabled light before draw lines to make the color of grid works
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.3f, 0.3f, 0.3f);
	glLineWidth(1.0);
	
	const int hw = 500;
	const int step = 20;
	const int bigstep = 100;
	
	int i;

	// Draw Grid
	for (i = -hw; i <= hw; i += step) {

		if (i % bigstep == 0) {
			glLineWidth(2.0);
		}
		else {
			glLineWidth(1.0);
		}
		if (i == 0) {
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_LINES);
			glVertex3i(i, -hw, 0);
			glVertex3i(i, hw, 0);
			glEnd();

			glColor3f(0.0f, 1.0f, 0.0f);
			glBegin(GL_LINES);
			glVertex3i(-hw, i, 0);
			glVertex3i(hw, i, 0);
			glEnd();
		}
		else {
			glColor3f(0.3f, 0.3f, 0.3f);
			glBegin(GL_LINES);
			glVertex3i(i, -hw, 0);
			glVertex3i(i, hw, 0);
			glEnd();
			
			glBegin(GL_LINES);
			glVertex3i(-hw, i, 0);
			glVertex3i(hw, i, 0);
			glEnd();
		}

	}
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}

void ModelReconstruct::keyFunc(unsigned char key, int x, int y)
{
	//resetTransformFactor();

	switch (key) {
	case 'w':
	case 'W':
		deltaMove = 1.0f;
		cameraOffY = 1.0f;
		//xRot -= 2.0;
		break;
	case 's':
	case 'S':
		deltaMove = -1.0f;

		cameraOffY = -1.0f;
		//xRot += 2.0;
		break;
	case 'a':
	case 'A':
		cameraOffX = 1.0f;
		zRot -= 2.0;
		break;
	case 'd':
	case 'D':
		zRot += 2.0;
		break;
	case 'z':
	case 'Z':
		xScale += 0.1;
		yScale += 0.1;
		zScale += 0.1;
		break;
	case 'x':
	case 'X':
		xScale -= 0.1;
		yScale -= 0.1;
		zScale -= 0.1;
		break;
	default:
		FBXSDK_printf("undefined key: %c\n",key);
		break;
	}
	if (xRot > 356.0)
		xRot = 0.0;
	if (xRot < -1.0)
		xRot = 355.0;
	if (zRot > 356.0)
		zRot = 0.0;
	if (zRot < -1.0)
		zRot = 355.0;

	glutPostRedisplay();
}

void keysCallBack(unsigned char key, int x, int y)
{
	currModelRec->keyFunc(key, x, y);
}


void ModelReconstruct::activateKeyBoard()
{
	
	currModelRec = this;
	glutKeyboardFunc(::keysCallBack);
}


void ModelReconstruct::motionFunc(int x, int y)
{
	// this will only be true when the left button is down
	if (xOrigin >= 0) {
		// update deltaAngle
		deltaAngle = (x - xOrigin) * 0.001f;

		// update camera's direction
		cameraRotX = -sin(angle + deltaAngle);
		cameraRotY = cos(angle + deltaAngle);
		
		//FBXSDK_printf("cameraRotX: %f, cameraRotY: %f, x: %d\n\n", cameraRotX, cameraRotY, x);
	}
	else {
		FBXSDK_printf("nothing \n\n");
	}
}


void motionCallBack(int x, int y)
{
	currModelRec->motionFunc(x, y);
}

void ModelReconstruct::activateMotionFunc()
{
	currModelRec = this;
	glutMotionFunc(::motionCallBack);
}

void ModelReconstruct::mouseFunc(int button, int state, int x, int y)
{
	
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			angle += deltaAngle;
			xOrigin = -1;
		}
		else  {// state = GLUT_DOWN
			xOrigin = x;
		}
	}
}

void mouseCallBack(int button, int state, int x, int y)
{
	currModelRec->mouseFunc(button, state, x, y);
}

void ModelReconstruct::activateMouseFunc()
{
	currModelRec = this;
	glutMouseFunc(::mouseCallBack);
}

void ModelReconstruct::keyUpFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 's':
		cameraOffY = 0.0f;
		deltaMove = 0.0f;
		break;
	case 'a':
	case 'd':
		cameraOffX = 0.0f;
		deltaMove = 0.0f;
		break;
	default:
		break;
	}
}

void keyUpCallBack(unsigned char key, int x, int y)
{
	currModelRec->keyUpFunc(key, x, y);
}

void ModelReconstruct::activeKeyUpFunc()
{
	currModelRec = this;
	glutKeyboardUpFunc(::keyUpCallBack);
}

void ModelReconstruct::activeIdleFunc()
{
	currModelRec = this;
	glutIdleFunc(::displayCallBack);
}

void ModelReconstruct::loop()
{
	glutPostRedisplay();
	glutMainLoop();
}

void ModelReconstruct::resetTransformFactor()
{
	xRot = 0.0;
	zRot = 0.0;

	xScale = 1.0;
	yScale = 1.0;
	zScale = 1.0;
	 
}


//since we failed to get the data from texture image, this function is not be called
bool ModelReconstruct::loadGLTextures()
{
	bool status = false;
	RGBImgStructure *textureImage[1];	//create storage space for textures
	memset(textureImage, 0, sizeof(RGBImgStructure*) * 1);	//init the pointer to NULL
	
	FbxString fileName = parser->getTextureFileName();
	fileName = "G:\\FBX_SDK\\FbxParser\\FbxParser\\E16011.tga";

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureArr[0]);
	glBindTexture(GL_TEXTURE_2D, textureArr[0]); //bind the texture to it's array
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*int width, height, nrChannels;
	unsigned char *data = stbi_load(fileName.Buffer(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		status = true;
	}
	else
	{
		FBXSDK_printf("error\n\n");
	}
	
	stbi_image_free(data);*/

#define TEXTUREIMAGE
#undef TEXTUREIMAGE

	if (isNotEmpty(fileName)) {
#ifdef TEXTUREIMAGE
		InitializeMagick(*argv);
		try {
			Image image(fileName.Buffer());
			Geometry geo = image.size();
			size_t width = geo.width();
			size_t height = geo.height();
			
			Blob blob;
			image.write(&blob);
			
			unsigned char* pixels = (unsigned char*)blob.data();	//get data from the image
			
			//glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage[0]->width, textureImage[0]->height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage[0]->data);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);

		}
		catch (Exception &error) {
			FBXSDK_printf("error: %s", error.what());
			return false;
		}
#else
		switch (getFileSuffix(fileName.Buffer()))
		{
		case eTextureType::TGA:
			return loadTGA(fileName.Buffer(), textureArr[0]);
			break;
		case eTextureType::DDS:
			return loadDDS(fileName.Buffer());
		case eTextureType::BMP:
			textureImage[0] = loadBMP(fileName.Buffer());
			break;
		case eTextureType::UNKNOWN:
			FBXSDK_printf("error!\n\n");
			break;
		default:
			break;
		}
	}
#endif
	return status;
}


void ModelReconstruct::reshapeFunc(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;

	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(120.0f, ratio, 1.0f, 80000.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void reshapeCallBack(int w, int h)
{
	currModelRec->reshapeFunc(w, h);
}

void ModelReconstruct::activeReshapFunc()
{
	currModelRec = this;
	glutReshapeFunc(::reshapeCallBack);
}