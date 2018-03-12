#ifndef UTILITIES_H
#define UTILITIES_H

#include <fbxsdk.h>
#include <vector>
using std::vector;

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
	FbxAMatrix globalBindPoseInverse;
	KeyFrame *animation;
	FbxNode *node;

	Joint() :
		animation(nullptr),
		node(nullptr)
	{
		globalBindPoseInverse.SetIdentity();
		parentIndex = -1;
	}
	/*~Joint()
	{
		while (animation)
		{
			KeyFrame *temp = animation->next;
			delete animation;
			animation = temp;
		}
	}*/
};

struct Skeleton{
	vector<Joint> joints;
};

#endif