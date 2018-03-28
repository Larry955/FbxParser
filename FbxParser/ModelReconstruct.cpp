#include "ModelReconstruct.h"

#include <vector>
#include <cstdio>
using std::vector;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"	//a third part header,used for resolving the image

GLuint  textureArr[1];         // Storage For One Texture ( NEW )  

//it is recommend that only one object of ModelReconstruct be constructed in the memory
ModelReconstruct::ModelReconstruct(FbxParser *parser, int argc, char **argv) :parser(parser), argc(argc), argv(argv)
{
	initModelSpace();
}

ModelReconstruct::~ModelReconstruct()
{

}

void ModelReconstruct::initModelSpace()
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Reconstructed FBX Model");


	GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_position[] = { 1.0, 0.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	GLfloat light1_ambient[] = { 0.0, 0.2, 0.2, 1.0 };
	GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat light1_specular[] = { 1.0, 0.0, 0.6, 1.0 };
	GLfloat light1_position[] = { 1.0, 0.0, 1.0, 0.0 };
	GLfloat spot_direction[] = { 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, 1, 1, 80000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//FBXSDK_printf("cameraPosX: %f, cameraPosY: %f, cameraPosZ: %f\n\n", cameraPosX, cameraPosY, cameraPosZ);

	gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
		cameraPosX + cameraRotX, cameraPosY + cameraRotY, cameraPosZ,
		0, 0, 1);

	loadGLTextures();
}

void displayCallBack()
{
	currModelRec->display();
}

void timerCallBack(int)
{
	currModelRec->timerFunc();
	
}

void ModelReconstruct::timerFunc()
{

	//loop in the animation stack if not paused
	if (parser->getStopTime() > parser->getStartTime() && parser->getStatus() == MUST_BE_LOADED) {
		//set the scene status flag to refresh 
		//the scene in the next timer call back
		parser->setAnimStatus(MUST_BE_REFRESHED);
		glutPostRedisplay();

		parser->setCurrentTime(parser->getCurrentTime() + parser->getFrameTime());
		/*parser->onTimerClick();*/
		if (parser->getCurrentTime() > parser->getStopTime()) {
			parser->setCurrentTime(parser->getStartTime());
		}
	}
	else {
		parser->setAnimStatus(REFRESHED);
	}
	parser->onTimerClick();
	glutPostRedisplay();
	glutTimerFunc((unsigned int)parser->getFrameTime().GetMilliSeconds(), ::timerCallBack, 0);
}

void ModelReconstruct::display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (deltaMove) {
		computePos(deltaMove);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, 1, 1, 80000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(cameraPosX, cameraPosY, cameraPosZ,
		cameraPosX + cameraRotX, cameraPosY + cameraRotY, cameraPosZ,
		0, 0, 1);
	glPushMatrix();

	glRotatef(xRot, 1.0, 0.0, 0.0);
	glRotatef(zRot, 0.0, 0.0, 1.0);
	

	drawModel();
	//glDisable(GL_TEXTURE_2D);

	glPopMatrix();

	FbxAMatrix lDummyGlobalPosition;
	FbxTime currentTime = parser->getCurrentTime();
	FbxAnimLayer *currAnimLayer = parser->getAnimLayer();
	drawSkeleton(parser->getFbxScene()->GetRootNode(), currentTime, currAnimLayer, lDummyGlobalPosition);
	//drawMesh(parser->getFbxScene()->GetRootNode(), currentTime, currAnimLayer,lDummyGlobalPosition);
	drawGrid(lDummyGlobalPosition);


	glFlush();
	glutSwapBuffers();

	if (parser->getStatus() == MUST_BE_LOADED)
		glutTimerFunc((unsigned int)parser->getFrameTime().GetMilliSeconds(), timerCallBack, 0);
}

