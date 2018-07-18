#include "AddMenuHelper.h"
#include "FbxParser.h"
#include "CameraManager.h"

extern FbxParser *parser;
extern CameraManager *cameraManager;

void AnimStackSelectCallback(int pIndex)
{
	parser->SetCurrAnimStack(pIndex);
}

void CameraSelectCallback(int pItem)
{
	const FbxArray<FbxNode*>& lCameraArray = parser->GetCameraArray();
	if (pItem == PRODUCER_PERSPECTIVE_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_PERSPECTIVE);
	}
	else if (pItem == PRODUCER_BACK_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_BACK);
	}
	else if (pItem == PRODUCER_FRONT_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_FRONT);
	}
	else if (pItem == PRODUCER_TOP_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_TOP);
	}
	else if (pItem == PRODUCER_BOTTOM_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_BOTTOM);
	}
	else if (pItem == PRODUCER_RIGHT_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_RIGHT);
	}
	else if (pItem == PRODUCER_LEFT_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_LEFT);
	}
	else if (pItem == CAMERA_SWITCHER_ITEM)
	{
		cameraManager->SetCurrentCamera(FBXSDK_CAMERA_SWITCHER);
	}
	else if (pItem >= 0 && pItem != lCameraArray.GetCount())
	{
		cameraManager->SetCurrentCamera(lCameraArray[pItem]->GetName());
	}
}

void ShadingModeSelectCallback(int pItem)
{
	if (pItem == SHADING_MODE_SHADED_ITEM)
	{
		parser->SetShadingMode(SHADING_MODE_SHADED);
	}
	else if (pItem == SHADING_MODE_WIREFRAME_ITEM)
	{
		parser->SetShadingMode(SHADING_MODE_WIREFRAME);
	}
}

void MenuSelectCallback(int pIndex)
{
	if (pIndex == PLAY_ANIMATION)
	{
		parser->SetCurrAnimStack(pIndex);
	}
	else if (pIndex == MENU_EXIT)
	{
		exit(0);
	}
}