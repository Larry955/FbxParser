#include "CameraManager.h"
#include "FbxParser.h"
#include <gl\glut.h>
extern FbxParser *parser;


#define HFOV2VFOV(h, ar) (2.0 * atan((ar) * tan( (h * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI) //ar : aspectY / aspectX
#define VFOV2HFOV(v, ar) (2.0 * atan((ar) * tan( (v * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI) //ar : aspectX / aspectY

static double gsOrthoCameraScale = 178.0;


bool CameraManager::SetCurrentCamera(const char *pCameraName)
{
	if (!pCameraName)
	{
		return false;
	}
	FbxGlobalSettings &lGlobalCameraSettings = parser->GetFbxScene()->GetGlobalSettings();
	lGlobalCameraSettings.SetDefaultCamera(pCameraName);
	parser->SetAnimStatus(MUST_BE_REFRESHED);
	return true;
}

void CameraManager::SetCamera(FbxScene *pScene, FbxTime &pTime, FbxAnimLayer *pAnimLayer,
	const FbxArray<FbxNode*> &cameraArray, int windowWidth, int windowHeight)
{
	//find the current camera at the given time
	FbxCamera *lCamera = GetCurrentCamera(pScene, pTime, pAnimLayer, cameraArray);
	if (lCamera == NULL)
		return;
	FbxNode *lCameraNode = lCamera ? lCamera->GetNode() : NULL;

	//compute the camera position and direction
	FbxVector4 lEye(0, 0, 1);
	FbxVector4 lCenter(0, 0, 0);
	FbxVector4 lUp(0, 1, 0);
	FbxVector4 lForward, lRight;
	
	if (lCamera)
	{
		lEye = lCamera->Position.Get();
		lUp = lCamera->UpVector.Get();
	}

	if (lCameraNode && lCameraNode->GetTarget())
	{
		lCenter = parser->GetGlobalPosition(lCameraNode->GetTarget(), pTime).GetT();
	}
	else 
	{
		if (!lCameraNode || IsProducerCamera(pScene, lCamera))
		{
			if (lCamera)
				lCenter = lCamera->InterestPosition.Get();
		}
		else
		{
			// get the direction
			FbxAMatrix lGlobalRotation;
			FbxVector4 lRotationVector(parser->GetGlobalPosition(lCameraNode, pTime).GetR());
			lGlobalRotation.SetR(lRotationVector);

			//get the length
			FbxVector4 lInterestPosition(lCamera->InterestPosition.Get());
			FbxVector4 lCameraGlobalPosition(parser->GetGlobalPosition(lCameraNode, pTime).GetT());
			double lLength = (FbxVector4(lInterestPosition - lCameraGlobalPosition).Length());

			//set the center
			//a camera with rotation = {0,0,0} points to the X direction. So create a 
			//vector in the X direction, rotate that vector by the global rotation amount
			//and then position the center by scaling and translating the resulting vector
			lRotationVector = FbxVector4(1.0, 0, 0);
			lCenter = lGlobalRotation.MultT(lRotationVector);
			lCenter *= lLength;
			lCenter += lEye;

			// Update the default up vector with the camera rotation.
			lRotationVector = FbxVector4(0, 1.0, 0);
			lUp = lGlobalRotation.MultT(lRotationVector);
		}
	}

	// Align the up vector.
	lForward = lCenter - lEye;
	lForward.Normalize();
	lRight = lForward.CrossProduct(lUp);
	lRight.Normalize();
	lUp = lRight.CrossProduct(lForward);
	lUp.Normalize();

	// Rotate the up vector with the roll value.
	double lRadians = 0;

	if (lCamera)
		lRadians = lCamera->Roll.Get() * FBXSDK_PI_DIV_180;
	lUp = lUp * cos(lRadians) + lRight * sin(lRadians);


	double lNearPlane = 0.01;
	if (lCamera)
		lNearPlane = lCamera->GetNearPlane();
	double lFarPlane = 4000.0;
	if (lCamera)
		lFarPlane = lCamera->GetFarPlane();

	//Get global scaling.
	FbxVector4 lCameraScaling = parser->GetGlobalPosition(lCameraNode, pTime).GetS();
	static const int  FORWARD_SCALE = 2;

	//scaling near plane and far plane
	lNearPlane *= lCameraScaling[FORWARD_SCALE];
	lFarPlane *= lCameraScaling[FORWARD_SCALE];


	// Get the relevant camera settings for a perspective view.
	if (lCamera && lCamera->ProjectionType.Get() == FbxCamera::ePerspective)
	{
		//get the aspect ratio
		FbxCamera::EAspectRatioMode lCamAspectRatioMode = lCamera->GetAspectRatioMode();
		double lAspectX = lCamera->AspectWidth.Get();
		double lAspectY = lCamera->AspectHeight.Get();
		double lAspectRatio = 1.333333;
		switch (lCamAspectRatioMode)
		{
		case FbxCamera::eWindowSize:
			lAspectRatio = lAspectX / lAspectY;
			break;
		case FbxCamera::eFixedRatio:
			lAspectRatio = lAspectX;

			break;
		case FbxCamera::eFixedResolution:
			lAspectRatio = lAspectX / lAspectY * lCamera->GetPixelRatio();
			break;
		case FbxCamera::eFixedWidth:
			lAspectRatio = lCamera->GetPixelRatio() / lAspectY;
			break;
		case FbxCamera::eFixedHeight:
			lAspectRatio = lCamera->GetPixelRatio() * lAspectX;
			break;
		default:
			break;

		}

		//get the aperture ratio
		double lFilmHeight = lCamera->GetApertureHeight();
		double lFilmWidth = lCamera->GetApertureWidth() * lCamera->GetSqueezeRatio();
		//here we use Height : Width
		double lApertureRatio = lFilmHeight / lFilmWidth;


		//change the aspect ratio to Height : Width
		lAspectRatio = 1 / lAspectRatio;
		//revise the aspect ratio and aperture ratio
		FbxCamera::EGateFit lCameraGateFit = lCamera->GateFit.Get();
		switch (lCameraGateFit)
		{

		case FbxCamera::eFitFill:
			if (lApertureRatio > lAspectRatio)  // the same as eHORIZONTAL_FIT
			{
				lFilmHeight = lFilmWidth * lAspectRatio;
				lCamera->SetApertureHeight(lFilmHeight);
				lApertureRatio = lFilmHeight / lFilmWidth;
			}
			else if (lApertureRatio < lAspectRatio) //the same as eVERTICAL_FIT
			{
				lFilmWidth = lFilmHeight / lAspectRatio;
				lCamera->SetApertureWidth(lFilmWidth);
				lApertureRatio = lFilmHeight / lFilmWidth;
			}
			break;
		case FbxCamera::eFitVertical:
			lFilmWidth = lFilmHeight / lAspectRatio;
			lCamera->SetApertureWidth(lFilmWidth);
			lApertureRatio = lFilmHeight / lFilmWidth;
			break;
		case FbxCamera::eFitHorizontal:
			lFilmHeight = lFilmWidth * lAspectRatio;
			lCamera->SetApertureHeight(lFilmHeight);
			lApertureRatio = lFilmHeight / lFilmWidth;
			break;
		case FbxCamera::eFitStretch:
			lAspectRatio = lApertureRatio;
			break;
		case FbxCamera::eFitOverscan:
			if (lFilmWidth > lFilmHeight)
			{
				lFilmHeight = lFilmWidth * lAspectRatio;
			}
			else
			{
				lFilmWidth = lFilmHeight / lAspectRatio;
			}
			lApertureRatio = lFilmHeight / lFilmWidth;
			break;
		case FbxCamera::eFitNone:
		default:
			break;
		}
		//change the aspect ratio to Width : Height
		lAspectRatio = 1 / lAspectRatio;

		double lFieldOfViewX = 0.0;
		double lFieldOfViewY = 0.0;
		if (lCamera->GetApertureMode() == FbxCamera::eVertical)
		{
			lFieldOfViewY = lCamera->FieldOfView.Get();
			lFieldOfViewX = VFOV2HFOV(lFieldOfViewY, 1 / lApertureRatio);
		}
		else if (lCamera->GetApertureMode() == FbxCamera::eHorizontal)
		{
			lFieldOfViewX = lCamera->FieldOfView.Get(); //get HFOV
			lFieldOfViewY = HFOV2VFOV(lFieldOfViewX, lApertureRatio);
		}
		else if (lCamera->GetApertureMode() == FbxCamera::eFocalLength)
		{
			lFieldOfViewX = lCamera->ComputeFieldOfView(lCamera->FocalLength.Get());    //get HFOV
			lFieldOfViewY = HFOV2VFOV(lFieldOfViewX, lApertureRatio);
		}
		else if (lCamera->GetApertureMode() == FbxCamera::eHorizAndVert) {
			lFieldOfViewX = lCamera->FieldOfViewX.Get();
			lFieldOfViewY = lCamera->FieldOfViewY.Get();
		}



		double lRealScreenRatio = (double)windowWidth / (double)windowHeight;
		int  lViewPortPosX = 0,
			lViewPortPosY = 0,
			lViewPortSizeX = windowWidth,
			lViewPortSizeY = windowHeight;
		//compute the view port
		if (lRealScreenRatio > lAspectRatio)
		{
			lViewPortSizeY = windowHeight;
			lViewPortSizeX = (int)(lViewPortSizeY * lAspectRatio);
			lViewPortPosY = 0;
			lViewPortPosX = (int)((windowWidth - lViewPortSizeX) * 0.5);
		}
		else
		{
			lViewPortSizeX = windowWidth;
			lViewPortSizeY = (int)(lViewPortSizeX / lAspectRatio);
			lViewPortPosX = 0;
			lViewPortPosY = (int)((windowHeight - lViewPortSizeY) * 0.5);
		}

		//revise the Perspective since we have film offset
		double lFilmOffsetX = lCamera->FilmOffsetX.Get();
		double lFilmOffsetY = lCamera->FilmOffsetY.Get();
		lFilmOffsetX = 0 - lFilmOffsetX / lFilmWidth * 2.0;
		lFilmOffsetY = 0 - lFilmOffsetY / lFilmHeight * 2.0;

		//set camera perspective
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glTranslated(lFilmOffsetX, lFilmOffsetY, 0);
		gluPerspective(60,	//fov, a higer value means wider field of view
			lAspectRatio,
			lNearPlane,
			lFarPlane);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		gluLookAt(lEye[0], lEye[1], lEye[2],
			lCenter[0], lCenter[1] + 50, lCenter[2],
			lUp[0], lUp[1], lUp[2]);
		//GlSetCameraPerspective(lFieldOfViewY, lAspectRatio, lNearPlane, lFarPlane, lEye, lCenter, lUp, lFilmOffsetX, lFilmOffsetY);



		//glMatrixMode(GL_PROJECTION);
		//double lTestPerpMatrix[ 16];
		//glGetDoublev( GL_PROJECTION_MATRIX, lTestPerpMatrix);

		//lTestPerpMatrix[ 8] -= lFilmOffsetX;
		//lTestPerpMatrix[ 9] -= lFilmOffsetY;
		//
		//glLoadMatrixd( lTestPerpMatrix);
		//glMatrixMode(GL_MODELVIEW);


		glViewport(lViewPortPosX, lViewPortPosY, lViewPortSizeX, lViewPortSizeY);


	}
	// Get the relevant camera settings for an orthogonal view.
	else
	{
		double lPixelRatio = 1.0;
		if (lCamera)
			lPixelRatio = lCamera->GetPixelRatio();

		double lLeftPlane, lRightPlane, lBottomPlane, lTopPlane;

		if (windowWidth < windowHeight)
		{
			lLeftPlane = -gsOrthoCameraScale * lPixelRatio;
			lRightPlane = gsOrthoCameraScale * lPixelRatio;
			lBottomPlane = -gsOrthoCameraScale * windowHeight / windowWidth;
			lTopPlane = gsOrthoCameraScale * windowHeight / windowWidth;
		}
		else
		{
			windowWidth *= (int)lPixelRatio;
			lLeftPlane = -gsOrthoCameraScale * windowWidth / windowHeight;
			lRightPlane = gsOrthoCameraScale * windowWidth / windowHeight;
			lBottomPlane = -gsOrthoCameraScale;
			lTopPlane = gsOrthoCameraScale;
		}

		//set camera orthogonal
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(lLeftPlane,
			lRightPlane,
			lBottomPlane,
			lTopPlane,
			lNearPlane,
			lFarPlane);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(lEye[0], lEye[1], lEye[2],
			lCenter[0], lCenter[1], lCenter[2],
			lUp[0], lUp[1], lUp[2]);

		/*GlSetCameraOrthogonal(lLeftPlane,
			lRightPlane,
			lBottomPlane,
			lTopPlane,
			lNearPlane,
			lFarPlane,
			lEye,
			lCenter,
			lUp);*/
	}
}


