#include "ModelReconstruct.h"

#include <Magick++.h>
using namespace Magick;

#include <iostream>
using namespace std;

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
	model->activateMouseFunc();
	model->activateMotionFunc();
	model->loop();
	system("pause");

}