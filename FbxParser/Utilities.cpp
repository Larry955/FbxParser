#include "Utilities.h"

void MatrixAdd(FbxAMatrix &desMat, const FbxAMatrix &srcMat)
{
	for (int i = 0; i != 4; ++i)
		for (int j = 0; j != 4; ++j)
			desMat[i][j] += srcMat[i][j];
}

void MatrixScale(FbxAMatrix &influence, double weight)
{
	for (int i = 0; i != 4; ++i)
		for (int j = 0; j != 4; ++j)
			influence[i][j] *= weight;
}

void MatrixAddToDiagonal(FbxAMatrix &influence, double value)
{
	for (int i = 0; i != 4; ++i)
		for (int j = 0; j != 4; ++j)
			if (i == j)
				influence[i][j] += value;
}

void Display3DVector(const char *prefix, FbxVector4 pValue)
{
	FbxString vec("");
	FbxString val1 = (float)pValue[0];
	FbxString val2 = (float)pValue[1];
	FbxString val3 = (float)pValue[2];
	//FbxString val4 = (float)pValue[3];

	val1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : val1.Buffer();
	val1 = pValue[0] >= HUGE_VAL ? "INFINITY" : val1.Buffer();
	val2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : val2.Buffer();
	val2 = pValue[1] >= HUGE_VAL ? "INFINITY" : val2.Buffer();
	val3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : val3.Buffer();
	val3 = pValue[2] >= HUGE_VAL ? "INFINITY" : val3.Buffer();
	/*val4 = pValue[3] <= -HUGE_VAL ? "-INFINITY" : val4.Buffer();
	val4 = pValue[3] >= HUGE_VAL ? "INFINITY" : val4.Buffer();
	*/
	vec = prefix;
	vec += val1;
	vec += ", ";
	vec += val2;
	vec += ", ";
	vec += val3;
	/*vec += ", ";
	vec += val4;*/
	vec += "\n";
	FBXSDK_printf(vec);
}

void DisplayInfoOnce(const char *prefix, int index, FbxString info)
{
	if (index == 1) {
		FbxString log("");
		log += prefix + info + "\n";
		FBXSDK_printf(log);
	}
}

bool IsNotEmpty(FbxString str)
{
	return (str == nullptr || str == FbxString("")) ? false : true;
}

void DisplayString(const char* pHeader, const char* pValue, const char* pSuffix)
{
	FbxString lString;

	lString = pHeader;
	lString += pValue;
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf(lString);
}


void DisplayBool(const char* pHeader, bool pValue, const char* pSuffix)
{
	FbxString lString;

	lString = pHeader;
	lString += pValue ? "true" : "false";
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf(lString);
}


void DisplayInt(const char* pHeader, int pValue, const char* pSuffix)
{
	FbxString lString;

	lString = pHeader;
	lString += pValue;
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf(lString);
}