FbxCamera* CameraManager::GetCurrentCamera(FbxScene *pScene, FbxTime &pTime,
	FbxAnimLayer* pAnimLayer, const FbxArray<FbxNode*> &cameraArray)
{
	FbxGlobalSettings &lGlobalSettings = pScene->GetGlobalSettings();
	FbxGlobalCameraSettings &lGlobalCameraSettings = pScene->GlobalCameraSettings();
	FbxString lCurrentCameraName = lGlobalSettings.GetDefaultCamera();

	//check if we need to create the Producer cameras
	if (lGlobalCameraSettings.GetCameraProducerPerspective() == NULL &&
		lGlobalCameraSettings.GetCameraProducerBack() == NULL &&
		lGlobalCameraSettings.GetCameraProducerFront() == NULL &&
		lGlobalCameraSettings.GetCameraProducerBottom() == NULL &&
		lGlobalCameraSettings.GetCameraProducerTop() == NULL &&
		lGlobalCameraSettings.GetCameraProducerLeft() == NULL &&
		lGlobalCameraSettings.GetCameraProducerRight() == NULL)
	{
		lGlobalCameraSettings.CreateProducerCameras();
	}

	if (lCurrentCameraName.Compare(FBXSDK_CAMERA_PERSPECTIVE) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerPerspective();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_BOTTOM) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerBottom();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_TOP) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerTop();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_FRONT) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerFront();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_BACK) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerBack();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_RIGHT) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerRight();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_LEFT) == 0)
	{
		return lGlobalCameraSettings.GetCameraProducerLeft();
	}
	else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_SWITCHER) == 0)
	{
		FbxCameraSwitcher *lCameraSwitcher = pScene->GlobalCameraSettings().GetCameraSwitcher();
		FbxAnimCurve *lCurve = NULL;
		if (lCameraSwitcher)
		{
			lCurve = lCameraSwitcher->CameraIndex.GetCurve(pAnimLayer);
			int lCameraIndex = lCurve ? int(lCurve->Evaluate(pTime)) - 1 : 0;
			if (lCameraIndex >= 0 && lCameraIndex != cameraArray.GetCount())
			{
				FbxNode *lNode = cameraArray[lCameraIndex];

				//get the animated parameters of the camera
				GetCameraAnimatedParameters(lNode, pTime, pAnimLayer);
				return (FbxCamera*)lNode->GetNodeAttribute();
			}
		}
	}
	else
	{
		FbxNode *lNode = NULL;

		//find the camera in the camera array
		for (int i = 0; i != cameraArray.GetCount(); ++i)
		{
			lNode = cameraArray[i];
			break;
		}

		if (lNode)
		{
			//get the animated parameters of the camera
			GetCameraAnimatedParameters(lNode, pTime, pAnimLayer);
			return (FbxCamera*)lNode->GetNodeAttribute();
		}
	}
	return lGlobalCameraSettings.GetCameraProducerPerspective();
}

