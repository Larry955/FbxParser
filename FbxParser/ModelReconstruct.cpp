#include "ModelReconstruct.h"
#include "AddMenuHelper.h"
#include "CameraManager.h"
#include "VBOMesh.h"
#include <vector>
#include <cstdio>
using std::vector;

extern FbxParser *parser;
extern bool gSupportVBO;

GLuint  textureArr[1];         // Storage For One Texture ( NEW )  

int windowWidth = 800;
int windowHeight = 1200;

CameraManager *cameraManager = new CameraManager();

bool InitOpenGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Reconstructed FBX Model");

	// Initialize GLEW.
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		FBXSDK_printf("GLEW Error: %s\n", glewGetErrorString(err));
		return false;
	}
	GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_position[] = { 1.0, 0.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	return true;
}

void RunOpenGL()
{
	glutDisplayFunc(Display);
	glutReshapeFunc(ReshapeFunc);
	glutIdleFunc(Display);
	/*glutKeyboardFunc(KeyFunc);
	glutKeyboardUpFunc(KeyUpFunc);*/
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);

	glutPostRedisplay();
	glutMainLoop();
}

void TimerFunc(int)
{
	//loop in the animation stack if not paused
	if (parser->GetStatus() == MUST_BE_REFRESHED)
	{
		glutPostRedisplay();
	}
	parser->OnTimerClick();
	glutTimerFunc((unsigned int)parser->GetFrameTime().GetMilliSeconds(), TimerFunc, 0);
}

void CreateMenus()
{
	//create the submenu to select the current animation stack
	int animStackMenu = glutCreateMenu(AnimStackSelectCallback);
	int currAnimStackIndex = 0;

	//add the animation stack names
	const FbxArray<FbxString*> &animStackArray = parser->GetAnimStackArray();
	for (int poseIndex = 0; poseIndex != animStackArray.GetCount(); ++poseIndex)
	{
		glutAddMenuEntry(animStackArray[poseIndex]->Buffer(), poseIndex);

		//track the current animation stack index
		if (animStackArray[poseIndex]->Compare(parser->GetFbxScene()->ActiveAnimStackName.Get()) == 0)
		{
			currAnimStackIndex = poseIndex;
		}
	}

	//call the animation stack selection callback immediately to initialize the start,stop and current time
	AnimStackSelectCallback(currAnimStackIndex);

	//create the submenu to select the current camera
	int cameraMenu = glutCreateMenu(CameraSelectCallback);

	//add the producer cameras
	glutAddMenuEntry(FBXSDK_CAMERA_PERSPECTIVE, PRODUCER_PERSPECTIVE_ITEM);
	glutAddMenuEntry(FBXSDK_CAMERA_TOP, PRODUCER_TOP_ITEM);
	glutAddMenuEntry(FBXSDK_CAMERA_BOTTOM, PRODUCER_BOTTOM_ITEM);
	glutAddMenuEntry(FBXSDK_CAMERA_FRONT, PRODUCER_FRONT_ITEM);
	glutAddMenuEntry(FBXSDK_CAMERA_BACK, PRODUCER_BACK_ITEM);
	glutAddMenuEntry(FBXSDK_CAMERA_RIGHT, PRODUCER_RIGHT_ITEM);
	glutAddMenuEntry(FBXSDK_CAMERA_LEFT, PRODUCER_LEFT_ITEM);

	//add the camera switcher if there is at least one camera in the scene
	const FbxArray<FbxNode*> &lCameraArray = parser->GetCameraArray();
	if (lCameraArray.GetCount() > 0)
	{
		glutAddMenuEntry(FBXSDK_CAMERA_SWITCHER, CAMERA_SWITCHER_ITEM);
	}

	//add the cameras contained in the scene
	for (int lCameraIndex = 0; lCameraIndex != lCameraArray.GetCount(); ++lCameraIndex)
	{
		glutAddMenuEntry(lCameraArray[lCameraIndex]->GetName(), lCameraIndex);
	}

	//create the submenu to select the shading mode
	const int shadingModeMenu = glutCreateMenu(ShadingModeSelectCallback);
	glutAddMenuEntry(SHADING_MODE_SHADED_STRING, SHADING_MODE_SHADED_ITEM);
	glutAddMenuEntry(SHADING_MODE_WIREFRAME_STRING, SHADING_MODE_WIREFRAME_ITEM);

	//Create Menus
	glutCreateMenu(MenuSelectCallback);
	glutAddSubMenu("Select Animation Stack", animStackMenu);
	glutAddSubMenu("Select Camera", cameraMenu);
	glutAddSubMenu("Select Shading Mode", shadingModeMenu);
	glutAddMenuEntry("Play", PLAY_ANIMATION);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	Status animStatus = parser->GetStatus();
	if (animStatus != UNLOADED && animStatus != MUST_BE_LOADED)
	{
		cameraManager->SetCamera(parser->GetFbxScene(), parser->GetCurrentTime(), parser->GetAnimLayer(), parser->GetCameraArray(), windowWidth, windowHeight);
		//DrawModel();
		//glDisable(GL_TEXTURE_2D);

		FbxAMatrix lDummyGlobalPosition;
		FbxTime currentTime = parser->GetCurrentTime();
		FbxAnimLayer *currAnimLayer = parser->GetAnimLayer();
		//DrawSkeleton(parser->GetFbxScene()->GetRootNode(), currentTime, currAnimLayer, lDummyGlobalPosition);
		
		//DrawCamera(parser->GetFbxScene()->GetRootNode(), currentTime, currAnimLayer,lDummyGlobalPosition);
		//lDummyGlobalPosition.SetIdentity();
		ShadingMode shadingMode = parser->GetShadingMode();
		DrawMesh(parser->GetFbxScene()->GetRootNode(), currentTime, currAnimLayer, lDummyGlobalPosition, shadingMode,0);
		//glPopMatrix();
		DrawGrid(lDummyGlobalPosition);

		glFlush();
	}
	glutSwapBuffers();
	if (parser->GetStatus() == MUST_BE_LOADED)
	{
		// This function is only called in the first display callback
		// to make sure that the application window is opened and a 
		// status message is displayed before.
		parser->LoadCacheRecursive(parser->GetFbxScene(), parser->GetAnimLayer(), parser->GetFbxFileName(), gSupportVBO);
		parser->SetAnimStatus(MUST_BE_REFRESHED);
		CreateMenus();
		glutTimerFunc((unsigned int)parser->GetFrameTime().GetMilliSeconds(), TimerFunc, 0);
	}
}

