#ifndef RGBIMGSTRUCTURE_H
#define RGBIMGSTRUCTURE_H
#include "tgaParser.h"
#include <gl\glut.h>

enum eTextureType
{
	TGA,
	DDS,
	BMP,
	UNKNOWN
};

typedef struct __RGBImgStructure{
	unsigned long width;
	unsigned long height;
	unsigned char *data;
}RGBImgStructure;

eTextureType getFileSuffix(const char *fileName);

bool loadTGA(const char *fileName, GLuint pTextureObject);
RGBImgStructure* loadBMP(const char *fileName);
bool loadDDS(const char *fileName);

RGBImgStructure* loadTextureImg(const char *fileName,char **argv);

#endif