FbxCamera *CameraManager::GetCurrentCamera(FbxScene *pScene)
{
	FbxCamera* lRet = NULL;
	FbxString     lCurrentCameraName;

	FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
	FbxGlobalSettings& lGlobalSettings = pScene->GetGlobalSettings();

	lCurrentCameraName = lGlobalSettings.GetDefaultCamera();

	if (lCurrentCameraName == FBXSDK_CAMERA_PERSPECTIVE)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerPerspective();
	}
	else if (lCurrentCameraName == FBXSDK_CAMERA_TOP)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerTop();
	}
	else if (lCurrentCameraName == FBXSDK_CAMERA_BOTTOM)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerBottom();
	}
	else if (lCurrentCameraName == FBXSDK_CAMERA_FRONT)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerFront();
	}
	else if (lCurrentCameraName == FBXSDK_CAMERA_BACK)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerBack();
	}
	else if (lCurrentCameraName == FBXSDK_CAMERA_RIGHT)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerRight();
	}
	else if (lCurrentCameraName == FBXSDK_CAMERA_LEFT)
	{
		lRet = lGlobalCameraSettings.GetCameraProducerLeft();
	}
	else
	{
		FbxNode* lCameraNode = pScene->FindNodeByName(lCurrentCameraName);
		if (lCameraNode)
		{
			lRet = lCameraNode->GetCamera();
		}
	}
	return lRet;
}