void ModelReconstruct::drawModel()
{
	if (parser->getFbxFileName() == "soldier" || parser->getFbxFileName() == "humanoid")
		glRotatef(90, 1.0, 0.0, 0.0);		//used for soldier.fbx

	glScalef(xScale, yScale, zScale);

	glBindTexture(GL_TEXTURE_2D, textureArr[0]);

	//Polygon Points, Normals and UVs
	vector<FbxVector4> polygonPoints = parser->getPolygonPoints();
	vector<FbxVector4> normals = parser->getNormals();
	vector<FbxVector2> uvs = parser->getTextureUVs();

	int uvSize = uvs.size();
	int polygonPointsIndex = 0;
	int polygonIndex = 0;
	while (polygonPointsIndex < polygonPoints.size()) {
		int polygonSize = parser->getFbxMesh()->GetPolygonSize(polygonIndex++);
		if (polygonSize == 3) {
			glEnable(GL_NORMALIZE);	//normalize
			//glColor3f(1.0f, 0.0f, 0.0f);

			glBegin(GL_TRIANGLES);
			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glEnd();
		}
		else if (polygonSize == 4) {
			glEnable(GL_NORMALIZE);	//normalize
			//glColor3f(1.0f, 1.0f, 0.0f);

			glBegin(GL_QUADS);
			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			if (uvs.size() > 0) {
				glNormal3f(static_cast<float>(normals[polygonPointsIndex].mData[0]), static_cast<float>(normals[polygonPointsIndex].mData[1]), static_cast<float>(normals[polygonPointsIndex].mData[2]));
				glTexCoord2f(static_cast<float>(uvs[polygonPointsIndex].mData[0]), static_cast<float>(uvs[polygonPointsIndex].mData[1]));
			}
			glVertex3f(polygonPoints[polygonPointsIndex].mData[0], polygonPoints[polygonPointsIndex].mData[1], polygonPoints[polygonPointsIndex].mData[2]);
			polygonPointsIndex += 1;

			glEnd();
		}
		else {
			FBXSDK_printf("undefined polygon size: %d\n", polygonSize);
		}
	}
}

