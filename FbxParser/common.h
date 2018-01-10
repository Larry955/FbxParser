#ifndef COMMON_H
#define COMMON_H

#include <fbxsdk.h>

void display3DVector(const char *prefix, FbxVector4 pValue);
void displayInfoOnce(const char *prefix, int index, FbxString info = "");
void displayString(const char* pHeader, const char* pValue, const char* pSuffix  = "" );
void displayBool(const char* pHeader, bool pValue, const char* pSuffix = "");
void displayInt(const char* pHeader, int pValue, const char* pSuffix = "");

bool isNotEmpty(FbxString str);

#endif