//get the animated parameters of a camera contained in the scene
//and store them in the associated member variables contained in the camera
void CameraManager::GetCameraAnimatedParameters(FbxNode *pNode, FbxTime &pTime, FbxAnimLayer *pAnimLayer)
{
	FbxCamera *lCamera = (FbxCamera*)pNode->GetNodeAttribute();
	lCamera->Position.Set(parser->GetGlobalPosition(pNode, pTime).GetT());

	FbxAnimCurve *fc = lCamera->Roll.GetCurve(pAnimLayer);
	if (fc)
		lCamera->Roll.Set(fc->Evaluate(pTime));
	FbxCamera::EApertureMode lCameraApertureMode = lCamera->GetApertureMode();
	if (lCameraApertureMode == FbxCamera::eHorizontal ||
		lCameraApertureMode == FbxCamera::eVertical)
	{
		double lFieldOfView = lCamera->FieldOfView.Get();
		fc = lCamera->FieldOfView.GetCurve(pAnimLayer);
		if (fc)
			lFieldOfView = fc->Evaluate(pTime);

		//update FOV and focal length
		lCamera->FieldOfView.Set(lFieldOfView);
		lCamera->FocalLength.Set(lCamera->ComputeFocalLength(lFieldOfView));
	}
	else if (lCameraApertureMode == FbxCamera::eHorizAndVert)
	{
		double lOldFieldOfViewX = lCamera->FieldOfViewX.Get();
		double lOldFieldOfViewY = lCamera->FieldOfViewY.Get();

		//update FOV
		double lNewFieldOfViewX = lOldFieldOfViewX;
		double lNewFieldOfViewY = lOldFieldOfViewY;
		fc = lCamera->FieldOfViewX.GetCurve(pAnimLayer);
		if (fc)
			lNewFieldOfViewX = fc->Evaluate(pTime);
		fc = lCamera->FieldOfViewY.GetCurve(pAnimLayer);
		if (fc)
			lNewFieldOfViewY = fc->Evaluate(pTime);
		lCamera->FieldOfViewX.Set(lNewFieldOfViewX);
		lCamera->FieldOfViewY.Set(lNewFieldOfViewY);

		//update aspect
		double lUpdateApertureX = lCamera->GetApertureWidth();
		double lUpdateApertureY = lCamera->GetApertureHeight();
		lUpdateApertureX *= tan(lNewFieldOfViewX * 0.5 * FBXSDK_PI_DIV_180) / tan(lOldFieldOfViewX * 0.5 * FBXSDK_PI_DIV_180);
		lUpdateApertureY *= tan(lNewFieldOfViewY * 0.5 *FBXSDK_PI_DIV_180) / tan(lOldFieldOfViewY * 0.5 * FBXSDK_PI_DIV_180);

		lCamera->FilmWidth.Set(lUpdateApertureX);
		lCamera->FilmHeight.Set(lUpdateApertureY);
		lCamera->FilmAspectRatio.Set(lUpdateApertureX / lUpdateApertureY);

	}
	else if (lCameraApertureMode == FbxCamera::eFocalLength)
	{
		double lFocalLength = lCamera->FocalLength.Get();
		fc = lCamera->FocalLength.GetCurve(pAnimLayer);
		if (fc && fc->Evaluate(pTime))
			lFocalLength = fc->Evaluate(pTime);

		//update FOV and focal length
		lCamera->FocalLength.Set(lFocalLength);
		lCamera->FieldOfView.Set(lCamera->ComputeFieldOfView(lFocalLength));
	}
}

