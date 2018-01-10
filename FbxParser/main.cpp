#include "ModelReconstruct.h"

#include <Magick++.h>
using namespace Magick;

#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
	FbxParser *parser = new FbxParser(FbxString("soldier"));
	bool loadResult = parser->loadScene();
	if (loadResult) {
		parser->displayGlobalLightSettings(&parser->getFbxScene()->GetGlobalSettings());
		parser->displayHierarchy(parser->getFbxScene());
		parser->displayContent(parser->getFbxScene());
		parser->displayPose(parser->getFbxScene());
	}
	else {
		FBXSDK_printf("error: load scene failed.\n\n");
		exit(1);
	}

	ModelReconstruct *model = new ModelReconstruct(parser, argc, argv);
	model->displayModel();	//show the model
	model->activeReshapFunc();	//reshape window
	model->activeIdleFunc();		//called when the window is idle
	model->activateKeyBoard();	//active keyboard so that we can move the camera/model by pressing a key
	model->activeKeyUpFunc();	//stop moving the camera
	model->activateMouseFunc();	//active the mouse function to get the cooridinate of mouse in the window
	model->activateMotionFunc();	//change the direction of camera	
	model->loop();		//glutMainLoop
	system("pause");
}