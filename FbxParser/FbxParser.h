#ifndef FBXPARSER_H
#define FBXPARSER_H

#include "common.h"

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

class FbxParser
{
public:
	FbxParser(FbxString fbxFile);
	~FbxParser();

	FbxParser(const FbxParser&) = delete;
	FbxParser& operator=(const FbxParser&) = delete;
	
	FbxManager* getFbxManager(){ return pManager; }
	FbxScene* getFbxScene(){ return pScene; }
	FbxVector4* getFbxVector4(){ return controlPoints; }
	FbxMesh* getFbxMesh(){ return pMesh; }
	void setFbxMesh(FbxMesh *mesh){ this->pMesh = pMesh; }

	bool loadScene();	//load scene,return false if failed
	void displayMetaData(FbxScene *pScene);
	void displayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings);
	void displayHierarchy(FbxScene *pScene);
	void displayContent(FbxScene *pScene);


private:
	FbxManager *pManager;
	FbxScene *pScene;
	FbxMesh *pMesh;
	FbxString fbxFile;
	FbxVector4 *controlPoints;
	void initFbxObjects();	//initialize FbxManage, FbxScene,etc

	void displayHierarchy(FbxNode *node, int depth);
	void displayContent(FbxNode *node);

	void displayMarker(FbxNode *node);
	void displaySkeleton(FbxNode *node);
	void displayMesh(FbxNode *node);
	void displayTexture(FbxNode *node);
};
 
#endif 