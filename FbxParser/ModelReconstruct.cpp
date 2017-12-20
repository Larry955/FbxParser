#include "ModelReconstruct.h"
#include <vector>
using std::vector;

ModelReconstruct::ModelReconstruct(FbxParser *parser) :parser(parser)
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
	GLfloat light0_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	GLfloat light1_ambient[] = { 1.0, 0.2, 0.2, 1.0 };
	GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat light1_specular[] = { 1.0, 0.6, 0.6, 1.0 };
	GLfloat light1_position[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat spot_direction[] = { 1.0, 1.0, -1.0 };

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
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, 1, 1, 80);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, -40, 50, 0, 60, 50, 0, 0, 1);
}

void* mymalloc(size_t size)
{
	printf("mymalloc %d\n", size);
	return malloc(size);
}

template <typename F>
void(*)() fkmemfunccastvoid(F f)
{
	void * p = mymalloc(sizeof(F));
	new(p) F(f);
	return p;
}

void ModelReconstruct::display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		}

		switch (parser->getFbxMesh()->GetPolygonSize(i))
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
		++vertexCounter;
		polygonVertex.clear();
		vertexNormal.clear();
	}

	FBXSDK_printf("shape load \n\n");
	glFlush();
	glutSwapBuffers();
}
void ModelReconstruct::displayModel()
{
	void(*)() p = fkmemfunccastvoid(&ModelReconstruct::display);

	
	glutDisplayFunc(p);
	
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