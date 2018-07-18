#pragma once
#include<fbxsdk.h>

enum CameraZoomMode
{
	ZOOM_FOCAL_LENGTH,
	ZOOM_POSITION
};

enum CameraStatus
{
	CAMERA_NOTHING,
	CAMERA_ORBIT,
	CAMERA_ZOOM,
	CAMERA_PAN
};
class CameraManager{
public:
	void SetCamera(FbxScene *pScene, FbxTime &pTime, FbxAnimLayer *pAnimLayer,
					const FbxArray<FbxNode*> &cameraArray, int windowWidth, int windowHeight);

	FbxCamera* GetCurrentCamera(FbxScene *pScene, FbxTime &pTime,
					FbxAnimLayer *pAnimLayer, const FbxArray<FbxNode*>& cameraArray);

	FbxCamera* GetCurrentCamera(FbxScene *pScene);

	void GetCameraAnimatedParameters(FbxNode *pNode, FbxTime &pTime, FbxAnimLayer *pAnimLayer);

	bool IsProducerCamera(FbxScene *pScene, FbxCamera *pCamera);

	void CameraOrbit(FbxScene *pScene, int pX, int pY);
	void CameraZoom(FbxScene *pScene, int pY);
	void CameraPan(FbxScene *pScene, int pX, int pY);

	double TransformAperture(double pAperture, double pTransform);
	void UpdatePerspCameraAttributes(FbxCamera* pCamera, double pNewApertureW, double pNewApertureH);

	CameraStatus GetCameraStatus(){ return mCameraStatus; }
	CameraZoomMode  GetZoomMode()        { return mCameraZoomMode; }
	
	bool SetCurrentCamera(const char *pCameraName);
	void SetZoomMode(CameraZoomMode pZoomMode);
	void SetCamStatus(CameraStatus cameraStatus){ mCameraStatus = cameraStatus; }
	void SetCamPosition(FbxVector4 camPosition){ mCamPosition = camPosition; }
	void SetCamCenter(FbxVector4 camCenter){ mCamCenter = camCenter; }
	void SetRoll(double roll){ mRoll = roll; }
	void SetLastX(int lastX){ mLastX = lastX; }
	void SetLastY(int lastY){ mLastY = lastY; }

private:
	
	// Data for camera manipulation
	mutable int mLastX, mLastY;
	mutable FbxVector4 mCamPosition, mCamCenter;
	mutable double mRoll;
	mutable CameraStatus mCameraStatus;

	//camera zoom mode
	CameraZoomMode mCameraZoomMode;
	bool mPause;
};
