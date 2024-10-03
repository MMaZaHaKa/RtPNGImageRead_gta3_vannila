#pragma once
//#include "Windows.h"
#include <iostream>
#include "RenderWare.h"

#ifdef RW_PS2
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned int uintptr;
#else
/* get rid of the stupid _t */
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uintptr_t uintptr;
#endif

typedef float float32;
typedef int32 bool32;
typedef uint8 byte;
typedef uint32 uint;

#define MAKEPLUGINID(v, id) (((v & 0xFFFFFF) << 8) | (id & 0xFF))
#define MAKEPIPEID(v, id) (((v & 0xFFFF) << 16) | (id & 0xFFFF))
#define VEND_CORE 0
enum MemHint
{
	MEMDUR_NA = 0,
	// used inside function
	MEMDUR_FUNCTION = 0x10000,
	// used for one frame
	MEMDUR_FRAME = 0x20000,
	// used for longer time
	MEMDUR_EVENT = 0x30000,
	// used while the engine is running
	MEMDUR_GLOBAL = 0x40000,

	ID_IMAGE = MAKEPLUGINID(VEND_CORE, 0x18),

};

// usage
//RwImage* image = (RwImage*)RtPNGImageRead("Memes\\1.png");
//RwRasterSetFromImage(pTex->raster, image);

struct RtImage : RwImage
{
	//int32 flags;
	//int32 width, height;
	//int32 depth;
	//int32 bpp;	// bytes per pixel // orig struct pos
	//int32 stride;
	//uint8* pixels;
	//uint8* palette;
	int32 bpp;	// bytes per pixel (app field, not in gta3, moved in the end for RwImage)

	static int32 numAllocated;

	static RtImage* create(int32 width, int32 height, int32 depth);
	void destroy(void);
	void allocate(void);
	void free(void);
	void setPixels(uint8* pixels);
	void setPixelsDXT(int32 type, uint8* pixels);
	void setPalette(uint8* palette);
	void compressPalette(void);	// turn 8 bit into 4 bit if possible
	bool32 hasAlpha(void);
	void convertTo32(void);
	void palettize(int32 depth);
	void unpalettize(bool forceAlpha = false);
	void makeMask(void);
	void applyMask(RtImage* mask);
	void removeMask(void);
	RtImage* extractMask(void);

	static void setSearchPath(const char*);
	static void printSearchPath(void);
	static char* getFilename(const char*);
	static RtImage* read(const char* imageName);
	static RtImage* readMasked(const char* imageName, const char* maskName);


	typedef RtImage* (*fileRead)(const char* afilename);
	typedef void (*fileWrite)(RtImage* image, const char* filename);
	static bool32 registerFileFormat(const char* ext, fileRead read, fileWrite write);


#ifndef RWPUBLIC
	static void registerModule(void);
#endif
};

//Image* readTGA(const char* filename);
//void writeTGA(Image* image, const char* filename);
// conv like RtImage* RtTGAImageRead(const char* imageName); // todo
//Image* readBMP(const char* filename);
//void writeBMP(Image* image, const char* filename);
// conv like RtImage* RtBMPImageRead(const char* imageName); // todo

RtImage* readPNG(const char* filename);
void writePNG(RtImage* image, const char* filename);
RtImage* RtPNGImageRead(const char* imageName);