void DrawCamera(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &globalPosition)
{
	FbxAMatrix lCameraGlobalPosition;
	FbxVector4 lCameraPosition, lCameraDefaultDirection, lCameraInterestPosition;

	if (node->GetNodeAttribute())
	{
		if (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eCamera)
		{
			lCameraPosition = globalPosition.GetT();

			// By default, FBX cameras point towards the X positive axis.
			FbxVector4 lXPositiveAxis(1.0, 0.0, 0.0);
			lCameraDefaultDirection = lCameraPosition + lXPositiveAxis;

			lCameraGlobalPosition = globalPosition;

			// If the camera is linked to an interest, get the interest position.
			if (node->GetTarget())
			{
				lCameraInterestPosition = parser->GetGlobalPosition(node->GetTarget(), currTime).GetT();

				// Compute the required rotation to make the camera point to it's interest.
				FbxVector4 lCameraDirection;
				FbxVector4::AxisAlignmentInEulerAngle(lCameraPosition,
					lCameraDefaultDirection,
					lCameraInterestPosition,
					lCameraDirection);

				// Must override the camera rotation 
				// to make it point to it's interest.
				lCameraGlobalPosition.SetR(lCameraDirection);
			}

			// Get the camera roll.
			FbxCamera* cam = node->GetCamera();
			double lRoll = 0;

			if (cam)
			{
				lRoll = cam->Roll.Get();
				FbxAnimCurve* fc = cam->Roll.GetCurve(currAnimLayer);
				if (fc) fc->Evaluate(currTime);
			}

			//draw camera
			glColor3d(1.0, 1.0, 1.0);
			glLineWidth(1.0);

			glPushMatrix();
			glMultMatrixd((double*)lCameraGlobalPosition);
			glRotated(lRoll, 1.0, 0.0, 0.0);
			glTranslatef(50, 150, 50);

			int i;
			float lCamera[10][2] = { { 0, 5.5 }, { -3, 4.5 },
			{ -3, 7.5 }, { -6, 10.5 }, { -23, 10.5 },
			{ -23, -4.5 }, { -20, -7.5 }, { -3, -7.5 },
			{ -3, -4.5 }, { 0, -5.5 } };

			glBegin(GL_LINE_LOOP);
			{
				for (i = 0; i < 10; i++)
				{
					glVertex3f(lCamera[i][0], lCamera[i][1], 4.5);
				}
			}
			glEnd();

			glBegin(GL_LINE_LOOP);
			{
				for (i = 0; i < 10; i++)
				{
					glVertex3f(lCamera[i][0], lCamera[i][1], -4.5);
				}
			}
			glEnd();

			for (i = 0; i < 10; i++)
			{
				glBegin(GL_LINES);
				{
					glVertex3f(lCamera[i][0], lCamera[i][1], -4.5);
					glVertex3f(lCamera[i][0], lCamera[i][1], 4.5);
				}
				glEnd();
			}

			glPopMatrix();
		}
	}
	int childCount = node->GetChildCount();
	for (int i = 0; i != childCount; ++i)
	{
		DrawCamera(node->GetChild(i), currTime, currAnimLayer, lCameraGlobalPosition);
	}
}

