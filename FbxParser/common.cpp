#include "common.h"

void display3DVector(const char *prefix, FbxVector4 pValue)
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

void displayInfoOnce(const char *prefix, int index, FbxString info)
{
	if (index == 1) {
		FbxString log("");
		log += prefix + info + "\n";
		FBXSDK_printf(log);
	}
}