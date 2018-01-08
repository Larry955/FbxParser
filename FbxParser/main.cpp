#include "ModelReconstruct.h"

int main(int argc, char **argv)
{
	FbxParser *parser = new FbxParser(FbxString("run"));
	bool loadResult = parser->loadScene();
	if (loadResult) {
		parser->displayGlobalLightSettings(&parser->getFbxScene()->GetGlobalSettings());
		parser->displayHierarchy(parser->getFbxScene());
		parser->displayContent(parser->getFbxScene());
	}
	else {
			FBXSDK_printf("error: load scene failed.\n\n");
		exit(1);
	}

	ModelReconstruct *model = new ModelReconstruct(parser, argc, argv);
	model->displayModel();
	model->activateKeyBoard();
	model->loop();
	system("pause");

}

//void SpecialKeys(int key, int x, int y)
//{
//	if (key == GLUT_KEY_UP) xRot -= 2.0;
//	if (key == GLUT_KEY_DOWN) xRot += 2.0;
//	if (key == GLUT_KEY_LEFT)yRot -= 2.0;
//	if (key == GLUT_KEY_RIGHT)yRot += 2.0;
//	if (xRot > 356.0)xRot = 0.0;
//	if (xRot < -1.0)xRot = 355.0;
//	if (yRot > 356.0)yRot = 0.0;
//	if (yRot < -1.0)yRot = 355.0;
//	glutPostRedisplay();
//}

//void MouseFunc(int button, int action, int x, int y)
//{
//	//left button to enlarge
//	if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN){
//		xScale += 0.1;
//		yScale += 0.1;
//		zScale += 0.1;
//	}
//	//right button to shrink
//	if (button == GLUT_RIGHT_BUTTON && action == GLUT_UP){
//		xScale -= 0.1;
//		yScale -= 0.1;
//		zScale -= 0.1;
//	}
//
//	glutPostRedisplay();
//}
//
//void changeSize(GLsizei w, GLsizei h)
//{
//	if (h == 0)		h = 1;
//	glViewport(0, 0, w, h);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	if (w <= h)
//		glOrtho(-200, 200, -200 * h / w, 200 * h / w, -200, 200);
//	else
//		glOrtho(-200 * h / w, 200 * h / w, -200, 200, -200, 200);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	gluLookAt(50, 20, 50, 0, 0, 0, 0, 0, 1);
//}