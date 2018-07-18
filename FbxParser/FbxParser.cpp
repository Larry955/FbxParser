#include "FbxParser.h"
#include <assert.h>
#include <math.h>
#include "VBOMesh.h"
#include "FreeImage.h"

FbxParser::FbxParser(FbxString fbxFile) :
pManager(nullptr), pScene(nullptr), pMesh(nullptr), fbxFile(fbxFile), 
textureFile(""), controlPointsInfo(), polygonPoints(), polygonCount(), skeleton(), animStatus(UNLOADED), shadingMode(SHADING_MODE_SHADED)
{
	InitFbxObjects();
}

FbxParser::~FbxParser()
{
	pManager->Destroy();
}

void FbxParser::InitFbxObjects()
{
	//create the FBX manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if (!pManager) {
		FBXSDK_printf("error: unable to create FBX manager!\n");
		exit(1);
	}
	else
		FBXSDK_printf("Autodesk FBX SDK version:%s\n", pManager->GetVersion());

	//create an IOSettings object. This object holds all import/export settings
	FbxIOSettings *ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//load plugins from the executable directory
	FbxString path = FbxGetApplicationDirectory();
	FBXSDK_printf("application path: %s\n", path.Buffer());
	pManager->LoadPluginsDirectory(path.Buffer());

	//create an FBX scene. This object holds most objects imported/exported from/to files
	pScene = FbxScene::Create(pManager, "my scene");
	if (!pScene) {
		FBXSDK_printf("error: unable to create FBX scene\n");
		exit(1);
	}
}

bool FbxParser::LoadScene()
{
	FbxString fullFbxFile = fbxFile + ".fbx";
	if (!FbxFileUtils::Exist(fullFbxFile))
	{
		return false;
	}
	assert(pManager != nullptr && pScene != nullptr);

	int animStackCount = 0;
	bool status = false;
	//create an importer
	FbxImporter *importer = FbxImporter::Create(pManager, "");

	//initialize the importer by providing a file name
	const bool imorterStatus = importer->Initialize(fbxFile + ".fbx", -1, pManager->GetIOSettings());
	if (!imorterStatus) 
	{
		FBXSDK_printf("error: initialize importer failed\n");
		return status;
	}

	if (importer->IsFBX()) 
	{
		FBXSDK_printf("\n\n-------Animation Stack Information-------\n\n");

		animStackCount = importer->GetAnimStackCount();
		FBXSDK_printf("number of animation stacks: %d\n", animStackCount);
		FBXSDK_printf("current animation stack: %s\n", importer->GetActiveAnimStackName().Buffer());

		for (int i = 0; i != animStackCount; ++i) 
		{
			FbxTakeInfo *takeInfo = importer->GetTakeInfo(i);

			FBXSDK_printf("animation stack %d\n", i);
			FBXSDK_printf("	name: %s\n", takeInfo->mName.Buffer());
			FBXSDK_printf("	description: %s\n", takeInfo->mDescription.Buffer());
			FBXSDK_printf("	import name: %s\n", takeInfo->mImportName.Buffer());
			FBXSDK_printf("	import state: %s\n\n", takeInfo->mSelect ? "true" : "false");

			// Set the import states. By default, the import states are always set to 
			// true. The code below shows how to change these states.
			IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
			IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
			IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
			IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
			IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
			IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
			IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
		}

		//import the scene
		status = importer->Import(pScene);
		char password[1024];
		if (status == false && importer->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			FBXSDK_printf("Please enter password: ");
			password[0] = '\0';

			FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
				scanf("%s", password);
			FBXSDK_CRT_SECURE_NO_WARNING_END

				FbxString pwd(password);

			IOS_REF.SetStringProp(IMP_FBX_PASSWORD, pwd);
			IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

			status = importer->Import(pScene);
			if (status == false && importer->GetStatus().GetCode() == FbxStatus::ePasswordError)
			{
				FBXSDK_printf("\nPassword is wrong, import aborted.\n");
			}
			
		}
		if (status) 
		{
			animStatus = MUST_BE_LOADED;
			
			//convert axis system to what is used in this example, if needed
			FbxAxisSystem sceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();
			FbxAxisSystem localAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
			if (sceneAxisSystem != localAxisSystem) 
			{
				localAxisSystem.ConvertScene(pScene);
			}

			//convert axis system to what is used in this example, if needed
			FbxSystemUnit sceneSystemUnit = pScene->GetGlobalSettings().GetSystemUnit();
			if (sceneSystemUnit.GetScaleFactor() != 1.0) 
			{
				//this unit in this example is centimeter
				FbxSystemUnit::cm.ConvertScene(pScene);
			}

			//get the list of all the animation stack
			pScene->FillAnimStackNameArray(animStackNameArray);
			
			cameraArray.Clear();
			FillCameraArray(pScene->GetRootNode(), cameraArray);
			//convert mesh, NURBS and patch into triangle mesh
			FbxGeometryConverter geomConverter(pManager);	//this is not necessary since I have considered the situation of quad mesh
			geomConverter.Triangulate(pScene, true);

			//bake the scene for one frame
			//LoadCacheRecursive(pScene->GetRootNode(), currAnimLayer, true);

			//initialize the frame period
			//SetCurrAnimStack(0);
			frameTime.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
		} 
		else
		{
			animStatus = UNLOADED;
		}
	}
	
	//destroy the importer
	importer->Destroy();

	return status;
}

