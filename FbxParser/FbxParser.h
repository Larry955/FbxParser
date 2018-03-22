#ifndef FBXPARSER_H
#define FBXPARSER_H

#include "common.h"
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

	

	bool loadScene();		//load scene,return false if failed
	void displayMetaData(FbxScene *pScene);		//display meta data
	void displayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings);		//display global light settings
	void displayHierarchy(FbxScene *pScene);		//display hierarchy of the fbx model
	void displayContent(FbxScene *pScene);		//display info of all the nodes, such as mesh, skeleton, marker, etc
	void displayPose(FbxScene *pScene);		//display pose of the input model

	void covertFormat();	//convert format


	FbxManager* getFbxManager() { return pManager; }	//get FbxManager
	FbxScene* getFbxScene(){ return pScene; }	//get FbxScene
	FbxString getFbxFileName() { return fbxFile; }		//get file name
	
	int getPolygonCount() { return polygonCount; }		//get polygon count
	vector<FbxVector4> getPolygonPoints() { return polygonPoints; }		//get polygon points
	vector<FbxVector4> getNormals() { return normals; }		//get normals
	vector<FbxVector2> getTextureUVs() { return uvs; }		//get texture uvs
	FbxMesh* getFbxMesh(){ return pMesh; }		//get FbxMesh
	void setFbxMesh(FbxMesh *mesh){ this->pMesh = pMesh; }	//set FbxMesh, it's useless actually

	void setTextureFileName(FbxString texFile){ textureFile = texFile; }
	FbxString getTextureFileName() { return textureFile; }
	
	
	FbxTime getFrameTime() { return frameTime; }
	FbxTime getStartTime() { return startTime; }
	FbxTime getStopTime() { return stopTime; }
	FbxTime getCurrentTime() { return currentTime; }
	FbxAnimLayer* getAnimLayer() { return currAnimLayer; }
	

	void setFrameTime(FbxTime frameTime) { this->frameTime = frameTime; }
	void setCurrentTime(FbxTime currentTime) { this->currentTime = currentTime; }
	void setStartTime(FbxTime startTime) { this->startTime = startTime; }
	void setStopTime(FbxTime stopTime) { this->stopTime = stopTime; }
	void setAnimStatus(Status animStatus) { this->animStatus = animStatus; }
	void setCurrAnimStack(int animIndex);
	void onTimerClick();

	FbxAMatrix getGlobalPosition(FbxNode *node, FbxTime currTime, FbxAMatrix *parentGlobalMatrix);
	FbxAMatrix getGeometry(FbxNode *node);
	Status getStatus() const { return animStatus; }
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
	FbxAnimLayer *currAnimLayer;
	FbxTime startTime;
	FbxTime stopTime;
	FbxTime frameTime;
	FbxTime currentTime;
	Status animStatus;

	
	vector<FbxVector4> normals;
	vector<FbxVector2> uvs;

	Skeleton skeleton;
	

	void getNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, vector<FbxVector4> &normals);
	void getTextureUV(FbxMesh *mesh, vector<FbxVector2> &uvs);

	void initFbxObjects();	//initialize FbxManage, FbxScene,etc

	void displayHierarchy(FbxNode *node, int depth, int currIndex, int parentIndex);	//used for displayHierarchy(FbxScene*)

	void displayContent(FbxNode *node);			//used for displayContent(FbxScene*)
	//functions below are called inside of displayContent
	void displayMarker(FbxNode *node);				
	void displaySkeleton(FbxNode *node);
	void displayMesh(FbxNode *node);
	void displayTexture(FbxNode *node);

	void displayTexture(FbxScene *pScene);

	void processJointsAndAnimations(FbxNode *node);
	FbxAMatrix getGeometryTransformation(FbxNode *node);
	int findJointIndexByName(const FbxString& jointName);
	FbxMatrix calcGlobalMatrix(int i);
	void debugSumOfWeights();
};
 
#endif 