void ModelReconstruct::drawSkeleton(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &parentGlobalMatrix)
{
	FbxAMatrix globalPosition = parser->getGlobalPosition(node, currTime, &parentGlobalMatrix);

	if (node->GetNodeAttribute()) {
		FbxAMatrix geometryOffset = parser->getGeometry(node);
		FbxAMatrix globalOffPosition = globalPosition * geometryOffset;

		FbxSkeleton *skeleton = (FbxSkeleton*)node->GetNodeAttribute();

		//FBXSDK_printf("skeleton name: %s\n", skeleton->GetName());

		if (skeleton->GetSkeletonType() == FbxSkeleton::eLimbNode &&
			node->GetParent() && node->GetParent()->GetNodeAttribute() &&
			node->GetParent()->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
			
			glPushMatrix();

			glDisable(GL_LIGHTING);	//disabled light before draw lines to make the color of grid works
			glDisable(GL_TEXTURE_2D);
			glRotatef(90, 1.0, 0.0, 0.0);
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
	for (int i = 0; i != childCount; ++i) {
		drawSkeleton(node->GetChild(i), currTime, currAnimLayer, globalPosition);
	}
}

void ModelReconstruct::drawMesh(FbxNode *node, FbxTime currTime, FbxAnimLayer *animLayer, FbxAMatrix &parentGlobalPosition)
{
	FbxAMatrix globalPosition = parser->getGlobalPosition(node, currTime, &parentGlobalPosition);
	FbxAMatrix geometryOffset;
	FbxAMatrix globalOffPosition;
	
	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
		geometryOffset = parser->getGeometry(node);
		globalOffPosition = globalPosition * geometryOffset;

		FbxMesh *mesh = (FbxMesh*)node->GetNodeAttribute();
		const int vertexCount = mesh->GetControlPointsCount();
		if (vertexCount == 0)
			return;

		const VBOMesh *meshCache = static_cast<const VBOMesh*>(mesh->GetUserDataPtr());

		const bool hasVertexCache = mesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
			(static_cast<FbxVertexCacheDeformer*>(mesh->GetDeformer(0, FbxDeformer::eVertexCache))->Active.Get());
		const bool hasShape = mesh->GetShapeCount() > 0;
		const bool hasSkin = mesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
		const bool hasDeformer = hasVertexCache || hasSkin || hasShape;

		FbxVector4 *vertexArray = nullptr;
		if (!meshCache || hasDeformer) {
			vertexArray = new FbxVector4[vertexCount];
			memcpy(vertexArray, mesh->GetControlPoints(), vertexCount * sizeof(FbxVector4));
		}

		if (hasDeformer) {
			if (hasVertexCache) {
				meshCache->ReadVertexCacheData(mesh, currTime, vertexArray);
			}
			else {
				if (hasShape) {
					// compute the vertex array with the shapes
					computeShapeDeformation(mesh, currTime, animLayer, vertexArray);
				}

				//get the number of clusters
				const int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
				int clusterCount = 0;
				for (int skinIndex = 0; skinIndex != skinCount; ++skinIndex) {
					clusterCount += ((FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin))->GetClusterCount();
				}
				if (clusterCount) {
					FbxPose *pose = nullptr;
					computeSkinDeformation(mesh, globalOffPosition, currTime, vertexArray, pose);
				}
			}

			if (meshCache) {
				meshCache->UpdateVertexPosition(mesh, vertexArray);
			}
		}


		glPushMatrix();
		glMultMatrixd((const double*)globalOffPosition);

		if (meshCache) {
			meshCache->BeginDraw();
			const int subMeshCount = meshCache->GetSubMeshCount();

			for (int i = 0; i != subMeshCount; ++i) {
				const FbxSurfaceMaterial *material = node->GetMaterial(i);
				if (material) {
					const MaterialCache *materialCache = static_cast<MaterialCache*>(mesh->GetUserDataPtr());
					if (materialCache) {
						materialCache->SetCurrentMaterial();
					}
				}
				else {

					//draw green for faces without material
					MaterialCache::SetDefaultMaterial();
				}
				meshCache->Draw(i);
			}
			meshCache->EndDraw();
		}
		else {
			glColor4f(0.0, 0.0, 1.0, 1.0);
			const int polygonCount = mesh->GetPolygonCount();
			for (int polygonIndex = 0; polygonIndex != polygonCount; ++polygonIndex) {
				const int polygonSize = mesh->GetPolygonSize(polygonIndex);

				glRotatef(90, 1.0, 0.0, 0.0);
				glBegin(GL_LINE_LOOP);
				for (int vertexIndex = 0; vertexIndex != vertexCount; ++vertexIndex) {
					glVertex3dv((const double*)vertexArray[mesh->GetPolygonVertex(polygonIndex, vertexIndex)]);
				}
				glEnd();

			}

		}
		glPopMatrix();
		delete[] vertexArray;
	}

	int childCount = node->GetChildCount();
	for (int i = 0; i != childCount; ++i) {
		drawMesh(node->GetChild(i), currTime, animLayer, globalOffPosition);
	}
}

void ModelReconstruct::computePos(GLfloat deltaMove)
{
	cameraPosX += deltaMove * cameraRotX * 0.1f;
	cameraPosY += deltaMove * cameraRotY * 0.1f;
}

void ModelReconstruct::displayModel()
{
	currModelRec = this;
	glutDisplayFunc(::displayCallBack);
	
}

void ModelReconstruct::drawGrid(const FbxAMatrix & pTransform)
{
	

	glPushMatrix();
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
	for (i = -hw; i <= hw; i += step) {

		if (i % bigstep == 0) {
			glLineWidth(2.0);
		}
		else {
			glLineWidth(1.0);
		}
		if (i == 0) {
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
		else {
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
	glPopMatrix();
}


void ModelReconstruct::computeShapeDeformation(FbxMesh *mesh, FbxTime &currTime, FbxAnimLayer *animLayer, FbxVector4 *vertexArray)
{
	int lVertexCount = mesh->GetControlPointsCount();

	FbxVector4* lSrcVertexArray = vertexArray;
	FbxVector4* lDstVertexArray = new FbxVector4[lVertexCount];
	memcpy(lDstVertexArray, vertexArray, lVertexCount * sizeof(FbxVector4));

	int lBlendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for (int lBlendShapeIndex = 0; lBlendShapeIndex<lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)mesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for (int lChannelIndex = 0; lChannelIndex<lBlendShapeChannelCount; ++lChannelIndex)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if (lChannel)
			{
				// Get the percentage of influence on this channel.
				FbxAnimCurve* lFCurve = mesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, animLayer);
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

	memcpy(vertexArray, lDstVertexArray, lVertexCount * sizeof(FbxVector4));

	delete[] lDstVertexArray;
}

void ModelReconstruct::computeSkinDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if (lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		computeLinearDeformation(mesh, globalPosition, currTime, vertexArray, pose);
	}
	else if (lSkinningType == FbxSkin::eDualQuaternion)
	{
		computeDualQuaternionDeformation(mesh, globalPosition, currTime, vertexArray, pose);
	}
	else if (lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = mesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, mesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, mesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		computeLinearDeformation(mesh, globalPosition, currTime, lVertexArrayLinear, pose);
		computeDualQuaternionDeformation(mesh, globalPosition, currTime, lVertexArrayDQ, pose);

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

void ModelReconstruct::computeLinearDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose)
{
	//all the links must have the same link node
	FbxCluster::ELinkMode clusterMode = ((FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int vertexCount = mesh->GetControlPointsCount();
	FbxAMatrix *clusterDeformation = new FbxAMatrix[vertexCount];
	memset(clusterDeformation, 0, vertexCount * sizeof(FbxAMatrix));

	double *clusterWeight = new double[vertexCount];
	memset(clusterWeight, 0, vertexCount * sizeof(double));

	if (clusterMode == FbxCluster::eAdditive) {
		for (int i = 0; i != vertexCount; ++i) {
			clusterDeformation[i].SetIdentity();
		}
	}

	//for all skins and all clusters, accumulate their deformation and weight
	//on each vertices and store them in clusterDeformation and clusterWeight
	int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int skinIndex = 0; skinIndex != skinCount; ++skinIndex) {
		FbxSkin *skinDeformer = (FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin);

		int clusterCount = skinDeformer->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex != clusterCount; ++clusterIndex) {
			FbxCluster *cluster = skinDeformer->GetCluster(clusterIndex);
			if (!cluster->GetLink())
				return;
			FbxAMatrix vertexTransformMatrix;
			computeClusterDeformation(mesh, globalPosition, cluster, vertexTransformMatrix, currTime, pose);
			
			int vertexIndexCount = cluster->GetControlPointIndicesCount();
			for (int vertexIndex = 0; vertexIndex != vertexIndexCount; ++vertexIndex) {
				int index = cluster->GetControlPointIndices()[vertexIndex];
				if (index >= vertexIndexCount)
					continue;

				double weight = cluster->GetControlPointWeights()[index];
				if (weight == 0)
					continue;

				//compute the influence of the link on the vertex
				FbxAMatrix influence = vertexTransformMatrix;
				matrixScale(influence, weight);

				if (clusterMode == FbxCluster::eAdditive) {
					matrixAddToDiagonal(influence, 1 - weight);

					clusterDeformation[index] = influence * clusterDeformation[index];
					clusterWeight[index] = 1.0;
				}
				else
				{
					matrixAdd(clusterDeformation[index], influence);
					clusterWeight[index] += weight;

				}
			}
		}

	}

	//actually deform each vertex here by information stored in clusterDeformation and clustreWeight
	for (int i = 0; i != vertexCount; ++i) {
		FbxVector4 srcVertex = vertexArray[i];
		FbxVector4 &desVertex = vertexArray[i];
		double weight = clusterWeight[i];

		//deform the vertex id there is at least one link with an influence on the vertex
		if (weight != 0.0) {
			desVertex = clusterDeformation[i].MultT(srcVertex);
			if (clusterMode == FbxCluster::eNormalize) { 
				desVertex /= weight;
			}
			else if (clusterMode == FbxCluster::eTotalOne) {
				srcVertex *= (1.0 - weight);
				desVertex += srcVertex;
			}
		}
	
	}
	delete[] clusterDeformation;
	delete[] clusterWeight;

}

void ModelReconstruct::matrixAdd(FbxAMatrix &desMat, const FbxAMatrix &srcMat)
{
	for (int i = 0; i != 4; ++i)
		for (int j = 0; j != 4; ++j)
			desMat[i][j] += srcMat[i][j];
}

void ModelReconstruct::matrixScale(FbxAMatrix &influence, double weight)
{
	for (int i = 0; i != 4; ++i)
		for (int j = 0; j != 4; ++j)
			influence[i][j] *= weight;
}

void ModelReconstruct::matrixAddToDiagonal(FbxAMatrix &influence, double value)
{
	for (int i = 0; i != 4; ++i)
		for (int j = 0; j != 4; ++j)
			if (i == j)
				influence[i][j] += value;
}


void ModelReconstruct::computeDualQuaternionDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose)
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
	for (int skinIndex = 0; skinIndex != skinCount; ++skinIndex) {
		FbxSkin *skinDeformer = (FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
		int clusterCount = skinDeformer->GetClusterCount();

		for (int clusterIndex = 0; clusterIndex != clusterCount; ++clusterIndex) {
			FbxCluster *cluster = skinDeformer->GetCluster(clusterIndex);
			if (!cluster->GetLink())
				continue;
			FbxAMatrix vertexTransformMatrix;
			computeClusterDeformation(mesh, globalPosition, cluster, vertexTransformMatrix, currTime, pose);

			FbxQuaternion lQ = vertexTransformMatrix.GetQ();
			FbxVector4 lT = vertexTransformMatrix.GetT();

			FbxDualQuaternion dualQuaternion(lQ, lT);

			int vertexIndexCount = cluster->GetControlPointIndicesCount();
			for (int vertexIndex = 0; vertexIndex != vertexIndexCount; ++vertexIndex) {
				int index = cluster->GetControlPointIndices()[vertexIndex];

				if (index >= vertexIndexCount)
					continue;
				
				double weight = cluster->GetControlPointWeights()[index];

				if (weight == 0.0)
					continue;
				
				//compute the influence of the link on the vertex
				FbxDualQuaternion influence = dualQuaternion * weight;
				if (clusterMode == FbxCluster::eAdditive) {
					//simply influenced by the dual quaternion
					DQClusterDeformation[index] = influence;
					clusterWeight[index] = 1.0;

				}
				else {
					if (clusterIndex == 0) {
						DQClusterDeformation[index] = influence;
					}
					else {
						//add to the sum of the deformations on the vertex
						//make sure the deformation is accumulated in the same direction
						//use the Dot Product to judge the sign
						double sign = DQClusterDeformation[index].GetFirstQuaternion().DotProduct(dualQuaternion.GetFirstQuaternion());
						if (sign >= 0.0) {
							DQClusterDeformation[index] += influence;
						}
						else {
							DQClusterDeformation[index] += influence;
						}
					}
				}
				clusterWeight[index] += weight;
			}
		}
	}

	//actually deform each vertex here by information sroted in clusterDeformation and clusterWeight
	for (int i = 0; i != vertexCount; ++i) {
		FbxVector4 srcVertex = vertexArray[i];
		FbxVector4 &desVertex = vertexArray[i];
		double weightSum = clusterWeight[i];

		//deform the vertex if there is at least one link with an influence on the vertex
		if (weightSum != 0.0) {
			DQClusterDeformation[i].Normalize();
			desVertex = DQClusterDeformation[i].Deform(desVertex);

			if (clusterMode == FbxCluster::eAdditive) {
				desVertex /= weightSum;
			}
			else if (clusterMode == FbxCluster::eTotalOne) {
				srcVertex *= (1 - weightSum);
				desVertex += srcVertex;
			}
		}
	}
	delete[] DQClusterDeformation;
	delete[] clusterWeight;
}

void ModelReconstruct::computeClusterDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxCluster *cluster, FbxAMatrix &vertexTransformMatrix, FbxTime &currTime, FbxPose *pose)
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
		lAssociateGeometry = parser->getGeometry(cluster->GetAssociateModel());
		lAssociateGlobalInitPosition *= lAssociateGeometry;
		lAssociateGlobalCurrentPosition = parser->getGlobalPosition(cluster->GetAssociateModel(), currTime, &lParentGlobalMatrix);

		cluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = parser->getGeometry(mesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		lReferenceGlobalCurrentPosition = globalPosition;

		// Get the link initial global position and the link current global position.
		cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = parser->getGeometry(cluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = parser->getGlobalPosition(cluster->GetLink(), currTime, &lParentGlobalMatrix);

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
		lReferenceGeometry = parser->getGeometry(mesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		cluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = parser->getGlobalPosition(cluster->GetLink(), currTime, &lParentGlobalMatrix);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		vertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}

void ModelReconstruct::keyFunc(unsigned char key, int x, int y)
{
	//resetTransformFactor();

	switch (key) {
	case 'w':
	case 'W':
		deltaMove = 1.0f;
		cameraOffY = 1.0f;
		//xRot -= 2.0;
		break;
	case 's':
	case 'S':
		deltaMove = -1.0f;

		cameraOffY = -1.0f;
		//xRot += 2.0;
		break;
	case 'a':
	case 'A':
		cameraOffX = 1.0f;
		zRot -= 2.0;
		break;
	case 'd':
	case 'D':
		zRot += 2.0;
		break;
	case 'z':
	case 'Z':
		xScale += 0.1;
		yScale += 0.1;
		zScale += 0.1;
		break;
	case 'x':
	case 'X':
		xScale -= 0.1;
		yScale -= 0.1;
		zScale -= 0.1;
		break;
	default:
		FBXSDK_printf("undefined key: %c\n",key);
		break;
	}
	if (xRot > 356.0)
		xRot = 0.0;
	if (xRot < -1.0)
		xRot = 355.0;
	if (zRot > 356.0)
		zRot = 0.0;
	if (zRot < -1.0)
		zRot = 355.0;

	glutPostRedisplay();
}

void keysCallBack(unsigned char key, int x, int y)
{
	currModelRec->keyFunc(key, x, y);
}


void ModelReconstruct::activateKeyBoard()
{
	
	currModelRec = this;
	glutKeyboardFunc(::keysCallBack);
}


void ModelReconstruct::motionFunc(int x, int y)
{
	// this will only be true when the left button is down
	if (xOrigin >= 0) {
		// update deltaAngle
		deltaAngle = (x - xOrigin) * 0.001f;

		// update camera's direction
		cameraRotX = -sin(angle + deltaAngle);
		cameraRotY = cos(angle + deltaAngle);
		
		//FBXSDK_printf("cameraRotX: %f, cameraRotY: %f, x: %d\n\n", cameraRotX, cameraRotY, x);
	}
	else {
		FBXSDK_printf("nothing \n\n");
	}
}


void motionCallBack(int x, int y)
{
	currModelRec->motionFunc(x, y);
}

void ModelReconstruct::activateMotionFunc()
{
	currModelRec = this;
	glutMotionFunc(::motionCallBack);
}

void ModelReconstruct::mouseFunc(int button, int state, int x, int y)
{
	
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			angle += deltaAngle;
			xOrigin = -1;
		}
		else  {// state = GLUT_DOWN
			xOrigin = x;
		}
	}
}

void mouseCallBack(int button, int state, int x, int y)
{
	currModelRec->mouseFunc(button, state, x, y);
}

void ModelReconstruct::activateMouseFunc()
{
	currModelRec = this;
	glutMouseFunc(::mouseCallBack);
}

void ModelReconstruct::keyUpFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 's':
		cameraOffY = 0.0f;
		deltaMove = 0.0f;
		break;
	case 'a':
	case 'd':
		cameraOffX = 0.0f;
		deltaMove = 0.0f;
		break;
	default:
		break;
	}
}

