#ifndef UTILITIES_H
#define UTILITIES_H

#include <fbxsdk.h>
#include <vector>
using std::vector;


enum ShadingMode
{
	SHADING_MODE_WIREFRAME,
	SHADING_MODE_SHADED,
};


//This stores the weight of each Control Point
struct IndexWeightPair
{
	unsigned int index;	//index of joint 
	double weight;		//weight of influence by the joint
	IndexWeightPair() :
		index(0), weight(0.0)
	{}
};

struct ControlPointInfo
{
	FbxVector4 ctrlPoint;
	vector<IndexWeightPair> weightPairs;
};

//This stores the information of each key frame of each Joint
struct KeyFrame{
	FbxLongLong frameNum;
	FbxAMatrix globalTransform;	//transform matrix
	KeyFrame *next;
	KeyFrame() :
		next(nullptr)
	{}
};


//This represents the joint of FBX model 
struct Joint{
	FbxString jointName;
	int currentIndex;	//index of current joint	
	int parentIndex;	//index to its parent joint
	FbxAMatrix globalMatrix;
	FbxAMatrix localMatrix;
	KeyFrame *animation;
	FbxNode *node;

	Joint() :
		animation(nullptr),
		node(nullptr)
	{
		localMatrix.SetIdentity();
		globalMatrix.SetIdentity();
		parentIndex = -1;
	}
};

struct Skeleton{
	vector<Joint> joints;
};

void MatrixScale(FbxAMatrix &influence, double weight);
void MatrixAddToDiagonal(FbxAMatrix &influence, double value);
void MatrixAdd(FbxAMatrix &mat1, const FbxAMatrix &mat2);

void Display3DVector(const char *prefix, FbxVector4 pValue);
void DisplayInfoOnce(const char *prefix, int index, FbxString info = "");
void DisplayString(const char* pHeader, const char* pValue, const char* pSuffix = "");
void DisplayBool(const char* pHeader, bool pValue, const char* pSuffix = "");
void DisplayInt(const char* pHeader, int pValue, const char* pSuffix = "");

bool IsNotEmpty(FbxString str);
#endif