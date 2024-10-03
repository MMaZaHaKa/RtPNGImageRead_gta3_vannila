#pragma once
// by MaZaHaKa and AAP
#include "RtImage.h"
#include "lodepng/lodepng.h"
#include <fstream>
#define null NULL


uint8_t* getFileContents(const char* name, uint32_t* len)
{
	if (!len) { return null; }
	std::ifstream file(name, std::ios::binary | std::ios::ate);
	if (!file.is_open()) { *len = 0; return null; }

	std::streamsize fileSize = file.tellg(); // Получаем размер файла
	file.seekg(0, std::ios::beg); // Возвращаемся в начало файла

	uint8_t* buffer = new uint8_t[fileSize];
	if (!file.read((char*)(buffer), fileSize)) { delete[] buffer; *len = 0; return null; }
	*len = (uint32_t)(fileSize);
	return buffer;
}

// librw raster.cpp
void
conv_RGBA8888_from_RGBA8888(uint8 *out, uint8 *in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void
conv_BGRA8888_from_RGBA8888(uint8 *out, uint8 *in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
	out[3] = in[3];
}

void
conv_RGBA8888_from_RGB888(uint8 *out, uint8 *in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 0xFF;
}

void
conv_BGRA8888_from_RGB888(uint8 *out, uint8 *in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
	out[3] = 0xFF;
}

void
conv_RGB888_from_RGB888(uint8 *out, uint8 *in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void
conv_BGR888_from_RGB888(uint8 *out, uint8 *in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
}

void
conv_ARGB1555_from_ARGB1555(uint8 *out, uint8 *in)
{
	out[0] = in[0];
	out[1] = in[1];
}

void
conv_ARGB1555_from_RGB555(uint8 *out, uint8 *in)
{
	out[0] = in[0];
	out[1] = in[1] | 0x80;
}

void
conv_RGBA5551_from_ARGB1555(uint8 *out, uint8 *in)
{
	uint32 r, g, b, a;
	a = (in[1]>>7) & 1;
	r = (in[1]>>2) & 0x1F;
	g = (in[1]&3)<<3 | ((in[0]>>5)&7);
	b = in[0] & 0x1F;
	out[0] = a | b<<1 | g<<6;
	out[1] = g>>2 | r<<3;
}

void
conv_ARGB1555_from_RGBA5551(uint8 *out, uint8 *in)
{
	uint32 r, g, b, a;
	a = in[0] & 1;
	b = (in[0]>>1) & 0x1F;
	g = (in[1]&7)<<2 | ((in[0]>>6)&3);
	r = (in[1]>>3) & 0x1F;
	out[0] = b | g<<5;
	out[1] = g>>3 | r<<2 | a<<7;
}

void
conv_RGBA8888_from_ARGB1555(uint8 *out, uint8 *in)
{
	uint32 r, g, b, a;
	a = (in[1]>>7) & 1;
	r = (in[1]>>2) & 0x1F;
	g = (in[1]&3)<<3 | ((in[0]>>5)&7);
	b = in[0] & 0x1F;
	out[0] = r*0xFF/0x1f;
	out[1] = g*0xFF/0x1f;
	out[2] = b*0xFF/0x1f;
	out[3] = a*0xFF;
}

void
conv_ABGR1555_from_ARGB1555(uint8 *out, uint8 *in)
{
	uint32 r, b;
	r = (in[1]>>2) & 0x1F;
	b = in[0] & 0x1F;
	out[1] = (in[1]&0x83) | b<<2;
	out[0] = (in[0]&0xE0) | r;
}

void
expandPal4(uint8 *dst, uint32 dststride, uint8 *src, uint32 srcstride, int32 w, int32 h)
{
	int32 x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++){
			dst[y*dststride + x*2 + 0] = src[y*srcstride + x] & 0xF;
			dst[y*dststride + x*2 + 1] = src[y*srcstride + x] >> 4;
		}
}
void
compressPal4(uint8 *dst, uint32 dststride, uint8 *src, uint32 srcstride, int32 w, int32 h)
{
	int32 x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++)
			dst[y*dststride + x] = src[y*srcstride + x*2 + 0] | src[y*srcstride + x*2 + 1] << 4;
}