void FbxParser::FillCameraArray(FbxNode *pNode, FbxArray<FbxNode*> &pCameraArray)
{
	//if current node is Camera
	if (pNode && pNode->GetNodeAttribute() &&
		pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera)
				pCameraArray.Add(pNode);		//add current camera into pCameraArray

	const int childCount = pNode->GetChildCount();
	for (int i = 0; i != childCount; ++i)
		FillCameraArray(pNode->GetChild(i), pCameraArray);
}

void FbxParser::OnTimerClick()
{
	if (stopTime > startTime) 
	{
		//set the scene status flag to refresh 
		//the scene in the next timer call back
		animStatus = MUST_BE_REFRESHED;
		//FBXSDK_printf("before currentTime: %d	frameTime: %d	stopTime: %d\n", currentTime.GetMilliSeconds(), frameTime.GetMilliSeconds(), stopTime.GetMilliSeconds());
		currentTime += frameTime;
		//FBXSDK_printf("after currentTime: %d	frameTime: %d	stopTime: %d\n", currentTime.GetMilliSeconds(), frameTime.GetMilliSeconds(), stopTime.GetMilliSeconds());
		
		if (currentTime > stopTime) 
		{
			currentTime = startTime;
		}
	}
	// Avoid displaying the same frame on 
	// and on if the animation stack has no length.
	else
	{
		// Set the scene status flag to avoid refreshing 
		// the scene in the next timer callback.
		animStatus = REFRESHED;
	}
}

void FbxParser::LoadCacheRecursive(FbxScene * pScene, FbxAnimLayer * pAnimLayer, const char * pFbxFileName, bool pSupportVBO)
{
	//获取贴图个数
	const int lTextureCount = pScene->GetTextureCount();
	for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
	{
		FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);	//获取贴图对象
		FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
		if (lFileTexture && !lFileTexture->GetUserDataPtr())
		{
			// Try to load the texture from absolute path
			const FbxString lFileName = lFileTexture->GetFileName();	//获取贴图文件名

			GLuint lTextureObject = 0;
			bool lStatus = LoadTextureFromFile(lFileName, lTextureObject);

			const FbxString lAbsFbxFileName = FbxPathUtils::Resolve(pFbxFileName);
			const FbxString lAbsFolderName = FbxPathUtils::GetFolderName(lAbsFbxFileName);	//获取绝对路径
			if (!lStatus)
			{
				// Load texture from relative file name (relative to FBX file)
				const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lFileTexture->GetRelativeFileName());
				lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
			}

			if (!lStatus)
			{
				// Load texture from file name only (relative to FBX file)
				const FbxString lTextureFileName = FbxPathUtils::GetFileName(lFileName);
				const FbxString lResolvedFileName = FbxPathUtils::Bind(lAbsFolderName, lTextureFileName);
				lStatus = LoadTextureFromFile(lResolvedFileName, lTextureObject);
			}

			if (!lStatus)
			{
				FBXSDK_printf("Failed to load texture file: %s\n", lFileName.Buffer());
				continue;
			}

			if (lStatus)
			{
				GLuint * lTextureName = new GLuint(lTextureObject);
				lFileTexture->SetUserDataPtr(lTextureName);
			}
		}
	}

	LoadCacheRecursive(pScene->GetRootNode(), pAnimLayer, pSupportVBO);
}

void FbxParser::LoadCacheRecursive(FbxNode * pNode, FbxAnimLayer * pAnimLayer, bool pSupportVBO)
{

	// Bake material and hook as user data.
	const int lMaterialCount = pNode->GetMaterialCount();
	for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
	{
		FbxSurfaceMaterial * lMaterial = pNode->GetMaterial(lMaterialIndex);
		if (lMaterial && !lMaterial->GetUserDataPtr())
		{
			FbxAutoPtr<MaterialCache> lMaterialCache(new MaterialCache);
			if (lMaterialCache->Initialize(lMaterial))
			{
				lMaterial->SetUserDataPtr(lMaterialCache.Release());
			}
		}
	}

	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute)
	{
		// Bake mesh as VBO(vertex buffer object) into GPU.
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh * lMesh = pNode->GetMesh();
			if (pSupportVBO && lMesh && !lMesh->GetUserDataPtr())
			{
				FbxAutoPtr<VBOMesh> lMeshCache(new VBOMesh);
				if (lMeshCache->Initialize(lMesh))
				{
					lMesh->SetUserDataPtr(lMeshCache.Release());
				}
			}
		}
		// Bake light properties.
		else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eLight)
		{
			FbxLight * lLight = pNode->GetLight();
			if (lLight && !lLight->GetUserDataPtr())
			{
				FbxAutoPtr<LightCache> lLightCache(new LightCache);
				if (lLightCache->Initialize(lLight, pAnimLayer))
				{
					lLight->SetUserDataPtr(lLightCache.Release());
				}
			}
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		LoadCacheRecursive(pNode->GetChild(lChildIndex), pAnimLayer, pSupportVBO);
	}
}

