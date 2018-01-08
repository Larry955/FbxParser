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

RGBImgStructure* loadTGA(const char* fileName)
{
	RGBImgStructure* textureImage;
	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
	textureImage->width = 0;
	textureImage->height = 0;
	textureImage->data = nullptr;
	
	GLubyte  TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Uncompressed TGA Header
	GLubyte  TGAcompare[12];        // Used To Compare TGA Header
	GLubyte  header[6];         // First 6 Useful Bytes From The Header
	GLuint  bytesPerPixel;        // Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint  imageSize;         // Used To Store The Image Size When Setting Aside Ram
	GLuint  temp;          // Temporary Variable
	GLuint  type = GL_RGBA;        // Set The Default GL Mode To RBGA (32 BPP)
	
	FILE *imageFile = fopen(fileName, "rb");      // Open The TGA File
	if (!imageFile||          // Does File Even Exist?
		fread(TGAcompare, 1, sizeof(TGAcompare), imageFile) != sizeof(TGAcompare) || // Are There 12 Bytes To Read?
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 || // Does The Header Match What We Want?
		fread(header, 1, sizeof(header), imageFile) != sizeof(header))    // If So Read Next 6 Header Bytes
	{
		if (!imageFile)         // Did The File Even Exist?
			return nullptr;         // Return False
		else
		{
			fclose(imageFile);         // If Anything Failed, Close The File
			return nullptr;         // Return False
		}
	}
	textureImage->width = header[1] * 256 + header[0];   //  Determine The TGA Width (highbyte*256+lowbyte)
	textureImage->height = header[3] * 256 + header[2];   // Determine The TGA Height (highbyte*256+lowbyte)

	if (textureImage->width <= 0 ||        // Is The Width Less Than Or Equal To Zero
		textureImage->height <= 0 ||        // Is The Height Less Than Or Equal To Zero
		(header[4] != 24 && header[4] != 32))     // Is The TGA 24 or 32 Bit? RGB Or RGBA
	{
		fclose(imageFile);          // If Anything Failed, Close The File
		return nullptr;          // Return False
	}

	bytesPerPixel = header[4] / 8;      // Divide By 8 To Get The Bytes Per Pixel
	imageSize = textureImage->width * textureImage->height * bytesPerPixel; // Calculate The Memory Required For The TGA Data
	textureImage->data = (unsigned char*)malloc(imageSize);  // Reserve Memory To Hold The TGA Data
	
	if (textureImage->data == nullptr ||       // Does The Storage Memory Exist?
		fread(textureImage->data, 1, imageSize, imageFile) != imageSize) // Does The Image Size Match The Memory Reserved?
	{
		if (textureImage->data != nullptr)      // Was Image Data Loaded
			free(textureImage->data);      // If So, Release The Image Data
		fclose(imageFile);          // Close The File
		return false;          // Return False
	}

	for (GLuint i = 0; i != int(imageSize); i += bytesPerPixel)  // Loop Through The Image Data
	{              // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = textureImage->data[i];       // Temporarily Store The Value At Image Data 'i'
		textureImage->data[i] = textureImage->data[i + 2]; // Set The 1st Byte To The Value Of The 3rd Byte
		textureImage->data[i + 2] = temp;     // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose(imageFile);           // Close The File
	return textureImage;
}

//bool loadDDS(const char *fileName)
//{
//	RGBImgStructure* textureImage;
//	textureImage = (RGBImgStructure*)malloc(sizeof(RGBImgStructure));
//	textureImage->width = 0;	//initialize
//	textureImage->height = 0;
//	textureImage->data = nullptr;
//	
//	FILE* imageFile = nullptr;
//	unsigned long size = 0;
//	imageFile = fopen(fileName, "rb");		//binary file
//
//	if (!imageFile) {
//		std::cerr << "error: open " << fileName << " failed\n\n" << std::endl;
//		return nullptr;
//	}
//
//}

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


	InitializeMagick(*argv);
	try {
		Image img(fileName);
		textureImage->width = img.size().width();
		textureImage->height = img.size().height();
		/*Pixels pixelsCache(img);
		Quantum* pixels;
		pixels = pixelsCache.get(0, 0, img.columns(), img.rows());*/

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