void
expandPal4_BE(uint8 *dst, uint32 dststride, uint8 *src, uint32 srcstride, int32 w, int32 h)
{
	int32 x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++){
			dst[y*dststride + x*2 + 1] = src[y*srcstride + x] & 0xF;
			dst[y*dststride + x*2 + 0] = src[y*srcstride + x] >> 4;
		}
}
void
compressPal4_BE(uint8 *dst, uint32 dststride, uint8 *src, uint32 srcstride, int32 w, int32 h)
{
	int32 x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++)
			dst[y*dststride + x] = src[y*srcstride + x*2 + 1] | src[y*srcstride + x*2 + 0] << 4;
}

void
copyPal8(uint8 *dst, uint32 dststride, uint8 *src, uint32 srcstride, int32 w, int32 h)
{
	int32 x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w; x++)
			dst[y*dststride + x] = src[y*srcstride + x];
}

//-------------------------------rtimage custom
// rwobjects.h
int32 RtImage::numAllocated;
RtImage*
readPNG(const char *filename)
{
	RtImage *image = null;
	uint32 length;
	uint8 *data = getFileContents(filename, &length);
	//assert(data != null);

	LodePNGState state;
	lodepng_state_init(&state);
	uint8 *raw = null;
	uint32 w, h;

	// First try: decode without conversion to see if we understand the format
	state.decoder.color_convert = 0;
	uint32 error = lodepng_decode(&raw, &w, &h, &state, data, length);
	if(error){
		//RWERROR((ERR_GENERAL, lodepng_error_text(error)));
		return null;
	}

	if(state.info_raw.bitdepth == 4 && state.info_raw.colortype == LCT_PALETTE){
		image = RtImage::create(w, h, 4);
		image->allocate();
		memcpy(image->palette, state.info_raw.palette, state.info_raw.palettesize*4);
		expandPal4_BE(image->cpPixels, image->stride, raw, w/2, w, h);
	}else if(state.info_raw.bitdepth == 8){
		switch(state.info_raw.colortype){
		case LCT_PALETTE:
			image = RtImage::create(w, h, state.info_raw.palettesize <= 16 ? 4 : 8);
			image->allocate();
			memcpy(image->palette, state.info_raw.palette, state.info_raw.palettesize*4);
			memcpy(image->cpPixels, raw, w*h);
			break;
		case LCT_RGB:
			image = RtImage::create(w, h, 24);
			image->allocate();
			memcpy(image->cpPixels, raw, w*h*3);
			break;
		default:
			// Second try: just load as 32 bit
			free(raw);
			lodepng_state_init(&state);
			error = lodepng_decode(&raw, &w, &h, &state, data, length);
			if(error){
				//RWERROR((ERR_GENERAL, lodepng_error_text(error)));
				return null;
			}
			// fall through
		case LCT_RGBA:
			image = RtImage::create(w, h, 32);
			image->allocate();
			memcpy(image->cpPixels, raw, w*h*4);
			break;
		}
	}

	free(raw);	// TODO: maybe override lodepng allocator

	return image;
}

RtImage*
RtPNGImageRead(const char* imageName)
{
#ifndef _WIN32
	RwImage* image;
	char* r = casepath(imageName);
	if (r) {
		image = rw::readPNG(r);
		free(r);
	}
	else {
		image = rw::readPNG(imageName);
	}
	return image;

#else
	return readPNG(imageName);
#endif
}

RtImage*
RtImage::create(int32 width, int32 height, int32 depth)
{
	//RtImage* img = (RtImage*)rwMalloc(sizeof(RtImage), MEMDUR_EVENT | ID_IMAGE); // engine todo? mazahaka
	RtImage* img = (RtImage*)malloc(sizeof(RtImage));
	if (img == null) {
		//RWERROR((ERR_ALLOC, sizeof(Image)));
		return null;
	}
	numAllocated++;
	img->flags = 0;
	img->width = width;
	img->height = height;
	img->depth = depth;
	img->bpp = depth < 8 ? 1 : depth / 8;
	img->stride = 0;
	img->cpPixels = null;
	img->palette = null;
	return img;
}


void
RtImage::allocate(void)
{
	if (this->cpPixels == null) {
		this->stride = this->width * this->bpp;
		//this->pixels = rwNewT(uint8, this->stride * this->height, MEMDUR_EVENT | ID_IMAGE); // видеопамять?
		this->cpPixels = (uint8*)malloc(this->stride * this->height); // ram :/
		this->flags |= 1;
	}
	if (this->palette == null) {
		if (this->depth == 4 || this->depth == 8)
			//this->palette = rwNewT(uint8, (1 << this->depth) * 4, MEMDUR_EVENT | ID_IMAGE);
			this->palette = (RwRGBA*)malloc((1 << this->depth) * 4);
		this->flags |= 2;
	}
}