bool FbxParser::LoadTextureFromFile(const FbxString & pFilePath, unsigned int & pTextureObject)
{
	// Transfer the texture data into GPU
	glGenTextures(1, &pTextureObject);
	glBindTexture(GL_TEXTURE_2D, pTextureObject);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(pFilePath.Buffer(), 0);
	if (fifmt == FIF_UNKNOWN)
	{
		fifmt = FreeImage_GetFIFFromFilename(pFilePath.Buffer());
		if (fifmt == FIF_UNKNOWN)
			return false;
	}
	
	FIBITMAP *dib = FreeImage_Load(fifmt, pFilePath.Buffer(), 0);
	dib = FreeImage_ConvertTo24Bits(dib);

	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);
	BYTE *pixels = (BYTE*)FreeImage_GetBits(dib);
	if (!pixels)
		return false;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0
								, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	FreeImage_Unload(dib);

	return true;
}

void FbxParser::SetCurrAnimStack(int animIndex)
{
	const int animStackCount = animStackNameArray.GetCount();
	if (!animStackCount || animIndex >= animStackCount) {
		return;
	}

	//select the base layer from the animation stack
	FbxAnimStack *currAnimStack = pScene->FindMember<FbxAnimStack>(animStackNameArray[animIndex]->Buffer());

	if (!currAnimStack) {
		throw std::exception("error: currAnimStack should exist\n");
		return;
	}

	//we assume that the first animation layer connected to the animation stack is the base layer
	currAnimLayer = currAnimStack->GetMember<FbxAnimLayer>();
	pScene->SetCurrentAnimationStack(currAnimStack);

	//get current animation infos
	FbxTakeInfo *currTakeInfo = pScene->GetTakeInfo((*animStackNameArray[animIndex]));
	if (currTakeInfo) {
		startTime = currTakeInfo->mLocalTimeSpan.GetStart();		//start time
		stopTime = currTakeInfo->mLocalTimeSpan.GetStop();		//stop time
	}
	else {		
		FbxTimeSpan timeLineSpan;
		pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(timeLineSpan);
		startTime = timeLineSpan.GetStart();
		stopTime = timeLineSpan.GetStop();
	}
	currentTime = startTime;		
	
	animStatus = MUST_BE_REFRESHED;

}

void FbxParser::DisplayMetaData(FbxScene *pScene)
{
	FBXSDK_printf("\n\n--------------------------Meta Data------------------------------\n\n");
	FbxDocumentInfo *sceneInfo = pScene->GetSceneInfo();
	if (sceneInfo) {
		FBXSDK_printf("title: %s\n", sceneInfo->mTitle.Buffer());
		FBXSDK_printf("author: %s\n", sceneInfo->mAuthor.Buffer());
		FBXSDK_printf("comment: %s\n", sceneInfo->mComment.Buffer());
		FBXSDK_printf("keywords: %s\n", sceneInfo->mKeywords.Buffer());
		FBXSDK_printf("revision: %s\n", sceneInfo->mRevision.Buffer());
		FBXSDK_printf("subject: %s\n", sceneInfo->mSubject.Buffer());

		FbxThumbnail *thumbnail = sceneInfo->GetSceneThumbnail();
		if (thumbnail) {
			FBXSDK_printf("Thumbnail:\n");

			switch (thumbnail->GetDataFormat())
			{
			case FbxThumbnail::eRGB_24:
				FBXSDK_printf("RGB\n");
				break;
			case FbxThumbnail::eRGBA_32:
				FBXSDK_printf("RGBA\n");
				break;
			default:
				break;
			}

			switch (thumbnail->GetSize())
			{
			case FbxThumbnail::eNotSet:
				FBXSDK_printf("Size: no dimensions specified (%ld bytes)\n", thumbnail->GetSizeInBytes());
				break;
			case FbxThumbnail::e64x64:
				FBXSDK_printf("Size: 64 x 64 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
				break;
			case FbxThumbnail::e128x128:
				FBXSDK_printf("Size: 128 x 128 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
			default:
				break;
			}
		}
		else {
			FBXSDK_printf("error: get thumbnail failed.\n");
		}

	}
}

void FbxParser::DisplayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings)
{
	FBXSDK_printf("\n\n---------------------Global Light Settings---------------------\n\n");

	FbxColor color = pGlobalSettings->GetAmbientColor();
	FBXSDK_printf("ambient color: \n(red)%lf, (green)%lf, (blue)%lf\n\n", color.mRed, color.mGreen, color.mBlue);
}

void FbxParser::DisplayHierarchy(FbxNode *node, int depth, int currIndex, int parentIndex)
{		
	FbxString jointName = node->GetName();
	//Display the hierarchy
	FbxString nodeNameBuf("");
	for (int i = 0; i != depth; ++i) {
		nodeNameBuf += "   ";
	}
	nodeNameBuf += jointName;
	nodeNameBuf += "\n";
	FBXSDK_printf(nodeNameBuf.Buffer());
	
	//if the type of current node is Skeleton, then it's a joint
	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() &&
		node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
		Joint jointTmp;
		jointTmp.jointName = jointName;		
		jointTmp.parentIndex = parentIndex;		//parent index
		jointTmp.currentIndex = currIndex;	
		skeleton.joints.push_back(jointTmp);	
	}

	//Display the hierarchy recursively
	for (int i = 0; i != node->GetChildCount(); ++i) {
		DisplayHierarchy(node->GetChild(i), depth + 1, skeleton.joints.size(), currIndex);
	}
}

