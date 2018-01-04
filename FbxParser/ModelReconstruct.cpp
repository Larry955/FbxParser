#include "ModelReconstruct.h"
#include <gl\GLAux.h>
#include <vector>
#include <cstdio>
using std::vector;

GLuint  textureArr[1];         // Storage For One Texture ( NEW )  

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
	glutCreateWindow("Reconstructed Model");


	GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	GLfloat light0_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat light0_position[] = { 1.0, 0.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	/*GLfloat light1_ambient[] = { 0.0, 0.2, 0.2, 1.0 };
	GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat light1_specular[] = { 1.0, 0.0, 0.6, 1.0 };
	GLfloat light1_position[] = { 1.0, 0.0, 1.0, 0.0 };
	GLfloat spot_direction[] = { 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);*/

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);


	if (parser->getFbxFileName() == "run") {
		//used for run.fbx
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(120, 1, 1, 80);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0, -40, 50, 0, 60, 40, 0, 0, 1);
	}
	else if (parser->getFbxFileName() == "bunny") {
		//for bunny.fbx
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(120, 1, 1, 80000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(200, 250, -300, 0, 150, 0, 0, 1, 0.5);
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(120, 1, 1, 80);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0, -40, 50, 0, 60, 40, 0, 0, 1);
	}
	

	loadGLTextures();
}

void displayCallBack()
{
	currModelRec->display();
}

void ModelReconstruct::display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRotatef(xRot, 1.0, 0.0, 0.0);
	glRotatef(zRot, 0.0, 0.0, 1.0);
	glScalef(xScale, yScale, zScale);
	glBindTexture(GL_TEXTURE_2D, textureArr[0]);


	//Polygon Points
	int polygonCount = parser->getFbxMesh()->GetPolygonCount();
	int vertexCounter = 0;
	vector<FbxVector4> polygonVertex;
	vector<FbxVector4> vertexNormal;

	FBXSDK_printf("\n\n---------------Polygon Points---------------------\n\n");
	FBXSDK_printf("polygon points count %d\n", polygonCount);

	for (int i = 0; i != polygonCount; ++i) {
		int polygonSize = parser->getFbxMesh()->GetPolygonSize(i);
		//FBXSDK_printf("polygonSize: %d\n", polygonSize);

		vertexNormal.resize(polygonSize);

		for (int j = 0; j != polygonSize; ++j) {
			int vertexIndex = parser->getFbxMesh()->GetPolygonVertex(i, j);
			FbxVector4 vec = (parser->getFbxVector4())[vertexIndex];
			//display3DVector("vec: ", vec);
			caclNormal(parser->getFbxMesh(), vertexIndex, vertexCounter, polygonSize, vertexNormal[j]);
			
			polygonVertex.push_back(vec);
			++vertexCounter;
		}
		
		/*for (auto s : vertexNormal) {
			if (s.mData[0] == static_cast<FbxDouble>(0) && 
				s.mData[1] == static_cast<FbxDouble>(0) &&
				s.mData[2] == static_cast<FbxDouble>(0) &&
				s.mData[3] == static_cast<FbxDouble>(0))
				FBXSDK_printf("invalid normal\n");
		}*/
		
		switch (polygonSize)
		{
		case 3:
			glEnable(GL_NORMALIZE);	//normalize
			glBegin(GL_TRIANGLES);
			glNormal3f(static_cast<float>(vertexNormal[0].mData[0]), static_cast<float>(vertexNormal[0].mData[1]), static_cast<float>(vertexNormal[0].mData[2]));
			glVertex3f(polygonVertex[0].mData[0], polygonVertex[0].mData[1], polygonVertex[0].mData[2]);
			glNormal3f(static_cast<float>(vertexNormal[1].mData[0]), static_cast<float>(vertexNormal[1].mData[1]), static_cast<float>(vertexNormal[1].mData[2]));
			glVertex3f(polygonVertex[1].mData[0], polygonVertex[1].mData[1], polygonVertex[1].mData[2]);
			glNormal3f(static_cast<float>(vertexNormal[2].mData[0]), static_cast<float>(vertexNormal[2].mData[1]), static_cast<float>(vertexNormal[2].mData[2]));
			glVertex3f(polygonVertex[2].mData[0], polygonVertex[2].mData[1], polygonVertex[2].mData[2]);
			glEnd();
			break;
		case 4:
			glEnable(GL_NORMALIZE);	//normalize
			glBegin(GL_QUADS);
			glNormal3f(static_cast<float>(vertexNormal[0].mData[0]), static_cast<float>(vertexNormal[0].mData[1]), static_cast<float>(vertexNormal[0].mData[2]));
			glVertex3f(polygonVertex[0].mData[0], polygonVertex[0].mData[1], polygonVertex[0].mData[2]);
			glNormal3f(static_cast<float>(vertexNormal[1].mData[0]), static_cast<float>(vertexNormal[1].mData[1]), static_cast<float>(vertexNormal[1].mData[2]));
			glVertex3f(polygonVertex[1].mData[0], polygonVertex[1].mData[1], polygonVertex[1].mData[2]);
			glNormal3f(static_cast<float>(vertexNormal[2].mData[0]), static_cast<float>(vertexNormal[2].mData[1]), static_cast<float>(vertexNormal[2].mData[2]));
			glVertex3f(polygonVertex[2].mData[0], polygonVertex[2].mData[1], polygonVertex[2].mData[2]);
			glNormal3f(static_cast<float>(vertexNormal[3].mData[0]), static_cast<float>(vertexNormal[3].mData[1]), static_cast<float>(vertexNormal[3].mData[2]));
			glVertex3f(polygonVertex[3].mData[0], polygonVertex[3].mData[1], polygonVertex[3].mData[2]);
			glEnd();
			break;
		default:
			FBXSDK_printf("undefined polygon size: %d\n", parser->getFbxMesh()->GetPolygonSize(i));
			break;
		}
	
		polygonVertex.clear();
		vertexNormal.clear();
	}

	FBXSDK_printf("UV layer count: %d\n", parser->getFbxMesh()->GetUVLayerCount());		//return 1
	FBXSDK_printf("UV texture count: %d\n", parser->getFbxMesh()->GetTextureUVCount());	//return 1970 in run.fbx
	FbxGeometryElementUV *uv = parser->getFbxMesh()->GetElementUV(0);
	FBXSDK_printf("name: %s\n", uv->GetName());	//return UV_channel_1

	glFlush();
	glutSwapBuffers();
}