bool CameraManager::IsProducerCamera(FbxScene *pScene, FbxCamera *pCamera)
{
	FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
	if (pCamera == lGlobalCameraSettings.GetCameraProducerPerspective())
		return true;
	if (pCamera == lGlobalCameraSettings.GetCameraProducerTop())
		return true;
	if (pCamera == lGlobalCameraSettings.GetCameraProducerBottom())
		return true;
	if (pCamera == lGlobalCameraSettings.GetCameraProducerFront())
		return true;
	if (pCamera == lGlobalCameraSettings.GetCameraProducerBack())
		return true;
	if (pCamera == lGlobalCameraSettings.GetCameraProducerRight())
		return true;
	if (pCamera == lGlobalCameraSettings.GetCameraProducerLeft())
		return true;

	return false;
}

void CameraManager::CameraOrbit(FbxScene *pScene, int pX, int pY)
{
	int dX = pX - mLastX;
	int dY = mLastY - pY;

	// Orbit the camera horizontally dX degrees, vertically dY degrees.
	FbxCamera* lCamera = GetCurrentCamera(pScene);
	if (!lCamera) return;
	FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
	if (lCamera != lGlobalCameraSettings.GetCameraProducerPerspective()) return;
	if (lCamera->LockMode.Get()) return;
	if (dX == 0 && dY == 0) return;

	FbxVector4 lRotationVector, lNewPosition, lCurPosition;
	FbxAMatrix lRotation;
	FbxVector4 lCenter = lCamera->InterestPosition.Get();

	// current position
	FbxVector4 lPosition = lCamera->Position.Get();
	lCurPosition = lPosition - lCenter;

	// translate
	lNewPosition = mCamPosition - lCenter;

	int rotX;
	if (lNewPosition[2] == 0) {
		rotX = 90;
	}
	else {
		rotX = (int)(atan((double)lNewPosition[0] / (double)lNewPosition[2]) * FBXSDK_180_DIV_PI);
	}
	bool bRoll = (((int)mRoll % 360) != 0);
	if ((lNewPosition[2] < 0 && !bRoll)
		|| (lNewPosition[2] > 0 && bRoll)) {
		dY = -dY;
	}
	if (bRoll) dX = -dX;

	// Center on the X axis (push)
	lRotationVector[1] = -rotX;
	lRotation.SetR(lRotationVector);
	lNewPosition = lRotation.MultT(lNewPosition);
	// Rotation for the vertical movement: around the X axis
	lRotationVector[1] = 0;
	lRotationVector[0] = dY;
	lRotation.SetR(lRotationVector);
	lNewPosition = lRotation.MultT(lNewPosition);
	// Back from the X axis (pop)
	lRotationVector[0] = 0;
	lRotationVector[1] = rotX;
	lRotation.SetR(lRotationVector);
	lNewPosition = lRotation.MultT(lNewPosition);
	// Rotation for the horizontal movement
	lRotationVector[1] = -dX;
	lRotation.SetR(lRotationVector);
	lNewPosition = lRotation.MultT(lNewPosition);

	// Detect camera flip
	if (lNewPosition[0] * lCurPosition[0] < 0
		&& lNewPosition[2] * lCurPosition[2] < 0) {
		// flip -> roll 180.
		double lRoll = lCamera->Roll.Get();
		lRoll = 180.0 - lRoll;
		lCamera->Roll.Set(lRoll);
	}

	// Back from center
	lNewPosition = lNewPosition + lCenter;

	lCamera->Position.Set(lNewPosition);
}

