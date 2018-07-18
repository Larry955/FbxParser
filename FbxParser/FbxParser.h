#ifndef FBXPARSER_H
#define FBXPARSER_H

#include "Utilities.h"
#include <vector>
#include <unordered_map>
using std::vector;
using std::unordered_map;

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

enum Status {
	UNLOADED,			//unload file or load file failed
	MUST_BE_LOADED,		//ready for loading file
	MUST_BE_REFRESHED,	//something changed and redraw needed
	REFRESHED			//no redraw needed
};

class FbxParser
{
public:
	FbxParser(FbxString fbxFile);	//constructor
	~FbxParser();			//deconstructor

	FbxParser(const FbxParser&) = delete;	//copy constructor/operator= is forbidden
	FbxParser& operator=(const FbxParser&) = delete;

	bool LoadScene();		//load scene,return false if failed
	void DisplayMetaData(FbxScene *pScene);		//Display meta data
	void DisplayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings);		//Display global light settings
	void DisplayHierarchy(FbxScene *pScene);		//Display hierarchy of the fbx model
	void DisplayContent(FbxScene *pScene);		//Display info of all the nodes, such as mesh, skeleton, marker, etc
	void DisplayPose(FbxScene *pScene);		//Display pose of the input model

	void CovertFormat();	//convert format

	FbxManager* GetFbxManager() { return pManager; }	//get FbxManager
	FbxScene* GetFbxScene(){ return pScene; }	//get FbxScene
	FbxString GetFbxFileName() { return fbxFile; }		//get file name
	
	int GetPolygonCount() { return polygonCount; }		//get polygon count
	vector<FbxVector4> GetPolygonPoints() { return polygonPoints; }		//get polygon points
	vector<FbxVector4> GetNormals() { return normals; }		//get normals
	vector<FbxVector2> GetTextureUVs() { return uvs; }		//get texture uvs
	FbxMesh* GetFbxMesh(){ return pMesh; }		//get FbxMesh
	void SetFbxMesh(FbxMesh *mesh){ this->pMesh = pMesh; }	//set FbxMesh, it's useless actually

	void SetTextureFileName(FbxString texFile){ textureFile = texFile; }
	FbxString GetTextureFileName() { return textureFile; }
	
	FbxTime GetFrameTime() { return frameTime; }
	FbxTime GetStartTime() { return startTime; }
	FbxTime GetStopTime() { return stopTime; }
	FbxTime GetCurrentTime() { return currentTime; }
	FbxAnimLayer* GetAnimLayer() { return currAnimLayer; }
	FbxArray<FbxString*> GetAnimStackArray(){ return animStackNameArray; }
	Skeleton GetSkeleton() { return skeleton; }
	const FbxArray<FbxNode*> &GetCameraArray()const { return cameraArray; }
	void SetFrameTime(FbxTime frameTime) { this->frameTime = frameTime; }
	void SetCurrentTime(FbxTime currentTime) { this->currentTime = currentTime; }
	void SetStartTime(FbxTime startTime) { this->startTime = startTime; }
	void SetStopTime(FbxTime stopTime) { this->stopTime = stopTime; }
	void SetAnimStatus(Status animStatus) { this->animStatus = animStatus; }
	void SetShadingMode(ShadingMode shadingMode)
	{ 
		this->shadingMode = shadingMode; 
		animStatus = MUST_BE_REFRESHED;
	}

	void SetCurrAnimStack(int animIndex);
	void OnTimerClick();
	void LoadCacheRecursive(FbxScene * pScene, FbxAnimLayer * pAnimLayer, const char * pFbxFileName, bool pSupportVBO);

	FbxAMatrix GetGlobalPosition(FbxNode *node, FbxTime currTime, FbxAMatrix *parentGlobalMatrix= NULL);
	FbxAMatrix GetGeometry(FbxNode *node);
	Status GetStatus() const { return animStatus; }
	ShadingMode GetShadingMode() const { return shadingMode; }
private:
	FbxManager *pManager;
	FbxScene *pScene;
	FbxMesh *pMesh;
	FbxString fbxFile;
	FbxString textureFile;
	FbxString animationName;
	FbxLongLong animationLength;
	unordered_map<int, ControlPointInfo> controlPointsInfo;
	vector<FbxVector4> polygonPoints;
	int polygonCount;

	FbxArray<FbxString*> animStackNameArray;
	FbxArray<FbxNode*> cameraArray;
	FbxAnimLayer *currAnimLayer;
	FbxTime startTime;
	FbxTime stopTime;
	FbxTime frameTime;
	FbxTime currentTime;
	Status animStatus;
	ShadingMode shadingMode;

	vector<FbxVector4> normals;
	vector<FbxVector2> uvs;

	Skeleton skeleton;

	void GetNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, vector<FbxVector4> &normals);
	void GetTextureUV(FbxMesh *mesh, vector<FbxVector2> &uvs);

	void InitFbxObjects();	//initialize FbxManage, FbxScene,etc

	void DisplayHierarchy(FbxNode *node, int depth, int currIndex, int parentIndex);	//used for DisplayHierarchy(FbxScene*)

	void DisplayContent(FbxNode *node);			//used for DisplayContent(FbxScene*)
	//functions below are called inside of DisplayContent
	void DisplayMarker(FbxNode *node);				
	void DisplaySkeleton(FbxNode *node);
	void DisplayMesh(FbxNode *node);
	void DisplayTexture(FbxNode *node);
	void DisplayTexture(FbxScene *pScene);

	void FillCameraArray(FbxNode *pNode, FbxArray<FbxNode*> &pCameraArray);
	void ProcessJointsAndAnimations(FbxNode *node);
	FbxAMatrix GetGeometryTransformation(FbxNode *node);
	int FindJointIndexByName(const FbxString& jointName);
	FbxMatrix CalcGlobalMatrix(int i);
	void DebugSumOfWeights();
	void LoadCacheRecursive(FbxNode * pNode, FbxAnimLayer * pAnimLayer, bool pSupportVBO);
	bool LoadTextureFromFile(const FbxString & pFilePath, unsigned int & pTextureObject);

};
 
#endif 