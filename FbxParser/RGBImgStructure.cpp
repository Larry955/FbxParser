#include "RGBImgStructure.h"

bool LoadTextureImg(const char *fileName)
{
	//1 get the image format
	FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(fileName, 0);

	//2 load texture file
	FIBITMAP *dib = FreeImage_Load(fifmt, fileName, 0);

	//3 convert rgb to 24 bits
	dib = FreeImage_ConvertTo24Bits(dib);

	if (!dib)
		return false;

	//4 get bits of image
	BYTE *pixels = (BYTE*)FreeImage_GetBits(dib);

	if (!pixels)
		return false;
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGB, 
		width, 
		height, 
		0, 
		GL_BGR_EXT, 
		GL_UNSIGNED_BYTE,
		pixels
		);

	//free the memory
	FreeImage_Unload(dib);

	return true;
}