void FbxParser::DisplayHierarchy(FbxScene *pScene)
{
	FBXSDK_printf("\n\n---------------------------Hierarchy-------------------------------\n\n");

	FbxNode *rootNode = pScene->GetRootNode();
	int childCount = rootNode->GetChildCount();
	for (int i = 0; i != childCount; ++i) {
		DisplayHierarchy(rootNode->GetChild(i), 0, 0, -1);
	}
}

void FbxParser::DisplayContent(FbxScene *pScene)
{
	FBXSDK_printf("\n\n------------------------Node Content---------------------------\n\n");

	FbxNode *rootNode = pScene->GetRootNode();
	int childCount = rootNode->GetChildCount();
	if (rootNode) {
		for (int i = 0; i != childCount; ++i) {
			DisplayContent(rootNode->GetChild(i));
		}
	}
	else {
		FBXSDK_printf("null node!\n");
	}
	//DisplayTexture(pScene);
}

void FbxParser::DisplayContent(FbxNode *node)
{
	DisplayTexture(node);
	
	FbxNodeAttribute::EType attributeType;
	if (!node->GetNodeAttribute()) {
		FBXSDK_printf("null node attribute.\n");
	}
	else {
		attributeType = node->GetNodeAttribute()->GetAttributeType();
		//Display different informations according to the attribute type of node
		switch (attributeType)
		{
		case fbxsdk::FbxNodeAttribute::eMarker:
			DisplayMarker(node);	
			break;
		case fbxsdk::FbxNodeAttribute::eSkeleton:
		{
			FbxAMatrix dummyGlobalPostion;

			//DisplaySkeleton(node);
			break;
		}
		case fbxsdk::FbxNodeAttribute::eMesh:
			DisplayMesh(node);
			break;
		case fbxsdk::FbxNodeAttribute::eCamera:
			FBXSDK_printf("eCamera\n");
			break;
		default:
			FBXSDK_printf("default\n");
			break;
		}
	}
	int childCount = node->GetChildCount();
	for (int i = 0; i != childCount; ++i) {
		DisplayContent(node->GetChild(i));
	}
}

FbxAMatrix FbxParser::GetGlobalPosition(FbxNode *node, FbxTime currTime, FbxAMatrix *parentGlobalMatrix)
{
	return node->EvaluateGlobalTransform(currTime);
}

FbxAMatrix FbxParser::GetGeometry(FbxNode *node)
{
	const FbxVector4 IT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 IR = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 IS = node->GetGeometricScaling(FbxNode::eSourcePivot);
	return FbxAMatrix(IT, IR, IS);
}

void FbxParser::DisplayTexture(FbxNode *node)
{
	FbxMesh *nodeMesh = node->GetMesh();
	if (nodeMesh) {
		int materialCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
		for (int index = 0; index != materialCount; ++index) {
			FbxSurfaceMaterial *material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(index);
			if (material) {
				FBXSDK_printf("material name: %s\n", material->GetName());
				FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

				int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
				if (layeredTextureCount > 0) {
					for (int layerIndex = 0; layerIndex != layeredTextureCount; ++layerIndex) {
						FbxLayeredTexture *layeredTexture = prop.GetSrcObject<FbxLayeredTexture>(layerIndex);
						int count = layeredTexture->GetSrcObjectCount<FbxTexture>();
						for (int c = 0; c != count; ++c) {
							FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(layeredTexture->GetSrcObject<FbxFileTexture>(c));
							const char *textureName = fileTexture->GetFileName();
							FBXSDK_printf("texture name: %s\n", textureName);
							SetTextureFileName(FbxString(textureName));
						}
					}
				}
				else {
					int textureCount = prop.GetSrcObjectCount<FbxTexture>();
					for (int i = 0; i != textureCount; ++i) {
						FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(i));
						const char *textureName = fileTexture->GetFileName();
						FBXSDK_printf("texture name: %s\n", textureName);
						SetTextureFileName(FbxString(textureName));
					}
				}
			}
		}
	}
}