void
RtImage::free(void)
{
	if (this->flags & 1) {
		//rwFree(this->pixels);
		delete (this->cpPixels);
		this->cpPixels = null;
	}
	if (this->flags & 2) {
		//rwFree(this->palette);
		delete (this->palette);
		this->palette = null;
	}
	this->flags = 0;
}

void
decompressDXT1(uint8 *adst, int32 w, int32 h, uint8 *src)
{
	/* j loops through old texels
	 * x and y loop through new texels */
	int32 x = 0, y = 0;
	uint32 c[4][4];
	uint8 idx[16];
	uint8 (*dst)[4] = (uint8(*)[4])adst;
	for(int32 j = 0; j < w*h/2; j += 8){
		/* calculate colors */
		uint32 col0 = *((uint16*)&src[j+0]);
		uint32 col1 = *((uint16*)&src[j+2]);
		c[0][0] = ((col0>>11) & 0x1F)*0xFF/0x1F;
		c[0][1] = ((col0>> 5) & 0x3F)*0xFF/0x3F;
		c[0][2] = ( col0      & 0x1F)*0xFF/0x1F;
		c[0][3] = 0xFF;

		c[1][0] = ((col1>>11) & 0x1F)*0xFF/0x1F;
		c[1][1] = ((col1>> 5) & 0x3F)*0xFF/0x3F;
		c[1][2] = ( col1      & 0x1F)*0xFF/0x1F;
		c[1][3] = 0xFF;
		if(col0 > col1){
			c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			c[2][2] = (2*c[0][2] + 1*c[1][2])/3;
			c[2][3] = 0xFF;

			c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
			c[3][3] = 0xFF;
		}else{
			c[2][0] = (c[0][0] + c[1][0])/2;
			c[2][1] = (c[0][1] + c[1][1])/2;
			c[2][2] = (c[0][2] + c[1][2])/2;
			c[2][3] = 0xFF;

			c[3][0] = 0x00;
			c[3][1] = 0x00;
			c[3][2] = 0x00;
			c[3][3] = 0x00;
		}

		/* make index list */
		uint32 indices = *((uint32*)&src[j+4]);
		for(int32 k = 0; k < 16; k++){
			idx[k] = indices & 0x3;
			indices >>= 2;
		}

		/* write bytes */
		for(uint32 l = 0; l < 4; l++)
			for(uint32 k = 0; k < 4; k++){
				dst[(y+l)*w + x+k][0] = c[idx[l*4+k]][0];
				dst[(y+l)*w + x+k][1] = c[idx[l*4+k]][1];
				dst[(y+l)*w + x+k][2] = c[idx[l*4+k]][2];
				dst[(y+l)*w + x+k][3] = c[idx[l*4+k]][3];
			}
		x += 4;
		if(x >= w){
			y += 4;
			x = 0;
		}
	}
}

void
decompressDXT3(uint8 *adst, int32 w, int32 h, uint8 *src)
{
	/* j loops through old texels
	 * x and y loop through new texels */
	int32 x = 0, y = 0;
	uint32 c[4][4];
	uint8 idx[16];
	uint8 a[16];
	uint8 (*dst)[4] = (uint8(*)[4])adst;
	for(int32 j = 0; j < w*h; j += 16){
		/* calculate colors */
		uint32 col0 = *((uint16*)&src[j+8]);
		uint32 col1 = *((uint16*)&src[j+10]);
		c[0][0] = ((col0>>11) & 0x1F)*0xFF/0x1F;
		c[0][1] = ((col0>> 5) & 0x3F)*0xFF/0x3F;
		c[0][2] = ( col0      & 0x1F)*0xFF/0x1F;

		c[1][0] = ((col1>>11) & 0x1F)*0xFF/0x1F;
		c[1][1] = ((col1>> 5) & 0x3F)*0xFF/0x3F;
		c[1][2] = ( col1      & 0x1F)*0xFF/0x1F;

		c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
		c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
		c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

		c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
		c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
		c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

		/* make index list */
		uint32 indices = *((uint32*)&src[j+12]);
		for(int32 k = 0; k < 16; k++){
			idx[k] = indices & 0x3;
			indices >>= 2;
		}
		uint64 alphas = *((uint64*)&src[j+0]);
		for(int32 k = 0; k < 16; k++){
			a[k] = (alphas & 0xF)*17;
			alphas >>= 4;
		}

		/* write bytes */
		for(uint32 l = 0; l < 4; l++)
			for(uint32 k = 0; k < 4; k++){
				dst[(y+l)*w + x+k][0] = c[idx[l*4+k]][0];
				dst[(y+l)*w + x+k][1] = c[idx[l*4+k]][1];
				dst[(y+l)*w + x+k][2] = c[idx[l*4+k]][2];
				dst[(y+l)*w + x+k][3] = a[l*4+k];
			}
		x += 4;
		if(x >= w){
			y += 4;
			x = 0;
		}
	}
}

