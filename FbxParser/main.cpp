//#include <fbxsdk.h>
//#include <gl\glut.h>
//#include <vector>
//#include <assert.h>

#include "ModelReconstruct.h"

//#ifdef IOS_REF
//#undef  IOS_REF
//#define IOS_REF (*(pManager->GetIOSettings()))
//#endif
//
//void initSdkObjects(FbxManager *&pManager, FbxScene *&pScene);
//bool loadScene(FbxManager *pManager, FbxDocument *pScene, FbxString fbxFilePath);
//
//void displayMetaData(FbxScene *pScene);
//void displayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings);
//void displayHierarchy(FbxScene *pScene);
//void displayContent(FbxScene *pScene);
//void displayMarker(FbxNode *node);
//void displaySkeleton(FbxNode *node);
//void displayMesh(FbxNode *node);
//void display3DVector(const char *prefix, FbxVector4 pValue);
//void caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal);
//void displayTexture(FbxNode *node);
//
////openGL
//void initOpenGL();
//void displayModel();
//void SpecialKeys(int key, int x, int y);
//void MouseFunc(int button, int action, int x, int y);
//void changeSize(GLsizei w, GLsizei h);
//
//vector<FbxVector4>polygonVertex;
//FbxVector4 *controlPoints;
//FbxMesh *mesh;
//
////rotate factor
//static GLfloat xRot = 0.0f;
//static GLfloat yRot = 0.0f;
//
////scale factor
//static GLfloat xScale = 1.0f;
//static GLfloat yScale = 1.0f;
//static GLfloat zScale = 1.0f;