void DrawModel()
{
	glBindTexture(GL_TEXTURE_2D, textureArr[0]);

	//Polygon Points, Normals and UVs
	vector<FbxVector4> polygonPoints = parser->GetPolygonPoints();
	vector<FbxVector4> normals = parser->GetNormals();
	vector<FbxVector2> uvs = parser->GetTextureUVs();

	int uvSize = uvs.size();
	int polygonPointsIndex = 0;
	int polygonIndex = 0;
	while (polygonPointsIndex < polygonPoints.size()) 
	{
		int polygonSize = parser->GetFbxMesh()->GetPolygonSize(polygonIndex++);
		glEnable(GL_NORMALIZE);	//normalize
		glColor3f(1.0f, 0.0f, 0.0f);

		glBegin(GL_TRIANGLES);
		if (normals.size() > 0 && polygonPointsIndex < normals.size())
		{
			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
		}
		if (uvs.size() > 0 && polygonPointsIndex < uvs.size())
		{
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
		}
		glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
		polygonPointsIndex += 1;

		if (normals.size() > 0 && polygonPointsIndex < normals.size())
		{
			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
		}
		if (uvs.size() > 0 && polygonPointsIndex < uvs.size())
		{
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
		}
		glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
		polygonPointsIndex += 1;

		if (normals.size() > 0 && polygonPointsIndex < normals.size())
		{
			glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
		}
		if (uvs.size() > 0 && polygonPointsIndex < uvs.size())
		{
			glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
		}
		glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
		polygonPointsIndex += 1;

		glEnd();
	}
}

void DrawSkeleton(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &parentGlobalMatrix)
{
	FbxAMatrix globalPosition = parser->GetGlobalPosition(node, currTime, &parentGlobalMatrix);

	if (node->GetNodeAttribute()) 
	{
		FbxAMatrix geometryOffset = parser->GetGeometry(node);
		FbxAMatrix globalOffPosition = globalPosition * geometryOffset;

		FbxSkeleton *skeleton = (FbxSkeleton*)node->GetNodeAttribute();

		//FBXSDK_printf("skeleton name: %s\n", skeleton->GetName());

		if (skeleton->GetSkeletonType() == FbxSkeleton::eLimbNode &&
			node->GetParent() && node->GetParent()->GetNodeAttribute() &&
			node->GetParent()->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) 
		{
			
			glPushMatrix();

			glDisable(GL_LIGHTING);	//disabled light before draw lines to make the color of grid works
			glDisable(GL_TEXTURE_2D);
			//glRotatef(90, 1.0, 0.0, 0.0);
			glColor3f(1.0, 0.0, 0.0);
			glLineWidth(2.0);

			glBegin(GL_LINES);
			
			glVertex3dv((GLdouble*)parentGlobalMatrix.GetT());
			glVertex3dv((GLdouble*)globalOffPosition.GetT());

			glEnd();

			glEnable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);
			glPopMatrix();
		
		}
	}
	const int childCount = node->GetChildCount();
	for (int i = 0; i != childCount; ++i) 
	{
		DrawSkeleton(node->GetChild(i), currTime, currAnimLayer, globalPosition);
	}
}