void FbxParser::DisplayTexture(FbxScene *pScene)
{
	const int textureCount = pScene->GetTextureCount();
	for (int textureIndex = 0; textureIndex != textureCount; ++textureIndex) {
		FbxTexture *pTexture = pScene->GetTexture(textureIndex);
		FbxFileTexture *pFileTexture = FbxCast<FbxFileTexture>(pTexture);

		if (pFileTexture && !pFileTexture->GetUserDataPtr()) {
			//try to load the texture from absolute path
			const FbxString fileName = pFileTexture->GetFileName();

		}
	}
}

void FbxParser::DisplayPose(FbxScene *pScene)
{
	FBXSDK_printf("\n\n---------------------------Pose-------------------------------\n\n");
	int      i, j, k, lPoseCount;
	FbxString  lName;

	lPoseCount = pScene->GetPoseCount();

	for (i = 0; i < lPoseCount; i++)
	{
		FbxPose* lPose = pScene->GetPose(i);

		lName = lPose->GetName();
		DisplayString("Pose Name: ", lName.Buffer());

		DisplayBool("    Is a bind pose: ", lPose->IsBindPose());

		DisplayInt("    Number of items in the pose: ", lPose->GetCount());

		DisplayString("pose: ", "");
		
		for (j = 0; j<lPose->GetCount(); j++)
		{
			lName = lPose->GetNodeName(j).GetCurrentName();
			//DisplayString("    Item name: ", lName.Buffer());
			FBXSDK_printf("Item %d: %s\n", j, lName);
			if (!lPose->IsBindPose())
			{
				// Rest pose can have local matrix
				DisplayBool("    Is local space matrix: ", lPose->IsLocalMatrix(j));
			}

			//DisplayString("    Matrix value: ", "");

			FbxString lMatrixValue;

			for (k = 0; k<4; k++)
			{
				FbxMatrix  lMatrix = lPose->GetMatrix(j);
				FbxVector4 lRow = lMatrix.GetRow(k);
				char        lRowValue[1024];

				//FBXSDK_sprintf(lRowValue, 1024, "%9.4f %9.4f %9.4f %9.4f\n", lRow[0], lRow[1], lRow[2], lRow[3]);
				lMatrixValue += FbxString("        ") + FbxString(lRowValue);
			}

			//DisplayString("", lMatrixValue.Buffer());
		}
	}

	lPoseCount = pScene->GetCharacterPoseCount();

	for (i = 0; i < lPoseCount; i++)
	{
		FbxCharacterPose* lPose = pScene->GetCharacterPose(i);
		FbxCharacter*     lCharacter = lPose->GetCharacter();

		if (!lCharacter) break;

		DisplayString("Character Pose Name: ", lCharacter->GetName());

		FbxCharacterLink lCharacterLink;
		FbxCharacter::ENodeId  lNodeId = FbxCharacter::eHips;

		while (lCharacter->GetCharacterLink(lNodeId, &lCharacterLink))
		{
			FbxAMatrix& lGlobalPosition = lCharacterLink.mNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);

			DisplayString("    Matrix value: ", "");

			FbxString lMatrixValue;

			for (k = 0; k < 4; k++)
			{
				FbxVector4 lRow = lGlobalPosition.GetRow(k);
				char        lRowValue[1024];

				//FBXSDK_sprintf(lRowValue, 1024, "%9.4f %9.4f %9.4f %9.4f\n", lRow[0], lRow[1], lRow[2], lRow[3]);
				lMatrixValue += FbxString("        ") + FbxString(lRowValue);
			}

			//DisplayString("", lMatrixValue.Buffer());

			lNodeId = FbxCharacter::ENodeId(int(lNodeId) + 1);
		}
	}

}

