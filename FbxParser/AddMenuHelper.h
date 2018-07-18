#pragma once

#include "Utilities.h"

/***************************************items ID***********************************************/
/*-------------------Camera----------------------------*/
#define PRODUCER_PERSPECTIVE_ITEM		100		//perspective
#define PRODUCER_TOP_ITEM			101		//top
#define PRODUCER_BOTTOM_ITEM		102		//bottom
#define PRODUCER_FRONT_ITEM			103		//front
#define PRODUCER_BACK_ITEM			104		//back
#define PRODUCER_RIGHT_ITEM			105		//right
#define PRODUCER_LEFT_ITEM			106		//left
#define CAMERA_SWITCHER_ITEM			107		//camera switcher

/*----------------animation menus ID-----------------*/
#define PLAY_ANIMATION				200	

/*-----------------shading mode ID--------------------*/
#define SHADING_MODE_SHADED_ITEM		300		//shading	
#define SHADING_MODE_WIREFRAME_ITEM	301		//wireframe

/*---------------------exit menu ID---------------------*/
#define MENU_EXIT					400			

/*-----------definiation strings of shading mode------------------*/
#define SHADING_MODE_SHADED_STRING		"Shaded"
#define SHADING_MODE_WIREFRAME_STRING	"WireFrame"

void AnimStackSelectCallback(int);			//select animation
void CameraSelectCallback(int);			//select camera
void ShadingModeSelectCallback(int);			//select shading mode
void MenuSelectCallback(int);				//select menus