void DrawMesh(FbxNode *node, FbxTime currTime, FbxAnimLayer *animLayer, FbxAMatrix &globalPosition, ShadingMode shadingMode, int meshCount)
{
	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		//if (meshCount <= 30)
		//{
		//	FbxVector4 lT = node->LclTranslation.Get();
		//	FbxVector4 lR = node->LclRotation.Get();
		//	glTranslatef(lT[0] / 100, lT[1] / 100, lT[2] / 100);
		//	glRotatef(lR[0], 1, 0, 0);
		//	glRotatef(lR[1], 0, 1, 0);
		//	glRotatef(lR[0], 0, 0, 1);
		//}
		//else
		//	return;

		FbxMesh* lMesh = (FbxMesh*)node->GetNodeAttribute();
		//FBXSDK_printf("mesh %d: %s\n", meshCount, lMesh->GetName());



		const int lVertexCount = lMesh->GetControlPointsCount();
		//FBXSDK_printf("lVertexCount in draw mesh function: %d		mesh name: %s\n", lVertexCount, lMesh->GetName());
		// No vertex to draw.
		if (lVertexCount == 0)
		{
			return;
		}

		const VBOMesh * lMeshCache = static_cast<const VBOMesh *>(lMesh->GetUserDataPtr());

		// If it has some defomer connection, update the vertices position
		const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
			(static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
		const bool lHasShape = lMesh->GetShapeCount() > 0;
		const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
		const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

		FbxVector4* lVertexArray = NULL;
		lVertexArray = new FbxVector4[lVertexCount];
		memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		if (lHasDeformation)
		{
			// Active vertex cache deformer will overwrite any other deformer
			if (lHasVertexCache)	//false, jump to else
			{
				//do nothing since we dont need vertex cache
			}
			else
			{
				if (lHasShape)	//true
				{
					ComputeShapeDeformation(lMesh, currTime, animLayer, lVertexArray);
				}

				//get number of joints
				const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
				int lClusterCount = 0;
				for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
				{
					lClusterCount += ((FbxSkin *)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
				}
				if (lClusterCount)
				{
					FbxPose *pose = NULL;
					ComputeSkinDeformation(lMesh, globalPosition, currTime, lVertexArray, pose);
				}
			}

			if (lMeshCache)
				lMeshCache->UpdateVertexPosition(lMesh, lVertexArray);
		}

		glPushMatrix();
		glMultMatrixd((const double*)globalPosition);

		//DrawModel();
		/* OpenGL driver is too lower and use Immediate Mode
		FBXSDK_printf("called\n");*/
		if (lMeshCache)
		{
			lMeshCache->BeginDraw(shadingMode);
			const int lSubMeshCount = lMeshCache->GetSubMeshCount();
			for (int lIndex = 0; lIndex < lSubMeshCount; ++lIndex)
			{
				if (shadingMode == SHADING_MODE_SHADED)
				{
					const FbxSurfaceMaterial * lMaterial = node->GetMaterial(lIndex);
					if (lMaterial)
					{
						const MaterialCache * lMaterialCache = static_cast<const MaterialCache *>(lMaterial->GetUserDataPtr());
						if (lMaterialCache)
						{
							lMaterialCache->SetCurrentMaterial();
						}
					}
					else
					{
						// Draw green for faces without material
						MaterialCache::SetDefaultMaterial();
					}
				}

				lMeshCache->Draw(lIndex, shadingMode);
			}
			lMeshCache->EndDraw();
			//glPopMatrix();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, textureArr[0]);

			vector<FbxVector4> normals = parser->GetNormals();
			vector<FbxVector2> uvs = parser->GetTextureUVs();

			int lNormalsSize = normals.size();
			int lUVsSize = uvs.size();

			//glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
			const int lPolygonCount = lMesh->GetPolygonCount();
			for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; lPolygonIndex++)
			{
				const int lVerticeCount = lMesh->GetPolygonSize(lPolygonIndex);
				glEnable(GL_NORMALIZE);
				glBegin(GL_TRIANGLES);
				for (int lVerticeIndex = 0; lVerticeIndex < lVerticeCount; lVerticeIndex++)
				{
					int index = lMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
					if (lNormalsSize > 0 && lNormalsSize < lPolygonCount)
					{
						glNormal3dv((GLdouble*)normals[lPolygonIndex * 3 + lVerticeIndex]);
					}
					if (lUVsSize > 0 && lUVsSize < lPolygonCount)
					{
						glTexCoord2dv((GLdouble*)uvs[lPolygonIndex * 3 + lVerticeIndex]);
					}
					glVertex3dv((GLdouble *)lVertexArray[index]);
				}
				glEnd();
			}
		}
		glPopMatrix();
		delete[] lVertexArray;
	}

	int childCount = node->GetChildCount();
	for (int i = 0; i != childCount; ++i) 
	{
		DrawMesh(node->GetChild(i), currTime, animLayer, globalPosition, shadingMode, ++meshCount);
	}
}