void FbxParser::DisplayMarker(FbxNode *node)
{
	FbxMarker *marker = (FbxMarker*)node->GetNodeAttribute();
	FbxString markerInfo("");
	markerInfo += "marker name: " + FbxString(node->GetName()) + "\n";
	FBXSDK_printf(markerInfo.Buffer());

	//type
	markerInfo = "marker type:\n";
	switch (marker->GetType())
	{
	case FbxMarker::eStandard:
		markerInfo += "standard\n";
		break;
	case FbxMarker::eOptical:
		markerInfo += "optical\n";
		break;
	case FbxMarker::eEffectorIK:
		markerInfo += "IK Effector\n";
		break;
	case FbxMarker::eEffectorFK:
		markerInfo += "FK Effector\n";
		break;
	default:
		break;
	}
	FBXSDK_printf(markerInfo.Buffer());

	//look
	markerInfo = "marker look:\n";
	switch (marker->Look.Get())
	{
	case FbxMarker::eCube:
		markerInfo += "Cube\n";
		break;
	case FbxMarker::eHardCross:
		markerInfo += "Hard Cross\n";
		break;
	case FbxMarker::eLightCross:
		markerInfo += "Light Cross\n";
		break;
	case FbxMarker::eSphere:
		markerInfo += "Sphere\n";
		break;
	default:
		break;
	}
	FBXSDK_printf(markerInfo.Buffer());

	//size
	markerInfo = "size: " + FbxString(marker->Size.Get());
	FBXSDK_printf(markerInfo.Buffer());

	//color
	FbxDouble3 c = marker->Color.Get();
	FbxColor color(c[0], c[1], c[2]);
	FBXSDK_printf("red: %lf,    green: %lf,    blue: %lf", color.mRed, color.mGreen, color.mBlue);

	//IKPivot
	FbxDouble3 pivot = marker->IKPivot.Get();
	FBXSDK_printf("IKPivot:    %lf, %lf, %lf", pivot.mData[0], pivot.mData[1], pivot.mData[2]);
}

void FbxParser::DisplayMesh(FbxNode *node)
{
	pMesh = (FbxMesh*)node->GetNodeAttribute();

	FBXSDK_printf("\n\n---------------Mesh---------------------\n\n");
	FbxString meshInfo("");
	meshInfo += "mesh name: " + FbxString(node->GetName()) + "\n";
	FBXSDK_printf(meshInfo.Buffer());

	//Meta Data
	int metaDataCount = ((FbxObject*)pMesh)->GetSrcObjectCount<FbxObjectMetaData>();
	if (metaDataCount > 0) {
		for (int i = 0; i != metaDataCount; ++i) {
			FbxObjectMetaData *metaData = ((FbxObject*)pMesh)->GetSrcObject<FbxObjectMetaData>(i);
			FBXSDK_printf("name: %s\n", metaData->GetName());
		}
	}
	else    FBXSDK_printf("get meta data failed.\n\n");

	//Control Points
	int controlPointsCount = pMesh->GetControlPointsCount();
	//controlPointsInfo.ctrlPoints = pMesh->GetControlPoints();

	FBXSDK_printf("\n\n---------------Control Points---------------------\n\n");
	FBXSDK_printf("Control Point Count %d\n", controlPointsCount);
	for (unsigned int i = 0; i != controlPointsCount; ++i) {
		ControlPointInfo currPoint;
		currPoint.ctrlPoint = pMesh->GetControlPointAt(i);
		controlPointsInfo[i] = currPoint;
	}


	FBXSDK_printf("UV layer count: %d\n", pMesh->GetUVLayerCount());		//return 1
	FBXSDK_printf("UV texture count: %d\n", pMesh->GetTextureUVCount());	//return 1970 in run.fbx
	//FbxGeometryElementUV *uv = pMesh->GetElementUV(0);
	//if (!uv)
	//	FBXSDK_printf("name: %s\n", uv->GetName());	//return UV_channel_1

	FBXSDK_printf("\n\n");

	ProcessJointsAndAnimations(node);
	//Polygon Counts
	polygonCount = pMesh->GetPolygonCount();
	int vertexCounter = 0;

	FBXSDK_printf("\n\n---------------Polygon Points---------------------\n\n");
	FBXSDK_printf("polygon points count %d\n", polygonCount);
	polygonPoints.reserve(polygonCount * 4);
	normals.reserve(polygonCount * 4);
	uvs.reserve(polygonCount * 4);
	GetTextureUV(pMesh, uvs);
	for (int i = 0; i != polygonCount; ++i) {
		int polygonSize = pMesh->GetPolygonSize(i);
		//	normals.resize(polygonSize);

		for (int j = 0; j != polygonSize; ++j) {
			int vertexIndex = pMesh->GetPolygonVertex(i, j);
			FbxVector4 vec = controlPointsInfo[vertexIndex].ctrlPoint;
			GetNormal(pMesh, vertexIndex, vertexCounter, normals);

			polygonPoints.push_back(vec);
			++vertexCounter;
		}
	}
}

void FbxParser::GetNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, vector<FbxVector4> &normals)
{
	if (mesh->GetElementNormalCount() < 1) {
		return;
	}

	FbxVector4 normal;
	FbxGeometryElementNormal *vertexNormal = mesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			normal = vertexNormal->GetDirectArray().GetAt(vertexIndex).mData;
			break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(vertexIndex);
			normal = vertexNormal->GetDirectArray().GetAt(index).mData;
			break;
		}
		default:
			throw std::exception("Invalid Reference");
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
			normal = vertexNormal->GetDirectArray().GetAt(vertexCounter).mData;
			break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(vertexCounter);
			normal = vertexNormal->GetDirectArray().GetAt(index).mData;
			break;
		}
		default:
			throw std::exception("Invalid Reference");
		}
	default:
		break;
	}
	normals.push_back(normal);
}

