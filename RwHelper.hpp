#pragma once
// by MaZaHaKa and AAP
#include "RenderWare.h"
#include "lodepng/lodepng.h" // https://github.com/aap/librw/tree/master/src/lodepng
#include <fstream>
#define null NULL

uint8_t* getFileContents(const char* name, uint32_t* len)
{
	if (!len) { return null; }
	std::ifstream file(name, std::ios::binary | std::ios::ate);
	if (!file.is_open()) { *len = 0; return null; }

	std::streamsize fileSize = file.tellg(); // размер файла
	file.seekg(0, std::ios::beg); // в начало файла

	uint8_t* buffer = new uint8_t[fileSize];
	if (!file.read((char*)(buffer), fileSize)) { delete[] buffer; *len = 0; return null; }
	*len = (uint32_t)(fileSize);
	return buffer;
}

void expandPal4_BE(uint8_t* dst, uint32_t dststride, uint8_t* src, uint32_t srcstride, int32_t w, int32_t h);

RwImage* RtRwImageReadPNG(const char* filename)
{
	RwImage* image = null;
	uint32_t length;
	uint8_t* data = getFileContents(filename, &length);
	//assert(data != null);

	LodePNGState state;
	lodepng_state_init(&state);
	uint8_t* raw = null;
	uint32_t w, h;

	// First try: decode without conversion to see if we understand the format
	state.decoder.color_convert = 0;
	uint32_t error = lodepng_decode(&raw, &w, &h, &state, data, length);
	if (error) { return null; }

	if (state.info_raw.bitdepth == 4 && state.info_raw.colortype == LCT_PALETTE) {

		image = RwImageCreate(w, h, 4);
		RwImageAllocatePixels(image);
		memcpy(image->palette, state.info_raw.palette, state.info_raw.palettesize * 4);
		expandPal4_BE(image->cpPixels, image->stride, raw, w / 2, w, h);
	}
	else if (state.info_raw.bitdepth == 8) {
		switch (state.info_raw.colortype) {
		case LCT_PALETTE:
			image = RwImageCreate(w, h, state.info_raw.palettesize <= 16 ? 4 : 8);
			RwImageAllocatePixels(image);
			memcpy(image->palette, state.info_raw.palette, state.info_raw.palettesize * 4);
			memcpy(image->cpPixels, raw, w * h);
			break;
		case LCT_RGB:
			image = RwImageCreate(w, h, 24);
			RwImageAllocatePixels(image);
			memcpy(image->cpPixels, raw, w * h * 3);
			break;
		default:
			// Second try: just load as 32 bit
			free(raw);
			lodepng_state_init(&state);
			error = lodepng_decode(&raw, &w, &h, &state, data, length);
			if (error) { return null; }
			// fall through
		case LCT_RGBA:
			image = RwImageCreate(w, h, 32);
			RwImageAllocatePixels(image);
			memcpy(image->cpPixels, raw, w * h * 4);
			break;
		}
	}

	free(raw);	// TODO: maybe override lodepng allocator

	return image;
}

// dbg
/*RwTexture* GetRwTextureFromPNG(const char* filename)
{
	RwTexture* pTexOut = null;
	RwImage* image = RtRwImageReadPNG(filename);
	if (!image) { return null; }
	RwInt32 width, height, depth, flags;
	RwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	bool is888 = (depth == 8 * 3);
	RwRaster* raster = RwRasterCreate(width, height, depth, flags);
	//RwRaster* raster = RwRasterCreate(width, height, depth, rwRASTERTYPETEXTURE | (is888 ? rwRASTERFORMAT888 : rwRASTERFORMAT8888));
	//RwUInt8* dstPixels = (RwUInt8*)RwRasterLock(raster, 0, rwRASTERLOCKREADWRITE); // raster lock 4 modify
	RwRasterSetFromImage(raster, image);
	RwImageDestroy(image);
	//RwRasterUnlock(raster);
	pTexOut = RwTextureCreate(raster);
	RwTextureSetFilterMode(pTexOut, rwFILTERLINEAR);
	return pTexOut;
}*/

RwTexture* LoadTextureFromPNGFile(const char* filename, const char* tex_name = null) // or check CPlayerSkin::GetSkinTexture
{
	RwImage* image = RtRwImageReadPNG(filename);
	if (!image) { return null; }
	RwInt32 width, height, depth, flags;
	RwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	RwRaster* raster = RwRasterCreate(width, height, depth, flags);
	RwRasterSetFromImage(raster, image);
	RwTexture* myTexture = RwTextureCreate(raster);
	if (tex_name) { RwTextureSetName(myTexture, tex_name); }
	RwImageDestroy(image);
	return myTexture;
};




