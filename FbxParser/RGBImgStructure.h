#ifndef RGBIMGSTRUCTURE_H
#define RGBIMGSTRUCTURE_H


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

RGBImgStructure* loadTGA(const char *fileName);
RGBImgStructure* loadBMP(const char *fileName);
bool loadDDS(const char *fileName);

RGBImgStructure* loadTextureImg(const char *fileName,char **argv);

#endif