void keyUpCallBack(unsigned char key, int x, int y)
{
	currModelRec->keyUpFunc(key, x, y);
}

void ModelReconstruct::activeKeyUpFunc()
{
	currModelRec = this;
	glutKeyboardUpFunc(::keyUpCallBack);
}

void ModelReconstruct::activeIdleFunc()
{
	currModelRec = this;
	glutIdleFunc(::displayCallBack);
}

void ModelReconstruct::loop()
{
	glutPostRedisplay();
	glutMainLoop();
}

void ModelReconstruct::resetTransformFactor()
{
	xRot = 0.0;
	zRot = 0.0;

	xScale = 1.0;
	yScale = 1.0;
	zScale = 1.0;
	 
}

//since we failed to get the data from texture image, this function is not be called
bool ModelReconstruct::loadGLTextures()
{
	bool status = false;
	RGBImgStructure *textureImage[1];	//create storage space for textures
	memset(textureImage, 0, sizeof(RGBImgStructure*) * 1);	//init the pointer to NULL
	
	FbxString fileName = parser->getTextureFileName();

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureArr[0]);
	glBindTexture(GL_TEXTURE_2D, textureArr[0]); //bind the texture to it's array
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (isNotEmpty(fileName)) {
		switch (getFileSuffix(fileName.Buffer()))
		{
		case eTextureType::TGA:
			return loadTGA(fileName.Buffer(), textureArr[0]);
			break;
		case eTextureType::DDS:
			return loadDDS(fileName.Buffer());
		case eTextureType::BMP:
			textureImage[0] = loadBMP(fileName.Buffer());
			break;
		case eTextureType::UNKNOWN:
			FBXSDK_printf("error!\n\n");
			break;
		default:
			break;
		}
	}
	return status;
}


void ModelReconstruct::reshapeFunc(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;

	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(120.0f, ratio, 1.0f, 80000.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void reshapeCallBack(int w, int h)
{
	currModelRec->reshapeFunc(w, h);
}

void ModelReconstruct::activeReshapFunc()
{
	currModelRec = this;
	glutReshapeFunc(::reshapeCallBack);
}