//RwImage* image = (RwImage*)RtPNGImageRead(filename);
//if (!image) { return null; }
//RwRasterSetFromImage(pTex->raster, image);

//memcpy(newraster->cpPixels, cppixels, width* height);
//for (RwInt32 y = 0; y < height; ++y) {
//	//memcpy(newraster->cpPixels + y * rowSize, cppixels + y * rowSize, rowSize);
//	memcpy(newraster->cpPixels + y * rowSize, &g, rowSize);
//}


// check RenderWare.cpp
RwTexture* CopyTexture(RwTexture* pTex) // mb use RwImageCopy
{
	if (!pTex) { return pTex; }
	RwTexture* pTexOut = null;

	RwInt32 width = pTex->raster->width;
	RwInt32 height = pTex->raster->height;
	RwInt32 depth = pTex->raster->depth;
	int32_t bpp = depth / 8; // bytes per pixel
	bool is888 = (depth == 8 * 3);

	if (!((depth == 8 * 3) || (depth == 8 * 4))) { printf("NOT 888/8888 format!\n"); return pTex; } // not 888 or 8888, mb DXT or etc.. todo
	if (RwRasterLock(pTex->raster, 0, rwRASTERLOCKREADWRITE | rwRASTERLOCKNOFETCH)) {
		//RwUInt8* srcPixels = (RwUInt8*)RwRasterLock(pTex->raster, 0, rwRASTERLOCKREAD); // srcPixels null
		RwUInt8* srcPixels = pTex->raster->cpPixels; // srcPixels !null

		RwRaster* newraster = RwRasterCreate(width, height, depth, rwRASTERTYPETEXTURE | (is888 ? rwRASTERFORMAT888 : rwRASTERFORMAT8888));
		RwUInt8* dstPixels = (RwUInt8*)RwRasterLock(newraster, 0, rwRASTERLOCKREADWRITE); // raster lock 4 modify
		//RwUInt8* dstPixels = newraster->cpPixels; // srcPixels be null

		//newraster->cpPixels = (RwUInt8*)malloc(newWidth * newHeight * (depth / 8));
		if (srcPixels && dstPixels) {
			// Копируем данные пикселей
			//for (int y = 0; y < height; ++y) {
			//	for (int x = 0; x < width; ++x) {
			//		for (int i = 0; i < bpp; ++i) {
			//			// Вычисляем индекс для исходной и новой текстуры
			//			int srcIndex = (y * width + x) * bpp + i;
			//			int dstIndex = (y * width + x) * bpp + i;
			//			// Копируем пиксель
			//			dstPixels[dstIndex] = srcPixels[srcIndex];
			//		}
			//	}
			//}
			RwInt32 dataSize = width * height * bpp; // Общий размер данных пикселей
			memcpy(dstPixels, srcPixels, dataSize);  // Копируем все данные сразу
		}
		//if (!srcPixels) { MessageBoxA(HWND_DESKTOP, "srcPixels", "srcPixels", MB_SYSTEMMODAL | MB_ICONWARNING); }
		//if (!dstPixels) { MessageBoxA(HWND_DESKTOP, "dstPixels", "dstPixels", MB_SYSTEMMODAL | MB_ICONWARNING); }

		RwRasterUnlock(newraster);
		pTexOut = RwTextureCreate(newraster);
		RwTextureSetFilterMode(pTexOut, rwFILTERLINEAR);

		// Unlock the raster
		RwRasterUnlock(pTex->raster);
	}
	return pTexOut;
}

// librwgta radaredit
//void
//copyImage(rw::Image* dst, int dstx, int dsty, rw::Image* src, int srcx, int srcy, int w, int h)
//{
//	int i;
//	if (dst->depth != src->depth)
//		return;
//	uint8* dstp = &dst->pixels[dsty * dst->stride + dstx * dst->bpp];
//	uint8* srcp = &src->pixels[srcy * src->stride + srcx * src->bpp];
//
//	for (i = 0; i < h; i++) {
//		memcpy(dstp, srcp, w * src->bpp);
//		dstp += dst->stride;
//		srcp += src->stride;
//	}
//}

