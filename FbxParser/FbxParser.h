#ifndef FBXPARSER_H
#define FBXPARSER_H

#include "common.h"
#include <vector>
using std::vector;

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

class FbxParser
{
public:
	FbxParser(FbxString fbxFile);	//constructor
	~FbxParser();			//deconstructor

	FbxParser(const FbxParser&) = delete;	//copy constructor/operator= is forbidden
	FbxParser& operator=(const FbxParser&) = delete;
	
	FbxManager* getFbxManager() { return pManager; }	//get FbxManager
	FbxScene* getFbxScene(){ return pScene; }	//get FbxScene
	FbxString getFbxFileName() { return fbxFile; }		//get file name
	FbxVector4* getControlPoints() { return controlPoints; }	//get control points of the fbx model
	int getPolygonCount() { return polygonCount; }		//get polygon count
	vector<FbxVector4> getPolygonPoints() { return polygonPoints; }		//get polygon points
	vector<FbxVector4> getNormals() { return normals; }		//get normals
	vector<FbxVector2> getTextureUVs() { return uvs; }		//get texture uvs
	FbxMesh* getFbxMesh(){ return pMesh; }		//get FbxMesh
	void setFbxMesh(FbxMesh *mesh){ this->pMesh = pMesh; }	//set FbxMesh, it's useless actually

	void setTextureFileName(FbxString texFile){ textureFile = texFile; }	
	FbxString getTextureFileName() { return textureFile; }

	bool loadScene();		//load scene,return false if failed
	void displayMetaData(FbxScene *pScene);		//display meta data
	void displayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings);		//display global light settings
	void displayHierarchy(FbxScene *pScene);		//display hierarchy of the fbx model
	void displayContent(FbxScene *pScene);		//display info of all the nodes, such as mesh, skeleton, marker, etc
	void displayPose(FbxScene *pScene);		//display pose of the input model

	void covertFormat();	//convert format

private:
	FbxManager *pManager;
	FbxScene *pScene;
	FbxMesh *pMesh;
	FbxString fbxFile;
	FbxString textureFile;

	FbxVector4 *controlPoints;
	vector<FbxVector4> polygonPoints;
	int polygonCount;

	vector<FbxVector4> normals;
	vector<FbxVector2> uvs;

	void getNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, vector<FbxVector4> &normals);
	void getTextureUV(FbxMesh *mesh, vector<FbxVector2> &uvs);

	void initFbxObjects();	//initialize FbxManage, FbxScene,etc

	void displayHierarchy(FbxNode *node, int depth);	//used for displayHierarchy(FbxScene*)

	void displayContent(FbxNode *node);			//used for displayContent(FbxScene*)
	//functions below are called inside of displayContent
	void displayMarker(FbxNode *node);				
	void displaySkeleton(FbxNode *node);
	void displayMesh(FbxNode *node);
	void displayTexture(FbxNode *node);

	void displayTexture(FbxScene *pScene);

};
 
#endif 