void FbxParser::GetTextureUV(FbxMesh *mesh, vector<FbxVector2> &uvs)
{
	//get all UV set names
	FbxStringList lUVSetNameList;
	pMesh->GetUVSetNames(lUVSetNameList);

	//iterating over all uv sets
	for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
	{
		//get lUVSetIndex-th uv set
		const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
		const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

		if (!lUVElement)
			continue;

		// only support mapping mode eByPolygonVertex and eByControlPoint
		if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return;

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		const int lPolyCount = pMesh->GetPolygonCount();

		if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					FbxVector2 lUVValue;
					//get the index of the current vertex in control points array
					int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

					uvs.push_back(lUVValue);
				}
			}
		}
		else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					if (lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

						uvs.push_back(lUVValue);

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}

void FbxParser::ProcessJointsAndAnimations(FbxNode *node)
{
	FbxMesh *currMesh = node->GetMesh();
	unsigned int numOfDeformers = currMesh->GetDeformerCount();

	//Get transform matrix
	FbxAMatrix geometryTransform = GetGeometryTransformation(node);
	
	//A deformer is a FBX thing, which contains some clusters
	//A cluster contains a link, which is basically a joint
	//Normally, there is only one deformer in a mesh
	for (unsigned int deformerIndex = 0; deformerIndex != numOfDeformers; ++deformerIndex) {
		//There are many types of deformers in FBX
		//We are using only skins, so we see if there is a skin
		FbxSkin *currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (!currSkin) {
			continue;
		}

		unsigned int numOfClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex != numOfClusters; ++clusterIndex) {
			FbxCluster *currCluster = currSkin->GetCluster(clusterIndex);
			FbxString currJointName = currCluster->GetLink()->GetName();
			int currJointIndex = FindJointIndexByName(currJointName);
			if (currJointIndex == -1) {
				FBXSDK_printf("error: can't find the joint: %s\n\n", currJointName);
				continue;
			}

			//FbxAMatrix transformMatrix;
			//FbxAMatrix transformLinkMatrix;
			//FbxAMatrix globalBindPoseInverseMatrix;
			//
			//currCluster->GetTransformMatrix(transformMatrix);
			//currCluster->GetTransformLinkMatrix(transformLinkMatrix);
			//globalBindPoseInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			////Update the information of the joints
			//skeleton.joints[currJointIndex].node = currCluster->GetLink();
			//skeleton.joints[currJointIndex].globalBindPoseInverse = globalBindPoseInverseMatrix;


			FbxAMatrix localMatrix = currCluster->GetLink()->EvaluateLocalTransform();

			skeleton.joints[currJointIndex].node = currCluster->GetLink();	//获取当前的关节
			skeleton.joints[currJointIndex].localMatrix = localMatrix;			
			
			unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
			for (unsigned int i = 0; i != numOfIndices; ++i) {
				IndexWeightPair weightPair;
				weightPair.index = currJointIndex;		
				weightPair.weight = currCluster->GetControlPointWeights()[i];		//weight to control point of current joint
				//add index-weight pair into ControlPointInfo Struct
				controlPointsInfo[currCluster->GetControlPointIndices()[i]].weightPairs.push_back(weightPair);
			}

			//Get Animation information
			//Now only support one take
			FbxAnimStack *currAnimStack = pScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			animationName = animStackName;
			FbxTakeInfo *takeInfo = pScene->GetTakeInfo(animStackName);
			FbxTime start = currAnimStack->GetLocalTimeSpan().GetStart();
			FbxTime end = currAnimStack->GetLocalTimeSpan().GetStop();

			animationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
			KeyFrame **currAnim = &skeleton.joints[currJointIndex].animation;

			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i != end.GetFrameCount(FbxTime::eFrames24); ++i) {
				FbxTime currTime;
				currTime.SetFrame(i, FbxTime::eFrames24);
				*currAnim = new KeyFrame();	
				(*currAnim)->frameNum = i;
				FbxAMatrix currTransformOffset = node->EvaluateGlobalTransform(currTime) * geometryTransform;
				(*currAnim)->globalTransform = currTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				currAnim = &((*currAnim)->next);
				
			}
			
		}
	}
	
	DebugSumOfWeights();		//should be 1.0
}

FbxMatrix FbxParser::CalcGlobalMatrix(int index)
{
	FbxMatrix globalMatrix;
	globalMatrix.SetIdentity();
	if (index == -1)
		return globalMatrix;
	else {
		Joint currJoint = skeleton.joints[index];
		return CalcGlobalMatrix(currJoint.parentIndex) * currJoint.globalMatrix;
	}
}

FbxAMatrix FbxParser::GetGeometryTransformation(FbxNode *node) 
{
	if (!node) {
		throw std::exception("Null for mesh geometry\n\n");
	}

	const FbxVector4 IT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 IR = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 IS = node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(IT, IR, IS);

}