RwTexture* LimitHeightWidthTexture(RwTexture* pTex, int maxW = 512, int maxH = 512)
{
	if (!pTex) { return pTex; }
	return CopyTexture(pTex);
	RwTexture* pTexOut = null;

	RwInt32 width = pTex->raster->width;
	RwInt32 height = pTex->raster->height;
	RwInt32 depth = pTex->raster->depth;
	int32_t bpp = depth / 8; // bytes per pixel
	bool is888 = (depth == 8 * 3);
	if (width <= maxW && height <= maxH) { return pTex; }
	if (!((depth == 8 * 3) || (depth == 8 * 4))) { printf("NOT 888/8888 format!\n"); return pTex; } // not 888 or 8888, mb DXT or etc.. todo


	if (RwRasterLock(pTex->raster, 0, rwRASTERLOCKREADWRITE | rwRASTERLOCKNOFETCH)) {
		RwUInt8* cppixels = pTex->raster->cpPixels;

		// Рассчитываем новые размеры с сохранением пропорций
		float aspectRatio = (float)width / (float)height;
		RwInt32 newWidth = maxW;
		RwInt32 newHeight = maxH;

		if (width > height) {
			newHeight = (RwInt32)(maxW / aspectRatio);
			if (newHeight > maxH) {
				newHeight = maxH;
				newWidth = (RwInt32)(maxH * aspectRatio);
			}
		}
		else {
			newWidth = (RwInt32)(maxH * aspectRatio);
			if (newWidth > maxW) {
				newWidth = maxW;
				newHeight = (RwInt32)(maxW / aspectRatio);
			}
		}


		RwRaster* newraster = RwRasterCreate(newWidth, newHeight, depth, rwRASTERTYPETEXTURE | (is888 ? rwRASTERFORMAT888 : rwRASTERFORMAT8888));
		//RwUInt8* srcPixels = (RwUInt8*)RwRasterLock(pTex->raster, 0, rwRASTERLOCKREAD); // srcPixels null
		RwUInt8* srcPixels = pTex->raster->cpPixels; // srcPixels !null
		RwUInt8* dstPixels = (RwUInt8*)RwRasterLock(newraster, 0, rwRASTERLOCKREADWRITE); // need for new RwRasterCreate

		//newraster->cpPixels = (RwUInt8*)malloc(newWidth * newHeight * (depth / 8));
		// алгоритм масштабирования (билинейное уменьшение)
		for (int y = 0; y < newHeight; ++y) {
			for (int x = 0; x < newWidth; ++x) {
				int srcX = x * width / newWidth;
				int srcY = y * height / newHeight;

				for (int i = 0; i < bpp; ++i) {
					uint32_t dindex = (y * newWidth + x) * (bpp)+i;
					uint32_t sindex = (srcY * width + srcX) * (bpp)+i;
					dstPixels[dindex] = srcPixels[sindex];
				}
			}
		}

		RwRasterUnlock(newraster);
		pTexOut = RwTextureCreate(newraster);
		RwTextureSetFilterMode(pTexOut, rwFILTERLINEAR);

		// Unlock the raster
		RwRasterUnlock(pTex->raster);
	}
	return pTexOut;
}





//--------------------------------------------------------------------------

