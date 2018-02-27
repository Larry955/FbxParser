#include "ModelReconstruct.h"


#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
	FbxParser *parser = new FbxParser(FbxString("soldier"));
	bool loadResult = parser->loadScene();
	if (loadResult) {
		parser->displayGlobalLightSettings(&parser->getFbxScene()->GetGlobalSettings());		//display global light settings
		parser->displayHierarchy(parser->getFbxScene());		//display hierarchy of model
		parser->displayContent(parser->getFbxScene());	//display 
		parser->displayPose(parser->getFbxScene());
	}
	else {
		FBXSDK_printf("error: load scene failed.\n\n");
		exit(1);
	}
	
	if (parser) {
		ModelReconstruct *model = new ModelReconstruct(parser, argc, argv);
		model->displayModel();	//show the model
		model->activeReshapFunc();	//reshape window
		model->activeIdleFunc();		//called when the window is idle
		model->activateKeyBoard();	//active keyboard so that we can move the camera/model by pressing a key
		model->activeKeyUpFunc();	//stop moving the camera
		model->activateMouseFunc();	//active the mouse function to get the cooridinate of mouse in the window
		model->activateMotionFunc();	//change the direction of camera	
		model->loop();		//glutMainLoop
	}
	if (loadResult) {
		parser->covertFormat();
	}
	system("pause");
}