void CameraManager::CameraZoom(FbxScene *pScene, int pY)
{
	int pZoomDepth = mLastY - pY;
	FbxCamera* lCamera = GetCurrentCamera(pScene);
	if (lCamera == NULL)
		return;
	if (mCameraZoomMode == ZOOM_FOCAL_LENGTH)
	{
		if (lCamera->ProjectionType.Get() == FbxCamera::ePerspective)
		{
			double lTransform = 0 - pZoomDepth / 400.0;

			double lApertureW = lCamera->GetApertureWidth();
			lApertureW = TransformAperture(lApertureW, lTransform);

			double lApertureH = lCamera->GetApertureHeight();
			lApertureH = TransformAperture(lApertureH, lTransform);

			UpdatePerspCameraAttributes(lCamera, lApertureW, lApertureH);


		}
		else
		{
			if (pZoomDepth > 0)
				gsOrthoCameraScale *= 0.8;
			else
				gsOrthoCameraScale *= 1.25;
		}
	}
	else
	{
		FbxNode*   lCameraNode = lCamera ? lCamera->GetNode() : NULL;

		// Compute the camera position and direction.
		FbxVector4 lEye(0, 0, 1);
		FbxVector4 lCenter(0, 0, 0);
		FbxVector4 lForward(0, 0, 0);

		if (lCamera)
		{
			lEye = lCamera->Position.Get();
		}

		if (lCameraNode && lCameraNode->GetTarget())
		{
			lCenter = lCameraNode->GetTarget()->LclTranslation.Get();
			lForward = lCenter - lEye;
		}
		else
		{
			if (!lCameraNode || IsProducerCamera(pScene, lCamera))
			{
				if (lCamera)
				{
					lCenter = lCamera->InterestPosition.Get();
					lForward = lCenter - lEye;
				}
			}
			else
			{
				// Get the direction
				FbxAMatrix lGlobalRotation;
				FbxVector4 lRotationVector(lCameraNode->LclRotation.Get());
				lGlobalRotation.SetR(lRotationVector);

				// Set the center.
				// A camera with rotation = {0,0,0} points to the X direction. So create a
				// vector in the X direction, rotate that vector by the global rotation amount
				// and then position the center by scaling and translating the resulting vector
				lRotationVector = FbxVector4(1.0, 0, 0);
				lForward = lGlobalRotation.MultT(lRotationVector);
			}
		}
		lForward.Normalize();
		lEye += lForward * pZoomDepth;
		FbxDouble3 lPosition(lEye[0], lEye[1], lEye[2]);
		lCamera->Position.Set(lPosition);

	}
}

