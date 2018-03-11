#ifndef UTILITIES_H
#define UTILITIES_H

#include <fbxsdk.h>
#include <vector>
using std::vector;


//This stores the weight of each Control Point
struct IndexWeightPair
{
	unsigned int index;	//index of Bone 
	double weight;		//weight of inifluence by the bone
	IndexWeightPair() :
		index(0), weight(0.0)
	{}
};

struct ControlPointInfo
{
	FbxVector4 ctrlPoint;
	vector<IndexWeightPair> weightPairs;
};

//This stores the information of each key frame of each Bone
struct KeyFrame{
	FbxLongLong frameNum;
	FbxAMatrix globalTransform;	//transform matrix
	KeyFrame *next;
	KeyFrame() :
		next(nullptr)
	{}
};


//This represents the bone of FBX model 
struct Bone{
	FbxString boneName;
	int currentIndex;	//index of current bone	
	int parentIndex;	//index to its parent bone
	FbxAMatrix globalBindPoseInverse;
	KeyFrame *animation;
	FbxNode *node;

	Bone() :
		animation(nullptr),
		node(nullptr)
	{
		globalBindPoseInverse.SetIdentity();
		parentIndex = -1;
	}
	~Bone()
	{
		while (animation)
		{
			KeyFrame *temp = animation->next;
			delete animation;
			animation = temp;
		}
	}
};


#endif