void ModelReconstruct::displayModel()
{
	currModelRec = this;
	glutDisplayFunc(::displayCallBack);
	
}

void ModelReconstruct::keyFunc(unsigned char key, int x, int y)
{
	resetTransformFactor();

	switch (key) {
	case 'w':
		xRot -= 2.0;
		break;
	case 's':
		xRot += 2.0;
		break;
	case 'a':
		zRot -= 2.0;
		break;
	case 'd':
		zRot += 2.0;
		break;
	case 'z':
		xScale += 0.1;
		yScale += 0.1;
		zScale += 0.1;
		break;
	case 'x':
		xScale -= 0.1;
		yScale -= 0.1;
		zScale -= 0.1;
		break;
	default:
		FBXSDK_printf("undefined key: %c\n",key);
		break;
	}
	/*if (key == GLUT_KEY_UP) {
		xRot = 0.0;
		xRot -= 2.0;
	}
	if (key == GLUT_KEY_DOWN) {
		xRot = 0.0;
		xRot += 2.0;
	}
	
	if (key == GLUT_KEY_LEFT){ 
		zRot = 0.0;
		zRot -= 2.0; 
	}
	if (key == GLUT_KEY_RIGHT){
		zRot = 0.0;
		zRot += 2.0;
	}*/
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

void ModelReconstruct::caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal)
{
	if (mesh->GetElementNormalCount() < 1) {
		throw std::exception("invalid normal number\n");
	}
	FbxGeometryElementNormal *vertexNormal = mesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			vertexNormal->GetDirectArray().GetCount();
			normal = vertexNormal->GetDirectArray().GetAt(vertexCounter).mData;
			break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(vertexIndex);
			normal = vertexNormal->GetDirectArray().GetAt(index).mData;
			break;
		}
		default:
			throw std::exception("Invalid Reference");
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			//c = vertexNormal->GetDirectArray().GetCount();
			normal = vertexNormal->GetDirectArray().GetAt(vertexCounter).mData;
			/*if (polygonSize == 3) {
			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
			}
			else if (polygonSize == 4) {
			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
			normal.mData[3] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[3]);
			}*/
			break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(vertexCounter);
			normal = vertexNormal->GetDirectArray().GetAt(index).mData;
			/*if (polygonSize == 3) {
			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			else if (polygonSize == 4) {
			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
			normal.mData[3] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[3]);
			}*/
			break;
		}
		default:
			throw std::exception("Invalid Reference");
		}
	default:
		break;
	}
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

bool ModelReconstruct::loadGLTextures()
{
	bool status = false;
	RGBImgStructure *textureImage[1];	//create storage space for textures
	memset(textureImage, 0, sizeof(RGBImgStructure*) * 1);	//init the pointer to NULL
	FbxString fileName = parser->getTextureFileName();

	if (isNotEmpty(fileName)) {
		switch (getFileSuffix(fileName.Buffer()))
		{
		case eTextureType::TGA:
			textureImage[0] = loadTGA(fileName.Buffer());
			break;
		case eTextureType::DDS:
			textureImage[0] = loadDDS(fileName.Buffer());
			break;
		case eTextureType::BMP:
			textureImage[0] = loadBMP(fileName.Buffer());
			break;
		case eTextureType::UNKNOWN:
			FBXSDK_printf("erro!\n\n");
			break;
		default:
			break;
		}
	}
	if (textureImage[0]) {	
		glGenTextures(1, &textureArr[0]);		//create the texture

		glBindTexture(GL_TEXTURE_2D, textureArr[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage[0]->width, textureImage[0]->height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (textureImage[0]->data) {
			FBXSDK_printf("ss%s\n", textureImage[0]->data);
			free(textureImage[0]->data);
		}
		free(textureImage[0]);

		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		status = true;
	}
	return status;
}

//RGBImgStructure* ModelReconstruct::loadImageFile(const char* fileName)
//{
//	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
//	textureImage->width = 0;	//initialize
//	textureImage->height = 0;
//	textureImage->data = nullptr;
//
//	FILE* imageFile = nullptr;
//	unsigned long size = 0;
//	imageFile = fopen(fileName, "rb");		//binary file
//
//	fseek(imageFile, 18, SEEK_SET);
//	fread(&(textureImage->width), 4, 1, imageFile);
//	fread(&(textureImage->height), 4, 1, imageFile);
//	fseek(imageFile, 0, SEEK_END);
//	size = ftell(imageFile) - 54;
//
//	textureImage->data = (unsigned char*)malloc(size);
//	memset(textureImage->data, 0, size);
//	fseek(imageFile, 54, SEEK_SET);
//	fread(textureImage->data, size, 1, imageFile);
//
//	fclose(imageFile);
//	return textureImage;
//}