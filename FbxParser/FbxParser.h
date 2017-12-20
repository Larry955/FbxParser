#ifndef FBXPARSER_H
#define FBXPARSER_H

#include <fbxsdk.h>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

class FbxParser
{
public:
	FbxParser();
	~FbxParser();
	bool loadScene(FbxManager *pManager, FbxScene *pScene, FbxString fbxFile);	//load scene,return false if failed
private:
	FbxManager *pManager;
	FbxScene *pScene;
	void initFbxObjects();	//initialize FbxManage, FbxScene,etc
};
 
#endif 