int FbxParser::FindJointIndexByName(const FbxString& jointName)
{
	for (int index = 0; index != skeleton.joints.size(); ++index) {
		if (skeleton.joints[index].jointName == jointName)
			return index;
	}
	return -1;
}

void FbxParser::DisplaySkeleton(FbxNode *node)
{
	FBXSDK_printf("\n\n-----------------------Skeleton---------------------------\n\n");
	FbxSkeleton *skeleton = (FbxSkeleton*)node->GetNodeAttribute();

	FbxString skeletonInfo("");
	skeletonInfo += "skeleton name: " + FbxString(node->GetName()) + "\n";
	FBXSDK_printf(skeletonInfo.Buffer());

	int srcObjCount = ((FbxObject*)skeleton)->GetSrcObjectCount<FbxObjectMetaData>();
	
	for (int i = 0; i != srcObjCount; ++i) {
		FbxObjectMetaData *metaData = ((FbxObject*)skeleton)->GetSrcObject<FbxObjectMetaData>(i);
		FBXSDK_printf("name: %s\n", metaData->GetName());
	}

	const char* skeletonTypes[] = { "Root", "Limb", "Limb Node", "Effector" };
	FBXSDK_printf("skeleton type: %s\n", skeletonTypes[skeleton->GetSkeletonType()]);

	if (skeleton->GetSkeletonType() == FbxSkeleton::eLimb) {
		FBXSDK_printf("limb type: %lf\n", skeleton->LimbLength.Get());
	}
	else if (skeleton->GetSkeletonType() == FbxSkeleton::eLimbNode) {
		FBXSDK_printf("limb node size: %lf\n", skeleton->Size.Get());
	}
	else if (skeleton->GetSkeletonType() == FbxSkeleton::eRoot) {
		FBXSDK_printf("limb root size: %lf\n", skeleton->Size.Get());
	}
	else {
		FBXSDK_printf("undefined skeleton type: %s\n", skeleton->GetSkeletonType());
	}

	FbxColor color = skeleton->GetLimbNodeColor();
	FBXSDK_printf("limb node color:	red: %lf, green: %lf, blue: %lf\n", color[0], color[1], color[2]);
}

void FbxParser::CovertFormat()
{
	const char* lFileTypes[] =
	{
		"_FBX_Binary.fbx", "FBX binary(*.fbx)",
		"_FBX_ascii.fbx", "FBX ascii(*.fbx)",
		"_FBX_encrypted.fbx", "FBX encrypted(*.fbx)",
		"_FBX_6.0_binary.fbx", "FBX 6.0 binary(*.fbx)",
		"_FBX_6.0_encrpyted.fbx", "FBX 6.0 encrpyted(*.fbx)",
		"_FBX_6.0_ascii.fbx", "FBX 6.0 ascii(*.fbx)"
	};
	const size_t lFileNameLength = strlen(fbxFile.Buffer());
	char* lNewFileName = new char[lFileNameLength + 64];
	FBXSDK_strcpy(lNewFileName, lFileNameLength + 64, fbxFile.Buffer());

	const size_t lFileTypeCount = sizeof(lFileTypes) / sizeof(lFileTypes[0]) / 2;

	for (size_t i = 0; i<lFileTypeCount; ++i)
	{
		// Retrieve the writer ID according to the description of file format.
		int lFormat = pManager->GetIOPluginRegistry()->FindWriterIDByDescription(lFileTypes[i * 2 + 1]);

		// Construct the output file name.
		FBXSDK_strcpy(lNewFileName + lFileNameLength, 60, lFileTypes[i * 2]);

		// Create an exporter.
		FbxExporter* lExporter = FbxExporter::Create(pManager, "");

		// Initialize the exporter.
		bool lResult = lExporter->Initialize(lNewFileName, lFormat, pManager->GetIOSettings());
		if (!lResult)
		{
			FBXSDK_printf("%s:\tCall to FbxExporter::Initialize() failed.\n", lFileTypes[i * 2 + 1]);
			FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		}
		else
		{
			// Export the scene.
			lResult = lExporter->Export(pScene);
			if (!lResult)
			{
				FBXSDK_printf("Call to FbxExporter::Export() failed.\n");
			}
		}

		// Destroy the exporter.
		lExporter->Destroy();
	}
	delete[] lNewFileName;
}

void FbxParser::DebugSumOfWeights()
{
	for (auto it = controlPointsInfo.begin(); it != controlPointsInfo.end(); ++it) {
		vector<IndexWeightPair> weightPairs = it->second.weightPairs;
		double sumOfWeights = 0.0;
	//	FBXSDK_printf("\joint id		weight\n");
		for (auto weightPair : weightPairs) {
		//	FBXSDK_printf("%d		%lf\n", weightPair.index, weightPair.weight);
			sumOfWeights += weightPair.weight;
		}
		//FBXSDK_printf("the sum of weights is: %lf\n", sumOfWeights);
		assert((sumOfWeights - 1.0) < 10e-5);	//sum of weight is not equal to 1.0
	}
}