// librw raster.cpp
void
conv_RGBA8888_from_RGBA8888(uint8_t* out, uint8_t* in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void
conv_BGRA8888_from_RGBA8888(uint8_t* out, uint8_t* in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
	out[3] = in[3];
}

void
conv_RGBA8888_from_RGB888(uint8_t* out, uint8_t* in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 0xFF;
}

void
conv_BGRA8888_from_RGB888(uint8_t* out, uint8_t* in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
	out[3] = 0xFF;
}

void
conv_RGB888_from_RGB888(uint8_t* out, uint8_t* in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void
conv_BGR888_from_RGB888(uint8_t* out, uint8_t* in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
}

void
conv_ARGB1555_from_ARGB1555(uint8_t* out, uint8_t* in)
{
	out[0] = in[0];
	out[1] = in[1];
}

void
conv_ARGB1555_from_RGB555(uint8_t* out, uint8_t* in)
{
	out[0] = in[0];
	out[1] = in[1] | 0x80;
}

void
conv_RGBA5551_from_ARGB1555(uint8_t* out, uint8_t* in)
{
	uint32_t r, g, b, a;
	a = (in[1] >> 7) & 1;
	r = (in[1] >> 2) & 0x1F;
	g = (in[1] & 3) << 3 | ((in[0] >> 5) & 7);
	b = in[0] & 0x1F;
	out[0] = a | b << 1 | g << 6;
	out[1] = g >> 2 | r << 3;
}

void
conv_ARGB1555_from_RGBA5551(uint8_t* out, uint8_t* in)
{
	uint32_t r, g, b, a;
	a = in[0] & 1;
	b = (in[0] >> 1) & 0x1F;
	g = (in[1] & 7) << 2 | ((in[0] >> 6) & 3);
	r = (in[1] >> 3) & 0x1F;
	out[0] = b | g << 5;
	out[1] = g >> 3 | r << 2 | a << 7;
}

void
conv_RGBA8888_from_ARGB1555(uint8_t* out, uint8_t* in)
{
	uint32_t r, g, b, a;
	a = (in[1] >> 7) & 1;
	r = (in[1] >> 2) & 0x1F;
	g = (in[1] & 3) << 3 | ((in[0] >> 5) & 7);
	b = in[0] & 0x1F;
	out[0] = r * 0xFF / 0x1f;
	out[1] = g * 0xFF / 0x1f;
	out[2] = b * 0xFF / 0x1f;
	out[3] = a * 0xFF;
}

void
conv_ABGR1555_from_ARGB1555(uint8_t* out, uint8_t* in)
{
	uint32_t r, b;
	r = (in[1] >> 2) & 0x1F;
	b = in[0] & 0x1F;
	out[1] = (in[1] & 0x83) | b << 2;
	out[0] = (in[0] & 0xE0) | r;
}

void
expandPal4(uint8_t* dst, uint32_t dststride, uint8_t* src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w / 2; x++) {
			dst[y * dststride + x * 2 + 0] = src[y * srcstride + x] & 0xF;
			dst[y * dststride + x * 2 + 1] = src[y * srcstride + x] >> 4;
		}
}
void
compressPal4(uint8_t* dst, uint32_t dststride, uint8_t* src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w / 2; x++)
			dst[y * dststride + x] = src[y * srcstride + x * 2 + 0] | src[y * srcstride + x * 2 + 1] << 4;
}

void
expandPal4_BE(uint8_t* dst, uint32_t dststride, uint8_t* src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w / 2; x++) {
			dst[y * dststride + x * 2 + 1] = src[y * srcstride + x] & 0xF;
			dst[y * dststride + x * 2 + 0] = src[y * srcstride + x] >> 4;
		}
}
void
compressPal4_BE(uint8_t* dst, uint32_t dststride, uint8_t* src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w / 2; x++)
			dst[y * dststride + x] = src[y * srcstride + x * 2 + 1] | src[y * srcstride + x * 2 + 0] << 4;
}

void
copyPal8(uint8_t* dst, uint32_t dststride, uint8_t* src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			dst[y * dststride + x] = src[y * srcstride + x];
}