void CameraManager::CameraPan(FbxScene *pScene, int pX, int pY)
{
	int dX = pX - mLastX;
	int dY = mLastY - pY;

	// Pan the camera horizontally dX degrees, vertically dY degrees.
	FbxCamera* lCamera = GetCurrentCamera(pScene);
	if (!lCamera) return;
	if (!IsProducerCamera(pScene, lCamera)) return;
	if (lCamera->LockMode.Get()) return;
	if (dX == 0 && dY == 0) return;

	FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();

	FbxVector4 lRotationXV, lRotationYV, lTranslationV;
	FbxAMatrix lRotationX, lRotationY, lRotationXInverse, lRotationYInverse, lTranslation;
	FbxVector4 lNewPosition = mCamPosition;
	FbxVector4 lNewCenter = mCamCenter;

	// Translate the camera in dX and dY according to its point of view.
	if (lCamera == lGlobalCameraSettings.GetCameraProducerTop()) {
		lTranslationV[0] = -dX;
		lTranslationV[1] = 0;
		lTranslationV[2] = dY;
	}
	else if (lCamera == lGlobalCameraSettings.GetCameraProducerBottom()) {
		lTranslationV[0] = dX;
		lTranslationV[1] = 0;
		lTranslationV[2] = dY;
	}
	else if (lCamera == lGlobalCameraSettings.GetCameraProducerFront()) {
		lTranslationV[0] = -dX;
		lTranslationV[1] = -dY;
		lTranslationV[2] = 0;
	}
	else if (lCamera == lGlobalCameraSettings.GetCameraProducerBack()) {
		lTranslationV[0] = dX;
		lTranslationV[1] = -dY;
		lTranslationV[2] = 0;
	}
	else if (lCamera == lGlobalCameraSettings.GetCameraProducerRight()) {
		lTranslationV[0] = 0;
		lTranslationV[1] = -dY;
		lTranslationV[2] = dX;
	}
	else if (lCamera == lGlobalCameraSettings.GetCameraProducerLeft()) {
		lTranslationV[0] = 0;
		lTranslationV[1] = -dY;
		lTranslationV[2] = -dX;
	}
	else {
		// Perspective view. More computation.

		// Adjust displacement when there is roll
		bool bRoll = (((int)mRoll % 360) != 0);
		if (bRoll) {
			dX = -dX;
			dY = -dY;
		}

		// Compute angles aY and aZ of the camera with Y and Z axis.
		double aZ, aY;
		/// Vector of distance between camera and center (lookat)
		FbxVector4 lDist = lNewPosition - lNewCenter;
		// Euclidian distance between camera and lookat
		double dist = (double)(lDist[0] * lDist[0] + lDist[1] * lDist[1] + lDist[2] * lDist[2]);
		// aZ
		if (lDist[2] == 0) {
			aZ = 90.0;
		}
		else {
			aZ = (atan((double)lDist[0] / (double)lDist[2]) * FBXSDK_180_DIV_PI);
		}
		if (lNewPosition[2] < lNewCenter[2]) aZ += 180;
		// aY
		if (dist > 0.001) {
			aY = (asin(sqrt((double)(lDist[1] * lDist[1]) / dist)) * FBXSDK_180_DIV_PI);
		}
		else {
			aY = 0;
		}
		if (lNewPosition[1] < lNewCenter[1]) aY = -aY;


		// Basis translation
		lTranslationV[0] = -dX;
		lTranslationV[1] = -dY;
		lTranslationV[2] = 0;

		// Rotation around Y axis
		lRotationYV[0] = 0;
		lRotationYV[1] = -aZ;
		lRotationYV[2] = 0;
		lRotationY.SetR(lRotationYV);
		// Rotation around X axis
		lRotationXV[0] = aY;
		lRotationXV[1] = 0;
		lRotationXV[2] = 0;
		lRotationX.SetR(lRotationXV);

		// Modify translation according to aY and aZ.
		lTranslation.SetT(lTranslationV);
		lRotationYInverse = lRotationY.Inverse();
		lRotationXInverse = lRotationX.Inverse();
		lTranslation = lRotationYInverse * lRotationXInverse * lTranslation * lRotationY * lRotationX;
		lTranslationV = lTranslation.GetT();
	}

	// Translate camera and center according to pan.
	lNewPosition += lTranslationV;
	lNewCenter += lTranslationV;

	lCamera->Position.Set(lNewPosition);
	lCamera->InterestPosition.Set(lNewCenter);
}