void DrawGrid(const FbxAMatrix & pTransform)
{
	glRotatef(90, 1.0, 0.0, 0.0);
	//glPushMatrix();
	glMultMatrixd(pTransform);

	// Draw a grid 500*500
	glDisable(GL_LIGHTING);	//disabled light before draw lines to make the color of grid works
	glDisable(GL_TEXTURE_2D);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(0.3f, 0.3f, 0.3f);
	glLineWidth(1.0);
	
	const int hw = 500;
	const int step = 20;
	const int bigstep = 100;
	
	int i;

	// Draw Grid
	for (i = -hw; i <= hw; i += step) 
	{

		if (i % bigstep == 0) 
		{
			glLineWidth(2.0);
		}
		else 
		{
			glLineWidth(1.0);
		}
		if (i == 0) 
		{
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_LINES);
			glVertex3i(i, -hw, 0);
			glVertex3i(i, hw, 0);
			glEnd();

			glColor3f(0.0f, 1.0f, 0.0f);
			glBegin(GL_LINES);
			glVertex3i(-hw, i, 0);
			glVertex3i(hw, i, 0);
			glEnd();
		}
		else 
		{
			glColor3f(0.3f, 0.3f, 0.3f);
			glBegin(GL_LINES);
			glVertex3i(i, -hw, 0);
			glVertex3i(i, hw, 0);
			glEnd();
			
			glBegin(GL_LINES);
			glVertex3i(-hw, i, 0);
			glVertex3i(hw, i, 0);
			glEnd();
		}

	}
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	//glPopMatrix();
}