void
decompressDXT1(uint8_t* adst, int32_t w, int32_t h, uint8_t* src)
{
	/* j loops through old texels
	 * x and y loop through new texels */
	int32_t x = 0, y = 0;
	uint32_t c[4][4];
	uint8_t idx[16];
	uint8_t(*dst)[4] = (uint8_t(*)[4])adst;
	for (int32_t j = 0; j < w * h / 2; j += 8) {
		/* calculate colors */
		uint32_t col0 = *((uint16_t*)&src[j + 0]);
		uint32_t col1 = *((uint16_t*)&src[j + 2]);
		c[0][0] = ((col0 >> 11) & 0x1F) * 0xFF / 0x1F;
		c[0][1] = ((col0 >> 5) & 0x3F) * 0xFF / 0x3F;
		c[0][2] = (col0 & 0x1F) * 0xFF / 0x1F;
		c[0][3] = 0xFF;

		c[1][0] = ((col1 >> 11) & 0x1F) * 0xFF / 0x1F;
		c[1][1] = ((col1 >> 5) & 0x3F) * 0xFF / 0x3F;
		c[1][2] = (col1 & 0x1F) * 0xFF / 0x1F;
		c[1][3] = 0xFF;
		if (col0 > col1) {
			c[2][0] = (2 * c[0][0] + 1 * c[1][0]) / 3;
			c[2][1] = (2 * c[0][1] + 1 * c[1][1]) / 3;
			c[2][2] = (2 * c[0][2] + 1 * c[1][2]) / 3;
			c[2][3] = 0xFF;

			c[3][0] = (1 * c[0][0] + 2 * c[1][0]) / 3;
			c[3][1] = (1 * c[0][1] + 2 * c[1][1]) / 3;
			c[3][2] = (1 * c[0][2] + 2 * c[1][2]) / 3;
			c[3][3] = 0xFF;
		}
		else {
			c[2][0] = (c[0][0] + c[1][0]) / 2;
			c[2][1] = (c[0][1] + c[1][1]) / 2;
			c[2][2] = (c[0][2] + c[1][2]) / 2;
			c[2][3] = 0xFF;

			c[3][0] = 0x00;
			c[3][1] = 0x00;
			c[3][2] = 0x00;
			c[3][3] = 0x00;
		}

		/* make index list */
		uint32_t indices = *((uint32_t*)&src[j + 4]);
		for (int32_t k = 0; k < 16; k++) {
			idx[k] = indices & 0x3;
			indices >>= 2;
		}

		/* write bytes */
		for (uint32_t l = 0; l < 4; l++)
			for (uint32_t k = 0; k < 4; k++) {
				dst[(y + l) * w + x + k][0] = c[idx[l * 4 + k]][0];
				dst[(y + l) * w + x + k][1] = c[idx[l * 4 + k]][1];
				dst[(y + l) * w + x + k][2] = c[idx[l * 4 + k]][2];
				dst[(y + l) * w + x + k][3] = c[idx[l * 4 + k]][3];
			}
		x += 4;
		if (x >= w) {
			y += 4;
			x = 0;
		}
	}
}

void
decompressDXT3(uint8_t* adst, int32_t w, int32_t h, uint8_t* src)
{
	/* j loops through old texels
	 * x and y loop through new texels */
	int32_t x = 0, y = 0;
	uint32_t c[4][4];
	uint8_t idx[16];
	uint8_t a[16];
	uint8_t(*dst)[4] = (uint8_t(*)[4])adst;
	for (int32_t j = 0; j < w * h; j += 16) {
		/* calculate colors */
		uint32_t col0 = *((uint16_t*)&src[j + 8]);
		uint32_t col1 = *((uint16_t*)&src[j + 10]);
		c[0][0] = ((col0 >> 11) & 0x1F) * 0xFF / 0x1F;
		c[0][1] = ((col0 >> 5) & 0x3F) * 0xFF / 0x3F;
		c[0][2] = (col0 & 0x1F) * 0xFF / 0x1F;

		c[1][0] = ((col1 >> 11) & 0x1F) * 0xFF / 0x1F;
		c[1][1] = ((col1 >> 5) & 0x3F) * 0xFF / 0x3F;
		c[1][2] = (col1 & 0x1F) * 0xFF / 0x1F;

		c[2][0] = (2 * c[0][0] + 1 * c[1][0]) / 3;
		c[2][1] = (2 * c[0][1] + 1 * c[1][1]) / 3;
		c[2][2] = (2 * c[0][2] + 1 * c[1][2]) / 3;

		c[3][0] = (1 * c[0][0] + 2 * c[1][0]) / 3;
		c[3][1] = (1 * c[0][1] + 2 * c[1][1]) / 3;
		c[3][2] = (1 * c[0][2] + 2 * c[1][2]) / 3;

		/* make index list */
		uint32_t indices = *((uint32_t*)&src[j + 12]);
		for (int32_t k = 0; k < 16; k++) {
			idx[k] = indices & 0x3;
			indices >>= 2;
		}
		uint64 alphas = *((uint64*)&src[j + 0]);
		for (int32_t k = 0; k < 16; k++) {
			a[k] = (alphas & 0xF) * 17;
			alphas >>= 4;
		}

		/* write bytes */
		for (uint32_t l = 0; l < 4; l++)
			for (uint32_t k = 0; k < 4; k++) {
				dst[(y + l) * w + x + k][0] = c[idx[l * 4 + k]][0];
				dst[(y + l) * w + x + k][1] = c[idx[l * 4 + k]][1];
				dst[(y + l) * w + x + k][2] = c[idx[l * 4 + k]][2];
				dst[(y + l) * w + x + k][3] = a[l * 4 + k];
			}
		x += 4;
		if (x >= w) {
			y += 4;
			x = 0;
		}
	}
}