double CameraManager::TransformAperture(double pAperture, double pTransform)
{
	double lTransformAperture = (pAperture + pTransform);
	if (lTransformAperture < 0.25)
	{
		lTransformAperture = 0.25;
	}
	if (lTransformAperture  > 179.0)
	{
		lTransformAperture = 179.0;
	}
	return lTransformAperture;
}

void CameraManager::UpdatePerspCameraAttributes(FbxCamera* pCamera, double pNewApertureW, double pNewApertureH)
{

	if (pCamera == NULL || FbxAnimUtilities::IsAnimated(pCamera))
		return;
	// update focal length or field of view.
	double lApertureWidthOrig = pCamera->GetApertureWidth();
	double lApertureHeightOrig = pCamera->GetApertureHeight();

	if (pCamera->GetApertureMode() == FbxCamera::eFocalLength) {
		// update focal length according to hypothetic new apertures.
		double lFLOrig = pCamera->FocalLength.Get();
		double lFOVOrig = pCamera->ComputeFieldOfView(lFLOrig); // recompute to be sure.
		// transform aperture width and height temporarily
		pCamera->SetApertureWidth(pNewApertureW);
		pCamera->SetApertureHeight(pNewApertureH);
		double lFLNew = pCamera->ComputeFocalLength(lFOVOrig);
		double lFOVNew = pCamera->ComputeFieldOfView(lFLNew);
		pCamera->FocalLength.Set(lFLNew);
		pCamera->FieldOfView.Set(lFOVNew);
	}
	else if (pCamera->GetApertureMode() == FbxCamera::eVertical
		|| pCamera->GetApertureMode() == FbxCamera::eHorizontal) {
		double lFOVOrig = pCamera->FieldOfView.Get();
		double lFLOrig = pCamera->ComputeFocalLength(lFOVOrig); // recompute to be sure.
		// transform aperture width and height temporarily
		pCamera->SetApertureWidth(pNewApertureW);
		pCamera->SetApertureHeight(pNewApertureH);
		double lFOVNew = pCamera->ComputeFieldOfView(lFLOrig);
		double lFLNew = pCamera->ComputeFocalLength(lFOVNew);
		pCamera->FieldOfView.Set(lFOVNew);
		pCamera->FocalLength.Set(lFLNew);
	}
	else if (pCamera->GetApertureMode() == FbxCamera::eHorizAndVert) {
		double lFOVOrigX = pCamera->FieldOfViewX.Get();
		double lFLOrig = pCamera->ComputeFocalLength(lFOVOrigX); // recompute to be sure.
		// transform aperture width and height temporarily
		pCamera->SetApertureWidth(pNewApertureW);
		pCamera->SetApertureHeight(pNewApertureH);
		double lFOVNewX = pCamera->ComputeFieldOfView(lFLOrig);
		double lFOVNewY = pCamera->ComputeFieldOfView(lFLOrig);
		double lFLNew = pCamera->ComputeFocalLength(lFOVNewX);
		pCamera->FieldOfViewY.Set(lFOVNewX);
		pCamera->FieldOfViewY.Set(lFOVNewY);
		pCamera->FocalLength.Set(lFLNew);
	}
	// reset aperture width and height
	pCamera->SetApertureWidth(lApertureWidthOrig);
	pCamera->SetApertureHeight(lApertureHeightOrig);
}