void
decompressDXT5(uint8 *adst, int32 w, int32 h, uint8 *src)
{
	/* j loops through old texels
	 * x and y loop through new texels */
	int32 x = 0, y = 0;
	uint32 c[4][4];
	uint32 a[8];
	uint8 idx[16];
	uint8 aidx[16];
	uint8 (*dst)[4] = (uint8(*)[4])adst;
	for(int32 j = 0; j < w*h; j += 16){
		/* calculate colors */
		uint32 col0 = *((uint16*)&src[j+8]);
		uint32 col1 = *((uint16*)&src[j+10]);
		c[0][0] = ((col0>>11) & 0x1F)*0xFF/0x1F;
		c[0][1] = ((col0>> 5) & 0x3F)*0xFF/0x3F;
		c[0][2] = ( col0      & 0x1F)*0xFF/0x1F;

		c[1][0] = ((col1>>11) & 0x1F)*0xFF/0x1F;
		c[1][1] = ((col1>> 5) & 0x3F)*0xFF/0x3F;
		c[1][2] = ( col1      & 0x1F)*0xFF/0x1F;
		if(col0 > col1){
			c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

			c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
		}else{
			c[2][0] = (c[0][0] + c[1][0])/2;
			c[2][1] = (c[0][1] + c[1][1])/2;
			c[2][2] = (c[0][2] + c[1][2])/2;

			c[3][0] = 0x00;
			c[3][1] = 0x00;
			c[3][2] = 0x00;
		}

		a[0] = src[j+0];
		a[1] = src[j+1];
		if(a[0] > a[1]){
			a[2] = (6*a[0] + 1*a[1])/7;
			a[3] = (5*a[0] + 2*a[1])/7;
			a[4] = (4*a[0] + 3*a[1])/7;
			a[5] = (3*a[0] + 4*a[1])/7;
			a[6] = (2*a[0] + 5*a[1])/7;
			a[7] = (1*a[0] + 6*a[1])/7;
		}else{
			a[2] = (4*a[0] + 1*a[1])/5;
			a[3] = (3*a[0] + 2*a[1])/5;
			a[4] = (2*a[0] + 3*a[1])/5;
			a[5] = (1*a[0] + 4*a[1])/5;
			a[6] = 0;
			a[7] = 0xFF;
		}

		/* make index list */
		uint32 indices = *((uint32*)&src[j+12]);
		for(int32 k = 0; k < 16; k++){
			idx[k] = indices & 0x3;
			indices >>= 2;
		}
		// only 6 indices
		uint64 alphas = *((uint64*)&src[j+2]);
		for(int32 k = 0; k < 16; k++){
			aidx[k] = alphas & 0x7;
			alphas >>= 3;
		}

		/* write bytes */
		for(uint32 l = 0; l < 4; l++)
			for(uint32 k = 0; k < 4; k++){
				dst[(y+l)*w + x+k][0] = c[idx[l*4+k]][0];
				dst[(y+l)*w + x+k][1] = c[idx[l*4+k]][1];
				dst[(y+l)*w + x+k][2] = c[idx[l*4+k]][2];
				dst[(y+l)*w + x+k][3] = a[aidx[l*4+k]];
			}
		x += 4;
		if(x >= w){
			y += 4;
			x = 0;
		}
	}
}


void
RtImage::setPixelsDXT(int32 type, uint8* pixels)
{
	switch (type) {
	case 1:
		decompressDXT1(this->cpPixels, this->width, this->height, pixels);
		break;
	case 3:
		decompressDXT3(this->cpPixels, this->width, this->height, pixels);
		break;
	case 5:
		decompressDXT5(this->cpPixels, this->width, this->height, pixels);
		break;
	}
}