void
decompressDXT5(uint8_t* adst, int32_t w, int32_t h, uint8_t* src)
{
	/* j loops through old texels
	 * x and y loop through new texels */
	int32_t x = 0, y = 0;
	uint32_t c[4][4];
	uint32_t a[8];
	uint8_t idx[16];
	uint8_t aidx[16];
	uint8_t(*dst)[4] = (uint8_t(*)[4])adst;
	for (int32_t j = 0; j < w * h; j += 16) {
		/* calculate colors */
		uint32_t col0 = *((uint16_t*)&src[j + 8]);
		uint32_t col1 = *((uint16_t*)&src[j + 10]);
		c[0][0] = ((col0 >> 11) & 0x1F) * 0xFF / 0x1F;
		c[0][1] = ((col0 >> 5) & 0x3F) * 0xFF / 0x3F;
		c[0][2] = (col0 & 0x1F) * 0xFF / 0x1F;

		c[1][0] = ((col1 >> 11) & 0x1F) * 0xFF / 0x1F;
		c[1][1] = ((col1 >> 5) & 0x3F) * 0xFF / 0x3F;
		c[1][2] = (col1 & 0x1F) * 0xFF / 0x1F;
		if (col0 > col1) {
			c[2][0] = (2 * c[0][0] + 1 * c[1][0]) / 3;
			c[2][1] = (2 * c[0][1] + 1 * c[1][1]) / 3;
			c[2][2] = (2 * c[0][2] + 1 * c[1][2]) / 3;

			c[3][0] = (1 * c[0][0] + 2 * c[1][0]) / 3;
			c[3][1] = (1 * c[0][1] + 2 * c[1][1]) / 3;
			c[3][2] = (1 * c[0][2] + 2 * c[1][2]) / 3;
		}
		else {
			c[2][0] = (c[0][0] + c[1][0]) / 2;
			c[2][1] = (c[0][1] + c[1][1]) / 2;
			c[2][2] = (c[0][2] + c[1][2]) / 2;

			c[3][0] = 0x00;
			c[3][1] = 0x00;
			c[3][2] = 0x00;
		}

		a[0] = src[j + 0];
		a[1] = src[j + 1];
		if (a[0] > a[1]) {
			a[2] = (6 * a[0] + 1 * a[1]) / 7;
			a[3] = (5 * a[0] + 2 * a[1]) / 7;
			a[4] = (4 * a[0] + 3 * a[1]) / 7;
			a[5] = (3 * a[0] + 4 * a[1]) / 7;
			a[6] = (2 * a[0] + 5 * a[1]) / 7;
			a[7] = (1 * a[0] + 6 * a[1]) / 7;
		}
		else {
			a[2] = (4 * a[0] + 1 * a[1]) / 5;
			a[3] = (3 * a[0] + 2 * a[1]) / 5;
			a[4] = (2 * a[0] + 3 * a[1]) / 5;
			a[5] = (1 * a[0] + 4 * a[1]) / 5;
			a[6] = 0;
			a[7] = 0xFF;
		}

		/* make index list */
		uint32_t indices = *((uint32_t*)&src[j + 12]);
		for (int32_t k = 0; k < 16; k++) {
			idx[k] = indices & 0x3;
			indices >>= 2;
		}
		// only 6 indices
		uint64 alphas = *((uint64*)&src[j + 2]);
		for (int32_t k = 0; k < 16; k++) {
			aidx[k] = alphas & 0x7;
			alphas >>= 3;
		}

		/* write bytes */
		for (uint32_t l = 0; l < 4; l++)
			for (uint32_t k = 0; k < 4; k++) {
				dst[(y + l) * w + x + k][0] = c[idx[l * 4 + k]][0];
				dst[(y + l) * w + x + k][1] = c[idx[l * 4 + k]][1];
				dst[(y + l) * w + x + k][2] = c[idx[l * 4 + k]][2];
				dst[(y + l) * w + x + k][3] = a[aidx[l * 4 + k]];
			}
		x += 4;
		if (x >= w) {
			y += 4;
			x = 0;
		}
	}
}


void
setPixelsDXT(RwImage* image, int32_t type, uint8_t* pixels)
{
	if (!image) { return; }
	switch (type) {
	case 1:
		decompressDXT1(image->cpPixels, image->width, image->height, pixels);
		break;
	case 3:
		decompressDXT3(image->cpPixels, image->width, image->height, pixels);
		break;
	case 5:
		decompressDXT5(image->cpPixels, image->width, image->height, pixels);
		break;
	}
}