int main(int argc, char **argv)
{
	FbxParser *parser = new FbxParser(FbxString("cube"));
	bool loadResult = parser->loadScene();
	if (loadResult) {
		parser->displayGlobalLightSettings(&parser->getFbxScene()->GetGlobalSettings());
		parser->displayHierarchy(parser->getFbxScene());
		parser->displayContent(parser->getFbxScene());
	}
	else {
		FBXSDK_printf("error: load scene failed.\n\n");
		exit(1);
	}

	ModelReconstruct *model = new ModelReconstruct(parser, argc, argv);
	model->displayModel();
	model->loop();
	system("pause");

//	glutInit(&argc, argv);
//	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
//	glutInitWindowPosition(100, 100);
//	glutInitWindowSize(800, 800);
//	glutCreateWindow("Test Input Models");
//
//	initOpenGL();
//	FbxManager *pManager = nullptr;
//	FbxScene *pScene = nullptr;
//	bool loadSceneResult = false;
//	//Prepare the FBX SDK
//	initSdkObjects(pManager, pScene);
//
//	FbxString fbxFilePath("G:\\FBX_SDK\\FBX_MODEL\\master\\at.FBX");	//fbx path
//	FBXSDK_printf("FBX file path: %s\n", fbxFilePath.Buffer());
//
//	loadSceneResult = loadScene(pManager, pScene, fbxFilePath);
//
//	if (loadSceneResult == false) {
//		FBXSDK_printf("error: load scene failed.\n\n");
//		exit(1);
//	}
//	else {
//		/*FBXSDK_printf("\n\n--------------------------Meta Data------------------------------\n\n");
//		displayMetaData(pScene);*/
//
//		FBXSDK_printf("\n\n---------------------Global Light Settings---------------------\n\n");
//		displayGlobalLightSettings(&pScene->GetGlobalSettings());
//
//		FBXSDK_printf("\n\n---------------------------Hierarchy-------------------------------\n\n");
//		displayHierarchy(pScene);
//
//		/*for (int i = 0; i != pManager->GetDataTypeCount(); ++i) {
//		FbxDataType dataType = pManager->GetDataType(i);
//		FBXSDK_printf("%s\n",dataType.GetName());
//		}
//		*/
//		FBXSDK_printf("\n\n------------------------Node Content---------------------------\n\n");
//		displayContent(pScene);
//
//		glutDisplayFunc(displayModel);
//		/*glutReshapeFunc(changeSize);
//		glutSpecialFunc(SpecialKeys);
//		glutMouseFunc(MouseFunc);*/
//	}
//	glutPostRedisplay();
//	glutMainLoop();
//}
//
//
//void initSdkObjects(FbxManager *&pManager, FbxScene *&pScene)
//{
//	//create the FBX manager which is the object allocator for almost all the classes in the SDK
//	pManager = FbxManager::Create();
//	if (!pManager) {
//		FBXSDK_printf("error: unable to create FBX manager!\n");
//		exit(1);
//	}
//	else
//		FBXSDK_printf("Autodesk FBX SDK version:%s\n", pManager->GetVersion());
//
//	//create an IOSettings object. This object holds all import/export settings
//	FbxIOSettings *ios = FbxIOSettings::Create(pManager, IOSROOT);
//	pManager->SetIOSettings(ios);
//
//	//load plugins from the executable directory
//	FbxString path = FbxGetApplicationDirectory();
//	FBXSDK_printf("application path: %s\n", path.Buffer());
//	pManager->LoadPluginsDirectory(path.Buffer());
//
//	//create an FBX scene. This object holds most objects imported/exported from/to files
//	pScene = FbxScene::Create(pManager, "my scene");
//	if (!pScene) {
//		FBXSDK_printf("error: unable to create FBX scene\n");
//		exit(1);
//	}
//}
//
//bool loadScene(FbxManager *pManager, FbxDocument *pScene, FbxString fbxFile)
//{
//	int animStackCount = 0;
//	bool status;
//	//create an importer
//	FbxImporter *importer = FbxImporter::Create(pManager, "");
//
//	//initialize the importer by providing a file name
//	const bool imorterStatus = importer->Initialize(fbxFile, -1, pManager->GetIOSettings());
//	if (!imorterStatus) {
//		FBXSDK_printf("error: initialize importer failed\n");
//		return false;
//	}
//
//	if (importer->IsFBX()) {
//		FBXSDK_printf("\n\n-------Animation Stack Information-------\n\n");
//
//		animStackCount = importer->GetAnimStackCount();
//		FBXSDK_printf("number of animation stacks: %d\n", animStackCount);
//		FBXSDK_printf("current animation stack: %s\n", importer->GetActiveAnimStackName().Buffer());
//
//		for (int i = 0; i != animStackCount; ++i) {
//			FbxTakeInfo *takeInfo = importer->GetTakeInfo(i);
//
//			FBXSDK_printf("animation stack %d\n", i);
//			FBXSDK_printf("	name: %s\n", takeInfo->mName.Buffer());
//			FBXSDK_printf("	description: %s\n", takeInfo->mDescription.Buffer());
//			FBXSDK_printf("	import name: %s\n", takeInfo->mImportName.Buffer());
//			FBXSDK_printf("	import state: %s\n\n", takeInfo->mSelect ? "true" : "false");
//
//			// Set the import states. By default, the import states are always set to 
//			// true. The code below shows how to change these states.
//			IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
//			IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
//			IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
//			IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
//			IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
//			IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
//			IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
//		}
//
//		//import the scene
//		status = importer->Import(pScene);
//		char password[1024];
//		if (status == false && importer->GetStatus().GetCode() == FbxStatus::ePasswordError)
//		{
//			FBXSDK_printf("Please enter password: ");
//
//			password[0] = '\0';
//
//			FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
//				scanf("%s", password);
//			FBXSDK_CRT_SECURE_NO_WARNING_END
//
//				FbxString pwd(password);
//
//			IOS_REF.SetStringProp(IMP_FBX_PASSWORD, pwd);
//			IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
//
//			status = importer->Import(pScene);
//
//			if (status == false && importer->GetStatus().GetCode() == FbxStatus::ePasswordError)
//			{
//				FBXSDK_printf("\nPassword is wrong, import aborted.\n");
//			}
//		}
//	}
//
//	//destroy the importer
//	importer->Destroy();
//
//	return status;
//}
//
//void displayMetaData(FbxScene *pScene)
//{
//	FbxDocumentInfo *sceneInfo = pScene->GetSceneInfo();
//	if (sceneInfo) {
//		FBXSDK_printf("title: %s\n", sceneInfo->mTitle.Buffer());
//		FBXSDK_printf("author: %s\n", sceneInfo->mAuthor.Buffer());
//		FBXSDK_printf("comment: %s\n", sceneInfo->mComment.Buffer());
//		FBXSDK_printf("keywords: %s\n", sceneInfo->mKeywords.Buffer());
//		FBXSDK_printf("revision: %s\n", sceneInfo->mRevision.Buffer());
//		FBXSDK_printf("subject: %s\n", sceneInfo->mSubject.Buffer());
//
//		FbxThumbnail *thumbnail = sceneInfo->GetSceneThumbnail();
//		if (thumbnail) {
//			FBXSDK_printf("Thumbnail:\n");
//
//			switch (thumbnail->GetDataFormat())
//			{
//			case FbxThumbnail::eRGB_24:
//				FBXSDK_printf("RGB\n");
//				break;
//			case FbxThumbnail::eRGBA_32:
//				FBXSDK_printf("RGBA\n");
//				break;
//			default:
//				break;
//			}
//
//			switch (thumbnail->GetSize())
//			{
//			case FbxThumbnail::eNotSet:
//				FBXSDK_printf("Size: no dimensions specified (%ld bytes)\n", thumbnail->GetSizeInBytes());
//				break;
//			case FbxThumbnail::e64x64:
//				FBXSDK_printf("Size: 64 x 64 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
//				break;
//			case FbxThumbnail::e128x128:
//				FBXSDK_printf("Size: 128 x 128 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
//			default:
//				break;
//			}
//		}
//		else {
//			FBXSDK_printf("error: get thumbnail failed.\n");
//		}
//
//	}
//
//}
//void displayGlobalLightSettings(FbxGlobalSettings *pGlobalSettings)
//{
//	FbxColor color = pGlobalSettings->GetAmbientColor();
//	FBXSDK_printf("ambient color: \n(red)%lf, (green)%lf, (blue)%lf\n\n", color.mRed, color.mGreen, color.mBlue);
//}
//
//void displayHierarchy(FbxNode *node, int depth)
//{
//	FbxString nodeName("");
//	for (int i = 0; i != depth; ++i) {
//		nodeName += "   ";
//	}
//	nodeName += node->GetName();
//	nodeName += "\n";
//
//	FBXSDK_printf(nodeName.Buffer());
//
//	for (int i = 0; i != node->GetChildCount(); ++i) {
//		displayHierarchy(node->GetChild(i), depth + 1);
//	}
//
//
//}
//
//void displayHierarchy(FbxScene *pScene)
//{
//	FbxNode *node = pScene->GetRootNode();
//	for (int i = 0; i != node->GetChildCount(); ++i) {
//		displayHierarchy(node->GetChild(i), 0);
//	}
//}
//
//void displayContent(FbxNode *node)
//{
//	displayTexture(node);
//
//	FbxNodeAttribute::EType attributeType;
//	if (!node->GetNodeAttribute()) {
//		FBXSDK_printf("null node attribute.\n");
//	}
//	else {
//		attributeType = node->GetNodeAttribute()->GetAttributeType();
//
//		switch (attributeType)
//		{
//		case fbxsdk::FbxNodeAttribute::eMarker:
//			displayMarker(node);
//			break;
//		case fbxsdk::FbxNodeAttribute::eSkeleton:
//			displaySkeleton(node);
//			break;
//		case fbxsdk::FbxNodeAttribute::eMesh:
//			displayMesh(node);
//			break;
//		case fbxsdk::FbxNodeAttribute::eCamera:
//			FBXSDK_printf("eCamera\n");
//			break;
//		default:
//			FBXSDK_printf("default\n");
//			break;
//		}
//	}
//
//}
//
//void displayContent(FbxScene *pScene)
//{
//	FbxNode *node = pScene->GetRootNode();
//	if (node) {
//		for (int i = 0; i != node->GetChildCount(); ++i) {
//			displayContent(node->GetChild(i));
//		}
//	}
//	else {
//		FBXSDK_printf("null node!\n");
//	}
//}
//
//void displayMarker(FbxNode *node)
//{
//	FbxMarker *marker = (FbxMarker*)node->GetNodeAttribute();
//	FbxString markerInfo("");
//	markerInfo += "marker name: " + FbxString(node->GetName()) + "\n";
//	FBXSDK_printf(markerInfo.Buffer());
//
//	//type
//	markerInfo = "marker type:\n";
//	switch (marker->GetType())
//	{
//	case FbxMarker::eStandard:
//		markerInfo += "standard\n";
//		break;
//	case FbxMarker::eOptical:
//		markerInfo += "optical\n";
//		break;
//	case FbxMarker::eEffectorIK:
//		markerInfo += "IK Effector\n";
//		break;
//	case FbxMarker::eEffectorFK:
//		markerInfo += "FK Effector\n";
//		break;
//	default:
//		break;
//	}
//	FBXSDK_printf(markerInfo.Buffer());
//
//	//look
//	markerInfo = "marker look:\n";
//	switch (marker->Look.Get())
//	{
//	case FbxMarker::eCube:
//		markerInfo += "Cube\n";
//		break;
//	case FbxMarker::eHardCross:
//		markerInfo += "Hard Cross\n";
//		break;
//	case FbxMarker::eLightCross:
//		markerInfo += "Light Cross\n";
//		break;
//	case FbxMarker::eSphere:
//		markerInfo += "Sphere\n";
//		break;
//	default:
//		break;
//	}
//	FBXSDK_printf(markerInfo.Buffer());
//
//	//size
//	markerInfo = "size: " + FbxString(marker->Size.Get());
//	FBXSDK_printf(markerInfo.Buffer());
//
//	//color
//	FbxDouble3 c = marker->Color.Get();
//	FbxColor color(c[0], c[1], c[2]);
//	FBXSDK_printf("red: %lf,    green: %lf,    blue: %lf", color.mRed, color.mGreen, color.mBlue);
//
//	//IKPivot
//	FbxDouble3 pivot = marker->IKPivot.Get();
//	FBXSDK_printf("IKPivot:    %lf, %lf, %lf", pivot.mData[0], pivot.mData[1], pivot.mData[2]);
//}
//
//void displaySkeleton(FbxNode *node)
//{
//	FBXSDK_printf("\n\n");
//	FbxSkeleton *skeleton = (FbxSkeleton*)node->GetNodeAttribute();
//
//	FbxString skeletonInfo("");
//	skeletonInfo += "skeleton name: " + FbxString(node->GetName()) + "\n";
//	FBXSDK_printf(skeletonInfo.Buffer());
//
//	int srcObjCount = ((FbxObject*)skeleton)->GetSrcObjectCount<FbxObjectMetaData>();
//	if (srcObjCount > 0)
//		FBXSDK_printf("meta data connections\n");
//	else
//		FBXSDK_printf("meta data connections failed\n");
//
//	for (int i = 0; i != srcObjCount; ++i) {
//		FbxObjectMetaData *metaData = ((FbxObject*)skeleton)->GetSrcObject<FbxObjectMetaData>(i);
//		FBXSDK_printf("name: %s\n", metaData->GetName());
//	}
//
//	const char* skeletonTypes[] = { "Root", "Limb", "Limb Node", "Effector" };
//	FBXSDK_printf("skeleton type: %s\n", skeletonTypes[skeleton->GetSkeletonType()]);
//
//	if (skeleton->GetSkeletonType() == FbxSkeleton::eLimb) {
//		FBXSDK_printf("limb type: %lf\n", skeleton->LimbLength.Get());
//	}
//	else if (skeleton->GetSkeletonType() == FbxSkeleton::eLimbNode) {
//		FBXSDK_printf("limb node size: %lf\n", skeleton->Size.Get());
//	}
//	else if (skeleton->GetSkeletonType() == FbxSkeleton::eRoot) {
//		FBXSDK_printf("limb root size: %lf\n", skeleton->Size.Get());
//	}
//	else {
//		FBXSDK_printf("undefined skeleton type: %s\n", skeleton->GetSkeletonType());
//	}
//
//	FbxColor color = skeleton->GetLimbNodeColor();
//	FBXSDK_printf("color:	red: %lf, green: %lf, blue: %lf\n", color[0], color[1], color[2]);
//}
//
//void displayTexture(FbxNode *node)
//{
//	FbxMesh *nodeMesh = node->GetMesh();
//	if (nodeMesh) {
//		int materialCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
//		for (int index = 0; index != materialCount; ++index) {
//			FbxSurfaceMaterial *material = (FbxSurfaceMaterial*)node->GetSrcObject<FbxSurfaceMaterial>(index);
//			if (material) {
//				FBXSDK_printf("material name: %s\n", material->GetName());
//				FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
//
//				int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
//				if (layeredTextureCount > 0) {
//					FBXSDK_printf("layeredTextureCount\n");
//					for (int layerIndex = 0; layerIndex != layeredTextureCount; ++layerIndex) {
//						FbxLayeredTexture *layeredTexture = prop.GetSrcObject<FbxLayeredTexture>(layerIndex);
//						int count = layeredTexture->GetSrcObjectCount<FbxTexture>();
//						for (int c = 0; c != count; ++c) {
//							FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(layeredTexture->GetSrcObject<FbxFileTexture>(c));
//							const char *textureName = fileTexture->GetFileName();
//							FBXSDK_printf("texture name: %s\n", textureName);
//						}
//					}
//				}
//				else {
//					FBXSDK_printf("textureName\n");
//					int textureCount = prop.GetSrcObjectCount<FbxTexture>();
//					for (int i = 0; i != textureCount; ++i) {
//						FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(i));
//						const char *textureName = fileTexture->GetFileName();
//						FBXSDK_printf("texture name: %s\n", textureName);
//					}
//				}
//			}
//		}
//	}
//}
//void displayMesh(FbxNode *node)
//{
//	mesh = (FbxMesh*)node->GetNodeAttribute();
//
//	FbxString meshInfo("");
//	meshInfo += "mesh name: " + FbxString(node->GetName()) + "\n";
//	FBXSDK_printf(meshInfo.Buffer());
//
//	//Meta Data
//	int metaDataCount = ((FbxObject*)mesh)->GetSrcObjectCount<FbxObjectMetaData>();
//	if (metaDataCount > 0) {
//		for (int i = 0; i != metaDataCount; ++i) {
//			FbxObjectMetaData *metaData = ((FbxObject*)mesh)->GetSrcObject<FbxObjectMetaData>(i);
//			FBXSDK_printf("name: %s\n", metaData->GetName());
//		}
//	}
//	else    FBXSDK_printf("get meta data failed.\n\n");
//
//	//Control Points
//	int controlPointsCount = mesh->GetControlPointsCount();
//	controlPoints = mesh->GetControlPoints();
//
//	FBXSDK_printf("\n\n---------------Control Points---------------------\n\n");
//	for (int i = 0; i != controlPointsCount; ++i) {
//		//FBXSDK_printf("    Control Point %d", i + 1);
//		//display3DVector("        Coordinates: ", controlPoints[i]);
//
//		for (int j = 0; j != mesh->GetElementNormalCount(); ++j) {
//			FbxGeometryElementNormal *geoEleNormal = mesh->GetElementNormal(j);
//			switch (geoEleNormal->GetMappingMode())
//			{
//			case FbxGeometryElement::eByControlPoint:
//				switch (geoEleNormal->GetReferenceMode())
//				{
//				case FbxGeometryElement::eDirect:
//					FBXSDK_printf("eDirect\n\n");
//					break;
//				case FbxGeometryElement::eIndexToDirect:
//					FBXSDK_printf("eIndexToDirect\n\n");
//					break;
//				default:
//					FBXSDK_printf("default element\n\n");
//					break;
//				}
//				break;
//			case FbxGeometryElement::eByPolygonVertex:
//				//FBXSDK_printf("eByPolygonVertex\n");
//				switch (geoEleNormal->GetReferenceMode())
//				{
//				case FbxGeometryElement::eDirect:
//					/*FBXSDK_printf("eDirect\n");
//					display3DVector("direct array:\n", geoEleNormal->GetDirectArray().GetAt(i));
//					FBXSDK_printf("\n");*/
//					break;
//				case FbxGeometryElement::eIndexToDirect:
//					FBXSDK_printf("eIndexToDirect\n\n");
//					break;
//				default:
//					break;
//				}
//				break;
//			default:
//				break;
//			}
//			/*if (geoEleNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
//			FBXSDK_printf("eByControlPoint\n");
//			char header[100];
//			FBXSDK_sprintf(header, 100, "        normal vector:\n");
//			if (geoEleNormal->GetReferenceMode() == FbxGeometryElement::eDirect) {
//			display3DVector(header, geoEleNormal->GetDirectArray().GetAt(i));
//			}
//			}
//			else if (geoEleNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
//			FBXSDK_printf("eByPolygonVertex\n");
//
//
//			}*/
//		}
//	}
//	//glutDisplayFunc(displayModel);
//	FBXSDK_printf("\n\n");
//}
//
//void display3DVector(const char *prefix, FbxVector4 pValue)
//{
//	FbxString vec("");
//	FbxString val1 = (float)pValue[0];
//	FbxString val2 = (float)pValue[1];
//	FbxString val3 = (float)pValue[2];
//	//FbxString val4 = (float)pValue[3];
//
//	val1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : val1.Buffer();
//	val1 = pValue[0] >= HUGE_VAL ? "INFINITY" : val1.Buffer();
//	val2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : val2.Buffer();
//	val2 = pValue[1] >= HUGE_VAL ? "INFINITY" : val2.Buffer();
//	val3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : val3.Buffer();
//	val3 = pValue[2] >= HUGE_VAL ? "INFINITY" : val3.Buffer();
//	/*val4 = pValue[3] <= -HUGE_VAL ? "-INFINITY" : val4.Buffer();
//	val4 = pValue[3] >= HUGE_VAL ? "INFINITY" : val4.Buffer();
//	*/
//	vec = prefix;
//	vec += val1;
//	vec += ", ";
//	vec += val2;
//	vec += ", ";
//	vec += val3;
//	/*vec += ", ";
//	vec += val4;*/
//	vec += "\n";
//	FBXSDK_printf(vec);
//}
//
//void caclNormal(FbxMesh *mesh, int vertexIndex, int vertexCounter, int polygonSize, FbxVector4 &normal)
//{
//	if (mesh->GetElementNormalCount() < 1) {
//		throw std::exception("invalid normal number\n");
//	}
//	FbxGeometryElementNormal *vertexNormal = mesh->GetElementNormal(0);
//	switch (vertexNormal->GetMappingMode())
//	{
//	case FbxGeometryElement::eByControlPoint:
//		switch (vertexNormal->GetReferenceMode())
//		{
//		case FbxGeometryElement::eDirect:
//			normal = vertexNormal->GetDirectArray().GetAt(vertexCounter).mData;
//			break;
//		case FbxGeometryElement::eIndexToDirect:
//		{
//			int index = vertexNormal->GetIndexArray().GetAt(vertexIndex);
//			normal = vertexNormal->GetDirectArray().GetAt(index).mData;
//			break;
//		}
//		default:
//			throw std::exception("Invalid Reference");
//		}
//		break;
//	case FbxGeometryElement::eByPolygonVertex:
//		switch (vertexNormal->GetReferenceMode())
//		{
//		case FbxGeometryElement::eDirect:
//			normal = vertexNormal->GetDirectArray().GetAt(vertexCounter).mData;
//			/*if (polygonSize == 3) {
//			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
//			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
//			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
//			}
//			else if (polygonSize == 4) {
//			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
//			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
//			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
//			normal.mData[3] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[3]);
//			}*/
//			break;
//		case FbxGeometryElement::eIndexToDirect:
//		{
//			int index = vertexNormal->GetIndexArray().GetAt(vertexCounter);
//			normal = vertexNormal->GetDirectArray().GetAt(index).mData;
//			/*if (polygonSize == 3) {
//			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
//			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
//			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
//			}
//			else if (polygonSize == 4) {
//			normal.mData[0] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
//			normal.mData[1] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
//			normal.mData[2] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
//			normal.mData[3] = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[3]);
//			}*/
//			break;
//		}
//		default:
//			throw std::exception("Invalid Reference");
//		}
//	default:
//		break;
//	}
//}
//
//void initOpenGL()
//{
//	GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
//	GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
//	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//	GLfloat mat_shininess[] = { 50.0 };
//
//	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
//	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
//	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
//	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
//
//	GLfloat light0_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
//	GLfloat light0_position[] = { 1.0, 1.0, 1.0, 0.0 };
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
//	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
//
//	GLfloat light1_ambient[] = { 1.0, 0.2, 0.2, 1.0 };
//	GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
//	GLfloat light1_specular[] = { 1.0, 0.6, 0.6, 1.0 };
//	GLfloat light1_position[] = { 1.0, 1.0, 1.0, 0.0 };
//	GLfloat spot_direction[] = { 1.0, 1.0, -1.0 };
//
//	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
//	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
//	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
//	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
//	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
//	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
//
//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
//	glEnable(GL_DEPTH_TEST);
//	glClearColor(1.0, 1.0, 1.0, 1.0);
//
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(120, 1, 1, 80);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	gluLookAt(0, -40, 50, 0, 60, 50, 0, 0, 1);
//}
//
//void displayModel()
//{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	//Polygon Points
//	int polygonCount = mesh->GetPolygonCount();
//	int vertexCounter = 0;
//	vector<FbxVector4> polygonVertex;
//	vector<FbxVector4> vertexNormal;
//
//	FBXSDK_printf("\n\n---------------Polygon Points---------------------\n\n");
//	FBXSDK_printf("polygon points count %d\n", polygonCount);
//
//
//	for (int i = 0; i != polygonCount; ++i) {
//		int polygonSize = mesh->GetPolygonSize(i);
//		//FBXSDK_printf("polygonSize: %d\n", polygonSize);
//
//		vertexNormal.resize(polygonSize);
//
//		for (int j = 0; j != polygonSize; ++j) {
//			int vertexIndex = mesh->GetPolygonVertex(i, j);
//			FbxVector4 vec = controlPoints[vertexIndex];
//			//display3DVector("vec: ", vec);
//			caclNormal(mesh, vertexIndex, vertexCounter, polygonSize, vertexNormal[j]);
//			polygonVertex.push_back(vec);
//		}
//
//		switch (mesh->GetPolygonSize(i))
//		{
//		case 3:
//			glEnable(GL_NORMALIZE);	//normalize
//			glBegin(GL_TRIANGLES);
//			glNormal3f(static_cast<float>(vertexNormal[0].mData[0]), static_cast<float>(vertexNormal[0].mData[1]), static_cast<float>(vertexNormal[0].mData[2]));
//			glVertex3f(polygonVertex[0].mData[0], polygonVertex[0].mData[1], polygonVertex[0].mData[2]);
//			glNormal3f(static_cast<float>(vertexNormal[1].mData[0]), static_cast<float>(vertexNormal[1].mData[1]), static_cast<float>(vertexNormal[1].mData[2]));
//			glVertex3f(polygonVertex[1].mData[0], polygonVertex[1].mData[1], polygonVertex[1].mData[2]);
//			glNormal3f(static_cast<float>(vertexNormal[2].mData[0]), static_cast<float>(vertexNormal[2].mData[1]), static_cast<float>(vertexNormal[2].mData[2]));
//			glVertex3f(polygonVertex[2].mData[0], polygonVertex[2].mData[1], polygonVertex[2].mData[2]);
//			glEnd();
//			break;
//		case 4:
//			glEnable(GL_NORMALIZE);	//normalize
//			glBegin(GL_QUADS);
//			glNormal3f(static_cast<float>(vertexNormal[0].mData[0]), static_cast<float>(vertexNormal[0].mData[1]), static_cast<float>(vertexNormal[0].mData[2]));
//			glVertex3f(polygonVertex[0].mData[0], polygonVertex[0].mData[1], polygonVertex[0].mData[2]);
//			glNormal3f(static_cast<float>(vertexNormal[1].mData[0]), static_cast<float>(vertexNormal[1].mData[1]), static_cast<float>(vertexNormal[1].mData[2]));
//			glVertex3f(polygonVertex[1].mData[0], polygonVertex[1].mData[1], polygonVertex[1].mData[2]);
//			glNormal3f(static_cast<float>(vertexNormal[2].mData[0]), static_cast<float>(vertexNormal[2].mData[1]), static_cast<float>(vertexNormal[2].mData[2]));
//			glVertex3f(polygonVertex[2].mData[0], polygonVertex[2].mData[1], polygonVertex[2].mData[2]);
//			glNormal3f(static_cast<float>(vertexNormal[3].mData[0]), static_cast<float>(vertexNormal[3].mData[1]), static_cast<float>(vertexNormal[3].mData[2]));
//			glVertex3f(polygonVertex[3].mData[0], polygonVertex[3].mData[1], polygonVertex[3].mData[2]);
//			glEnd();
//			break;
//		default:
//			FBXSDK_printf("undefined polygon size: %d\n", mesh->GetPolygonSize(i));
//			break;
//		}
//		++vertexCounter;
//		polygonVertex.clear();
//		vertexNormal.clear();
//	}
//
//	FBXSDK_printf("shape load \n\n");
//	glFlush();
//	glutSwapBuffers();
//}
//
//void SpecialKeys(int key, int x, int y)
//{
//	if (key == GLUT_KEY_UP) xRot -= 2.0;
//	if (key == GLUT_KEY_DOWN) xRot += 2.0;
//	if (key == GLUT_KEY_LEFT)yRot -= 2.0;
//	if (key == GLUT_KEY_RIGHT)yRot += 2.0;
//	if (xRot > 356.0)xRot = 0.0;
//	if (xRot < -1.0)xRot = 355.0;
//	if (yRot > 356.0)yRot = 0.0;
//	if (yRot < -1.0)yRot = 355.0;
//	glutPostRedisplay();
//}
//
//void MouseFunc(int button, int action, int x, int y)
//{
//	//left button to enlarge
//	if (button == GLUT_LEFT_BUTTON && action == GLUT_DOWN){
//		xScale += 0.1;
//		yScale += 0.1;
//		zScale += 0.1;
//	}
//	//right button to shrink
//	if (button == GLUT_RIGHT_BUTTON && action == GLUT_UP){
//		xScale -= 0.1;
//		yScale -= 0.1;
//		zScale -= 0.1;
//	}
//
//	glutPostRedisplay();
//}
//
//void changeSize(GLsizei w, GLsizei h)
//{
//	if (h == 0)		h = 1;
//	glViewport(0, 0, w, h);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	if (w <= h)
//		glOrtho(-200, 200, -200 * h / w, 200 * h / w, -200, 200);
//	else
//		glOrtho(-200 * h / w, 200 * h / w, -200, 200, -200, 200);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	gluLookAt(50, 20, 50, 0, 0, 0, 0, 0, 1);
}