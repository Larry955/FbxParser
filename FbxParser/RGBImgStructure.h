#ifndef RGBIMGSTRUCTURE_H
#define RGBIMGSTRUCTURE_H

#include <gl\glew.h>
#include <GL/gl.h>
#include <gl\glut.h>

#include "FreeImage.h"

// the format of texture, *.tga, *.dds, *.bmp

bool LoadTextureImg(const char *fileName);

#endif