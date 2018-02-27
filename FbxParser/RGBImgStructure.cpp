#include "RGBImgStructure.h"
#include <gl\glut.h>
#include <cstring>
#include <algorithm>
#include <string>
#include <cctype>
#include <cstdio>
#include <iostream>

#include <Magick++.h>
using namespace Magick;

eTextureType getFileSuffix(const char *fileName)
{
	size_t sz = strlen(fileName);
	std::string ret("");
	size_t i = sz - 1;
	while (fileName[i] != '.') {
		ret += isupper(fileName[i]) ? tolower(fileName[i]) : fileName[i];
		--i;
	}
	std::string suffix(ret.rbegin(), ret.rend());

	if (suffix == "dds")
		return eTextureType::DDS;
	else if (suffix == "tga")
		return eTextureType::TGA;
	else if (suffix == "bmp")
		return eTextureType::BMP;
	else
		return eTextureType::UNKNOWN;
}

bool loadTGA(const char* fileName, GLuint pTextureObject)
{
	RGBImgStructure* textureImage;
	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
	textureImage->width = 0;
	textureImage->height = 0;
	textureImage->data = nullptr;
	
	tga_image lTGAImage;

	if (tga_read(&lTGAImage, fileName) == TGA_NOERR)
	{
		// Make sure the image is left to right
		if (tga_is_right_to_left(&lTGAImage))
			tga_flip_horiz(&lTGAImage);

		// Make sure the image is bottom to top
		if (tga_is_top_to_bottom(&lTGAImage))
			tga_flip_vert(&lTGAImage);

		// Make the image BGR 24
		tga_convert_depth(&lTGAImage, 24);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, lTGAImage.width, lTGAImage.height, 0, GL_BGR,
			GL_UNSIGNED_BYTE, lTGAImage.image_data);
		
		tga_free_buffers(&lTGAImage);

		return true;
	}
	else
		return false;
}

bool loadDDS(const char *fileName)
{
	RGBImgStructure* textureImage;
	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
	textureImage->width = 0;	//initialize
	textureImage->height = 0;
	textureImage->data = nullptr;
	
	FILE* imageFile = nullptr;
	unsigned long size = 0;
	imageFile = fopen(fileName, "rb");		//binary file

	if (!imageFile) {
		std::cerr << "error: open " << fileName << " failed\n\n" << std::endl;
		return nullptr;
	}
	return textureImage;
}

RGBImgStructure* loadBMP(const char *fileName)
{
	RGBImgStructure* textureImage;
	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
	textureImage->width = 0;	//initialize
	textureImage->height = 0;
	textureImage->data = nullptr;

	FILE* imageFile = nullptr;
	unsigned long size = 0;
	imageFile = fopen(fileName, "rb");		//binary file

	fseek(imageFile, 18, SEEK_SET);
	fread(&(textureImage->width), 4, 1, imageFile);
	fread(&(textureImage->height), 4, 1, imageFile);
	fseek(imageFile, 0, SEEK_END);
	size = ftell(imageFile) - 54;

	textureImage->data = (unsigned char*)malloc(size);
	memset(textureImage->data, 0, size);
	fseek(imageFile, 54, SEEK_SET);
	fread(textureImage->data, size, 1, imageFile);

	fclose(imageFile);
	return textureImage;
}

RGBImgStructure* loadTextureImg(const char *fileName,char **argv)
{
	RGBImgStructure* textureImage;
	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
	textureImage->width = 0;	//initialize
	textureImage->height = 0;
	textureImage->data = nullptr;


	//use Magick++ to get data from images, but we failed
	InitializeMagick(*argv);
	try {
		Image img(fileName);
		textureImage->width = img.size().width();
		textureImage->height = img.size().height();
		
		Pixels pixelsCache(img);
		Quantum* pixels;
		pixels = pixelsCache.get(0, 0, img.columns(), img.rows());
		

		Blob blob;
		img.write(&blob);
		size_t size = blob.length() + 1;
		textureImage->data = (unsigned char*)malloc(size * sizeof(char));
		
		memcpy(textureImage->data, blob.data(), size);
		textureImage->data[size - 1] = '\0';
		return textureImage;
	}
	catch (Exception &error) {
		std::cerr << "error: " << error.what() << std::endl;
		return nullptr;
	}
}