void ComputeShapeDeformation(FbxMesh *pMesh, FbxTime &currTime, FbxAnimLayer *pAnimLayer, FbxVector4 *pVertexArray)
{
	int lVertexCount = pMesh->GetControlPointsCount();

	FbxVector4* lSrcVertexArray = pVertexArray;
	FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
	memcpy(lDstVertexArray, pVertexArray, lVertexCount * sizeof(FbxVector4));

	int lBlendShapeDeformerCount = pMesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for (int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)pMesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for (int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if (lChannel)	//true
			{
				//FBXSDK_printf("lChannel is not NULL\n");
				// Get the percentage of influence on this channel.
				FbxAnimCurve* lFCurve = pMesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer);
				if (!lFCurve) continue;
				double lWeight = lFCurve->Evaluate(currTime);

				/*
				If there is only one targetShape on this channel, the influence is easy to calculate:
				influence = (targetShape - baseGeometry) * weight * 0.01
				dstGeometry = baseGeometry + influence

				But if there are more than one targetShapes on this channel, this is an in-between
				blendshape, also called progressive morph. The calculation of influence is different.

				For example, given two in-between targets, the full weight percentage of first target
				is 50, and the full weight percentage of the second target is 100.
				When the weight percentage reach 50, the base geometry is already be fully morphed
				to the first target shape. When the weight go over 50, it begin to morph from the
				first target shape to the second target shape.

				To calculate influence when the weight percentage is 25:
				1. 25 falls in the scope of 0 and 50, the morphing is from base geometry to the first target.
				2. And since 25 is already half way between 0 and 50, so the real weight percentage change to
				the first target is 50.
				influence = (firstTargetShape - baseGeometry) * (25-0)/(50-0) * 100
				dstGeometry = baseGeometry + influence

				To calculate influence when the weight percentage is 75:
				1. 75 falls in the scope of 50 and 100, the morphing is from the first target to the second.
				2. And since 75 is already half way between 50 and 100, so the real weight percentage change
				to the second target is 50.
				influence = (secondTargetShape - firstTargetShape) * (75-50)/(100-50) * 100
				dstGeometry = firstTargetShape + influence
				*/

				// Find the two shape indices for influence calculation according to the weight.
				// Consider index of base geometry as -1.

				int lShapeCount = lChannel->GetTargetShapeCount();
				//FBXSDK_printf("lShapeCount: %d\n", lShapeCount);
				
				double* lFullWeights = lChannel->GetTargetShapeFullWeights();

				// Find out which scope the lWeight falls in.
				int lStartIndex = -1;
				int lEndIndex = -1;
				for (int lShapeIndex = 0; lShapeIndex<lShapeCount; ++lShapeIndex)
				{
					if (lWeight > 0 && lWeight <= lFullWeights[0])
					{
						lEndIndex = 0;
						break;
					}
					if (lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex + 1])
					{
						lStartIndex = lShapeIndex;
						lEndIndex = lShapeIndex + 1;
						break;
					}
				}

				FbxShape* lStartShape = NULL;
				FbxShape* lEndShape = NULL;
				if (lStartIndex > -1)
				{
					lStartShape = lChannel->GetTargetShape(lStartIndex);
				}
				if (lEndIndex > -1)
				{
					lEndShape = lChannel->GetTargetShape(lEndIndex);
				}

				//The weight percentage falls between base geometry and the first target shape.
				if (lStartIndex == -1 && lEndShape)
				{
					double lEndWeight = lFullWeights[0];
					// Calculate the real weight.
					lWeight = (lWeight / lEndWeight) * 100;
					// Initialize the lDstVertexArray with vertex of base geometry.
					memcpy(lDstVertexArray, lSrcVertexArray, lVertexCount * sizeof(FbxVector4));
					for (int j = 0; j < lVertexCount; j++)
					{
						// Add the influence of the shape vertex to the mesh vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}
				}
				//The weight percentage falls between two target shapes.
				else if (lStartShape && lEndShape)
				{
					//FBXSDK_printf("update vertex\n");
					double lStartWeight = lFullWeights[lStartIndex];
					double lEndWeight = lFullWeights[lEndIndex];
					// Calculate the real weight.
					lWeight = ((lWeight - lStartWeight) / (lEndWeight - lStartWeight)) * 100;
					// Initialize the lDstVertexArray with vertex of the previous target shape geometry.
					memcpy(lDstVertexArray, lStartShape->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
					for (int j = 0; j < lVertexCount; j++)
					{
						// Add the influence of the shape vertex to the previous shape vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lStartShape->GetControlPoints()[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}
				}
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

	memcpy(pVertexArray, lDstVertexArray, lVertexCount * sizeof(FbxVector4));

	delete[] lDstVertexArray;
}

void ComputeSkinDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if (lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(mesh, globalPosition, currTime, vertexArray, pose);
	}
	else if (lSkinningType == FbxSkin::eDualQuaternion)
	{
		ComputeDualQuaternionDeformation(mesh, globalPosition, currTime, vertexArray, pose);
	}
	else if (lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = mesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, mesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, mesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		ComputeLinearDeformation(mesh, globalPosition, currTime, lVertexArrayLinear, pose);
		ComputeDualQuaternionDeformation(mesh, globalPosition, currTime, lVertexArrayDQ, pose);

		// To blend the skinning according to the blend weights
		// Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
		// DQSVertex: vertex that is deformed by dual quaternion skinning method;
		// LinearVertex: vertex that is deformed by classic linear skinning method;
		int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
		for (int lBWIndex = 0; lBWIndex<lBlendWeightsCount; ++lBWIndex)
		{
			double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
			vertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
		}
	}
}

void ComputeLinearDeformation(FbxMesh *pMesh, FbxAMatrix &pGlobalPosition, FbxTime &currTime, FbxVector4 *pVertexArray, FbxPose *pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
	memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	if (lClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < lVertexCount; ++i)
		{
			lClusterDeformation[i].SetIdentity();
		}
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int lSkinIndex = 0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

		int lClusterCount = lSkinDeformer->GetClusterCount();
		for (int lClusterIndex = 0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pMesh, pGlobalPosition, lCluster, lVertexTransformMatrix, currTime, pPose);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k)
			{
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
				{
					continue;
				}

				// Compute the influence of the link on the vertex.
				FbxAMatrix lInfluence = lVertexTransformMatrix;
				MatrixScale(lInfluence, lWeight);

				if (lClusterMode == FbxCluster::eAdditive)
				{
					// Multiply with the product of the deformations on the vertex.
					MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
					lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					// Add to the sum of the deformations on the vertex.
					MatrixAdd(lClusterDeformation[lIndex], lInfluence);

					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex			
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	//FBXSDK_printf("vertex count: %d\n", lVertexCount);
	for (int i = 0; i < lVertexCount; i++)
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeight = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeight != 0.0)
		{
			lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeight;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeight);
				lDstVertex += lSrcVertex;
			}
		}
	}

	delete[] lClusterDeformation;
	delete[] lClusterWeight;

}

void ComputeDualQuaternionDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose)
{
	//all the links must have the same link mode
	FbxCluster::ELinkMode clusterMode = ((FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int vertexCount = mesh->GetControlPointsCount();
	int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion *DQClusterDeformation = new FbxDualQuaternion[vertexCount];
	memset(DQClusterDeformation, 0, vertexCount * sizeof(FbxDualQuaternion));

	double *clusterWeight = new double[vertexCount];
	memset(clusterWeight, 0, vertexCount * sizeof(double));

	//for all skins and all clusters, accumulate their deformation and weights
	//on each vertices and store them in clusterDeformation and clusterWeight
	for (int skinIndex = 0; skinIndex != skinCount; ++skinIndex) 
	{
		FbxSkin *skinDeformer = (FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
		int clusterCount = skinDeformer->GetClusterCount();

		for (int clusterIndex = 0; clusterIndex != clusterCount; ++clusterIndex) 
		{
			FbxCluster *cluster = skinDeformer->GetCluster(clusterIndex);
			if (!cluster->GetLink())
				continue;
			FbxAMatrix vertexTransformMatrix;
			ComputeClusterDeformation(mesh, globalPosition, cluster, vertexTransformMatrix, currTime, pose);

			FbxQuaternion lQ = vertexTransformMatrix.GetQ();
			FbxVector4 lT = vertexTransformMatrix.GetT();

			FbxDualQuaternion dualQuaternion(lQ, lT);

			int vertexIndexCount = cluster->GetControlPointIndicesCount();
			for (int vertexIndex = 0; vertexIndex != vertexIndexCount; ++vertexIndex) 
			{
				int index = cluster->GetControlPointIndices()[vertexIndex];

				if (index >= vertexIndexCount)
					continue;
				
				double weight = cluster->GetControlPointWeights()[index];

				if (weight == 0.0)
					continue;
				
				//compute the influence of the link on the vertex
				FbxDualQuaternion influence = dualQuaternion * weight;
				if (clusterMode == FbxCluster::eAdditive) 
				{
					//simply influenced by the dual quaternion
					DQClusterDeformation[index] = influence;
					clusterWeight[index] = 1.0;

				}
				else 
				{
					if (clusterIndex == 0) 
					{
						DQClusterDeformation[index] = influence;
					}
					else 
					{
						//add to the sum of the deformations on the vertex
						//make sure the deformation is accumulated in the same direction
						//use the Dot Product to judge the sign
						double sign = DQClusterDeformation[index].GetFirstQuaternion().DotProduct(dualQuaternion.GetFirstQuaternion());
						if (sign >= 0.0) 
						{
							DQClusterDeformation[index] += influence;
						}
						else 
						{
							DQClusterDeformation[index] += influence;
						}
					}
				}
				clusterWeight[index] += weight;
			}
		}
	}

	//actually deform each vertex here by information sroted in clusterDeformation and clusterWeight
	for (int i = 0; i != vertexCount; ++i) 
	{
		FbxVector4 srcVertex = vertexArray[i];
		FbxVector4 &desVertex = vertexArray[i];
		double weightSum = clusterWeight[i];

		//deform the vertex if there is at least one link with an influence on the vertex
		if (weightSum != 0.0) 
		{
			DQClusterDeformation[i].Normalize();
			desVertex = DQClusterDeformation[i].Deform(desVertex);

			if (clusterMode == FbxCluster::eAdditive) 
			{
				desVertex /= weightSum;
			}
			else if (clusterMode == FbxCluster::eTotalOne) 
			{
				srcVertex *= (1 - weightSum);
				desVertex += srcVertex;
			}
		}
	}
	delete[] DQClusterDeformation;
	delete[] clusterWeight;
}

void ComputeClusterDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxCluster *cluster, FbxAMatrix &vertexTransformMatrix, FbxTime &currTime, FbxPose *pose)
{
	FbxCluster::ELinkMode clusterMode = cluster->GetLinkMode();
	FbxAMatrix lReferenceGlobalInitPosition;
	FbxAMatrix lReferenceGlobalCurrentPosition;
	FbxAMatrix lAssociateGlobalInitPosition;
	FbxAMatrix lAssociateGlobalCurrentPosition;
	FbxAMatrix lClusterGlobalInitPosition;
	FbxAMatrix lClusterGlobalCurrentPosition;

	FbxAMatrix lReferenceGeometry;
	FbxAMatrix lAssociateGeometry;
	FbxAMatrix lClusterGeometry;

	FbxAMatrix lClusterRelativeInitPosition;
	FbxAMatrix lClusterRelativeCurrentPositionInverse;
	FbxAMatrix lParentGlobalMatrix;
	lParentGlobalMatrix.SetIdentity();

	if (clusterMode == FbxCluster::eAdditive && cluster->GetAssociateModel())
	{
		cluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
		// Geometric transform of the model
		lAssociateGeometry = parser->GetGeometry(cluster->GetAssociateModel());
		lAssociateGlobalInitPosition *= lAssociateGeometry;
		lAssociateGlobalCurrentPosition = parser->GetGlobalPosition(cluster->GetAssociateModel(), currTime, &lParentGlobalMatrix);

		cluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = parser->GetGeometry(mesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		lReferenceGlobalCurrentPosition = globalPosition;

		// Get the link initial global position and the link current global position.
		cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = parser->GetGeometry(cluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = parser->GetGlobalPosition(cluster->GetLink(), currTime, &lParentGlobalMatrix);

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		vertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
			lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}
	else
	{
		cluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		lReferenceGlobalCurrentPosition = globalPosition;
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = parser->GetGeometry(mesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = parser->GetGlobalPosition(cluster->GetLink(), currTime, &lParentGlobalMatrix);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		vertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}

void MotionFunc(int x, int y)
{
	switch (cameraManager->GetCameraStatus())
	{
	case CAMERA_ORBIT:
		cameraManager->CameraOrbit(parser->GetFbxScene(), x, y);
		parser->SetAnimStatus(MUST_BE_REFRESHED);
		break;
	case CAMERA_ZOOM:
		cameraManager->CameraZoom(parser->GetFbxScene(), y);
		cameraManager->SetLastY(y);
		parser->SetAnimStatus(MUST_BE_REFRESHED);
		break;
	case CAMERA_PAN:
		cameraManager->CameraPan(parser->GetFbxScene(), x, y);
		parser->SetAnimStatus(MUST_BE_REFRESHED);
		break;
	default:
		break;
	}
}

void MouseFunc(int button, int state, int x, int y)
{
	
	FbxCamera *lCamera = cameraManager->GetCurrentCamera(parser->GetFbxScene());
	if (lCamera)
	{
		cameraManager->SetCamPosition(lCamera->Position.Get());
		cameraManager->SetCamCenter(lCamera->InterestPosition.Get());
		cameraManager->SetRoll(lCamera->Roll.Get());

	}
	cameraManager->SetLastX(x);
	cameraManager->SetLastY(y);

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		// ORBIT (or PAN)
		switch (state)
		{
		case GLUT_DOWN:
			if (cameraManager->GetCameraStatus() == CAMERA_ZOOM)
			{
				cameraManager->SetCamStatus(CAMERA_PAN);
			}
			else
			{
				cameraManager->SetCamStatus(CAMERA_ORBIT);
			}
			break;

		default:
			if (cameraManager->GetCameraStatus() == CAMERA_PAN)
			{
				cameraManager->SetCamStatus(CAMERA_ZOOM);
			}
			else
			{
				cameraManager->SetCamStatus(CAMERA_NOTHING);
			}
			break;
		}
		break;

	case GLUT_MIDDLE_BUTTON:
		// ZOOM (or PAN)
		switch (state)
		{
		case GLUT_DOWN:
			if (cameraManager->GetCameraStatus() == CAMERA_ORBIT)
			{
				cameraManager->SetCamStatus(CAMERA_PAN);
			}
			else
			{
				cameraManager->SetCamStatus(CAMERA_ZOOM);
			}
			break;

		default:
			if (cameraManager->GetCameraStatus() == CAMERA_PAN)
			{
				cameraManager->SetCamStatus(CAMERA_ORBIT);
			}
			else
			{
				cameraManager->SetCamStatus(CAMERA_NOTHING);
			}
			break;
		}
		break;
	}
}

void ReshapeFunc(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	windowWidth = w;
	windowHeight = h;
}