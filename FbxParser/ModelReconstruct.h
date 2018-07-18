#ifndef MODELRECONSTRUCT_H
#define MODELRECONSTRUCT_H

#include "FbxParser.h"
#include <gl\glew.h>
#include <gl\glut.h>
#include <gl\GLU.h>

bool InitOpenGL(int argc, char **argv);	//initialize OpenGL environments
void RunOpenGL();				//run loop
void Display();					

void DrawGrid(const FbxAMatrix & pTransform);		//draw grids
//draw node
void DrawNodeRecursive(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &globalPosition);
//draw camera
void DrawCamera(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &globalPosition);
//draw model
void DrawModel();
//draw skeleton
void DrawSkeleton(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &parentGlobalPosition);
//draw mesh
void DrawMesh(FbxNode *node, FbxTime currTime, FbxAnimLayer *currAnimLayer, FbxAMatrix &globalPosition, ShadingMode shadingMode, int meshCount);

//compute deformation
void ComputeShapeDeformation(FbxMesh *mesh, FbxTime &currTime, FbxAnimLayer *animLayer, FbxVector4 *vertexArray);
void ComputeSkinDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose);
void ComputeLinearDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose);
void ComputeDualQuaternionDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxTime &currTime, FbxVector4 *vertexArray, FbxPose *pose);
void ComputeClusterDeformation(FbxMesh *mesh, FbxAMatrix &globalPosition, FbxCluster *cluster, 
														FbxAMatrix &vertexTransformMatrix, FbxTime &currTime, FbxPose *pose);
//glut functions
void MouseFunc(int button, int state, int x, int y);
void MotionFunc(int x, int y);
void ReshapeFunc(int w, int h);
void TimerFunc(int);
//create menus
void CreateMenus();

#endif
