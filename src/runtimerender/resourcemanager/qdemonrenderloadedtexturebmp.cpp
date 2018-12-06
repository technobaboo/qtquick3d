/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
// ==========================================================
// BMP Loader and Writer
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Markus Loibl (markus.loibl@epost.de)
// - Martin Weber (martweb@gmx.net)
// - Herve Drolon (drolon@infonie.fr)
// - Michal Novotny (michal@etc.cz)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#include <QtDemonRuntimeRender/qdemonrenderloadedtexturefreeimagecompat.h>

// ----------------------------------------------------------
//   Constants + headers
// ----------------------------------------------------------

static const BYTE RLE_COMMAND = 0;
static const BYTE RLE_ENDOFLINE = 0;
static const BYTE RLE_ENDOFBITMAP = 1;
static const BYTE RLE_DELTA = 2;

static const BYTE BI_RGB = 0;
static const BYTE BI_RLE8 = 1;
static const BYTE BI_RLE4 = 2;
static const BYTE BI_BITFIELDS = 3;

// ----------------------------------------------------------

#ifdef _WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct tagBITMAPCOREHEADER
{
    DWORD bcSize;
    WORD bcWidth;
    WORD bcHeight;
    WORD bcPlanes;
    WORD bcBitCnt;
} BITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOOS2_1X_HEADER
{
    DWORD biSize;
    WORD biWidth;
    WORD biHeight;
    WORD biPlanes;
    WORD biBitCount;
} BITMAPINFOOS2_1X_HEADER, *PBITMAPINFOOS2_1X_HEADER;

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Internal functions
// ==========================================================

#ifdef FREEIMAGE_BIGENDIAN
static void SwapInfoHeader(BITMAPINFOHEADER *header)
{
    SwapLong(&header->biSize);
    SwapLong((DWORD *)&header->biWidth);
    SwapLong((DWORD *)&header->biHeight);
    SwapShort(&header->biPlanes);
    SwapShort(&header->biBitCount);
    SwapLong(&header->biCompression);
    SwapLong(&header->biSizeImage);
    SwapLong((DWORD *)&header->biXPelsPerMeter);
    SwapLong((DWORD *)&header->biYPelsPerMeter);
    SwapLong(&header->biClrUsed);
    SwapLong(&header->biClrImportant);
}

static void SwapCoreHeader(BITMAPCOREHEADER *header)
{
    SwapLong(&header->bcSize);
    SwapShort(&header->bcWidth);
    SwapShort(&header->bcHeight);
    SwapShort(&header->bcPlanes);
    SwapShort(&header->bcBitCnt);
}

static void SwapOS21XHeader(BITMAPINFOOS2_1X_HEADER *header)
{
    SwapLong(&header->biSize);
    SwapShort(&header->biWidth);
    SwapShort(&header->biHeight);
    SwapShort(&header->biPlanes);
    SwapShort(&header->biBitCount);
}

static void SwapFileHeader(BITMAPFILEHEADER *header)
{
    SwapShort(&header->bfType);
    SwapLong(&header->bfSize);
    SwapShort(&header->bfReserved1);
    SwapShort(&header->bfReserved2);
    SwapLong(&header->bfOffBits);
}
#endif

// --------------------------------------------------------------------------

/**
Load uncompressed image pixels for 1-, 4-, 8-, 16-, 24- and 32-bit dib
@param io FreeImage IO
@param handle FreeImage IO handle
@param dib Image to be loaded
@param height Image height
@param pitch Image pitch
@param bit_count Image bit-depth (1-, 4-, 8-, 16-, 24- or 32-bit)
*/
static void LoadPixelData(FreeImageIO *io, fi_handle handle, FIBITMAP *dib, int height, int pitch,
                          int bit_count)
{
    (void)bit_count;
    // Load pixel data
    // NB: height can be < 0 for BMP data
    if (height > 0) {
        io->read_proc((void *)FreeImage_GetBits(dib), height * pitch, 1, handle);
    } else {
        int positiveHeight = abs(height);
        for (int c = 0; c < positiveHeight; ++c) {
            io->read_proc((void *)FreeImage_GetScanLine(dib, positiveHeight - c - 1), pitch, 1,
                          handle);
        }
    }

// swap as needed
#ifdef FREEIMAGE_BIGENDIAN
    if (bit_count == 16) {
        for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
            WORD *pixel = (WORD *)FreeImage_GetScanLine(dib, y);
            for (unsigned x = 0; x < FreeImage_GetWidth(dib); x++) {
                SwapShort(pixel);
                pixel++;
            }
        }
    }
#endif

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
    if (bit_count == 24 || bit_count == 32) {
        for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
            BYTE *pixel = FreeImage_GetScanLine(dib, y);
            for (unsigned x = 0; x < FreeImage_GetWidth(dib); x++) {
                INPLACESWAP(pixel[0], pixel[2]);
                pixel += (bit_count >> 3);
            }
        }
    }
#endif
}

/**
Load image pixels for 4-bit RLE compressed dib
@param io FreeImage IO
@param handle FreeImage IO handle
@param width Image width
@param height Image height
@param dib Image to be loaded
@return Returns TRUE if successful, returns FALSE otherwise
*/
static BOOL LoadPixelDataRLE4(FreeImageIO *io, fi_handle handle, int width, int height,
                              FIBITMAP *dib)
{
    int status_byte = 0;
    BYTE second_byte = 0;
    int bits = 0;

    BYTE *pixels = nullptr; // temporary 8-bit buffer

    try {
        height = abs(height);

        pixels = (BYTE *)malloc(width * height * sizeof(BYTE));
        if (!pixels)
            throw(1);
        memset(pixels, 0, width * height * sizeof(BYTE));

        BYTE *q = pixels;
        BYTE *end = pixels + height * width;

        for (int scanline = 0; scanline < height;) {
            if (q < pixels || q >= end) {
                break;
            }
            if (io->read_proc(&status_byte, sizeof(BYTE), 1, handle) != 1) {
                throw(1);
            }
            if (status_byte != 0) {
                status_byte = (int)MIN((size_t)status_byte, (size_t)(end - q));
                // Encoded mode
                if (io->read_proc(&second_byte, sizeof(BYTE), 1, handle) != 1) {
                    throw(1);
                }
                for (int i = 0; i < status_byte; i++) {
                    *q++ = (BYTE)((i & 0x01) ? (second_byte & 0x0f) : ((second_byte >> 4) & 0x0f));
                }
                bits += status_byte;
            } else {
                // Escape mode
                if (io->read_proc(&status_byte, sizeof(BYTE), 1, handle) != 1) {
                    throw(1);
                }
                switch (status_byte) {
                case RLE_ENDOFLINE: {
                    // End of line
                    bits = 0;
                    scanline++;
                    q = pixels + scanline * width;
                } break;

                case RLE_ENDOFBITMAP:
                    // End of bitmap
                    q = end;
                    break;

                case RLE_DELTA: {
                    // read the delta values

                    BYTE delta_x = 0;
                    BYTE delta_y = 0;

                    if (io->read_proc(&delta_x, sizeof(BYTE), 1, handle) != 1) {
                        throw(1);
                    }
                    if (io->read_proc(&delta_y, sizeof(BYTE), 1, handle) != 1) {
                        throw(1);
                    }

                    // apply them

                    bits += delta_x;
                    scanline += delta_y;
                    q = pixels + scanline * width + bits;
                } break;

                default: {
                    // Absolute mode
                    status_byte = (int)MIN((size_t)status_byte, (size_t)(end - q));
                    for (int i = 0; i < status_byte; i++) {
                        if ((i & 0x01) == 0) {
                            if (io->read_proc(&second_byte, sizeof(BYTE), 1, handle) != 1) {
                                throw(1);
                            }
                        }
                        *q++ =
                            (BYTE)((i & 0x01) ? (second_byte & 0x0f) : ((second_byte >> 4) & 0x0f));
                    }
                    bits += status_byte;
                    // Read pad byte
                    if (((status_byte & 0x03) == 1) || ((status_byte & 0x03) == 2)) {
                        BYTE padding = 0;
                        if (io->read_proc(&padding, sizeof(BYTE), 1, handle) != 1) {
                            throw(1);
                        }
                    }
                } break;
                }
            }
        }

        {
            // Convert to 4-bit
            for (int y = 0; y < height; y++) {
                const BYTE *src = (BYTE *)pixels + y * width;
                BYTE *dst = FreeImage_GetScanLine(dib, y);

                BOOL hinibble = TRUE;

                for (int cols = 0; cols < width; cols++) {
                    if (hinibble) {
                        dst[cols >> 1] = (src[cols] << 4);
                    } else {
                        dst[cols >> 1] |= src[cols];
                    }

                    hinibble = !hinibble;
                }
            }
        }

        free(pixels);

        return TRUE;

    } catch (int) {
        if (pixels)
            free(pixels);
        return FALSE;
    }
}

/**
Load image pixels for 8-bit RLE compressed dib
@param io FreeImage IO
@param handle FreeImage IO handle
@param width Image width
@param height Image height
@param dib Image to be loaded
@return Returns TRUE if successful, returns FALSE otherwise
*/
static BOOL LoadPixelDataRLE8(FreeImageIO *io, fi_handle handle, int width, int height,
                              FIBITMAP *dib)
{
    BYTE status_byte = 0;
    BYTE second_byte = 0;
    int scanline = 0;
    int bits = 0;

    for (;;) {
        if (io->read_proc(&status_byte, sizeof(BYTE), 1, handle) != 1) {
            return FALSE;
        }

        switch (status_byte) {
        case RLE_COMMAND:
            if (io->read_proc(&status_byte, sizeof(BYTE), 1, handle) != 1) {
                return FALSE;
            }

            switch (status_byte) {
            case RLE_ENDOFLINE:
                bits = 0;
                scanline++;
                break;

            case RLE_ENDOFBITMAP:
                return TRUE;

            case RLE_DELTA: {
                // read the delta values

                BYTE delta_x = 0;
                BYTE delta_y = 0;

                if (io->read_proc(&delta_x, sizeof(BYTE), 1, handle) != 1) {
                    return FALSE;
                }
                if (io->read_proc(&delta_y, sizeof(BYTE), 1, handle) != 1) {
                    return FALSE;
                }

                // apply them

                bits += delta_x;
                scanline += delta_y;

                break;
            }

            default: {
                if (scanline >= abs(height)) {
                    return TRUE;
                }

                int count = MIN((int)status_byte, width - bits);

                BYTE *sline = FreeImage_GetScanLine(dib, scanline);

                if (io->read_proc((void *)(sline + bits), sizeof(BYTE) * count, 1, handle) != 1) {
                    return FALSE;
                }

                // align run length to even number of bytes

                if ((status_byte & 1) == 1) {
                    if (io->read_proc(&second_byte, sizeof(BYTE), 1, handle) != 1) {
                        return FALSE;
                    }
                }

                bits += status_byte;

                break;
            }
            }

            break;

        default: {
            if (scanline >= abs(height)) {
                return TRUE;
            }

            int count = MIN((int)status_byte, width - bits);

            BYTE *sline = FreeImage_GetScanLine(dib, scanline);

            if (io->read_proc(&second_byte, sizeof(BYTE), 1, handle) != 1) {
                return FALSE;
            }

            for (int i = 0; i < count; i++) {
                *(sline + bits) = second_byte;

                bits++;
            }

            break;
        }
        }
    }
}

// --------------------------------------------------------------------------

static FIBITMAP *LoadWindowsBMP(FreeImageIO *io, fi_handle handle, int flags,
                                unsigned bitmap_bits_offset)
{
    FIBITMAP *dib = nullptr;
    (void)flags;
    try {
        // load the info header

        BITMAPINFOHEADER bih;

        io->read_proc(&bih, sizeof(BITMAPINFOHEADER), 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
        SwapInfoHeader(&bih);
#endif

        // keep some general information about the bitmap

        int used_colors = bih.biClrUsed;
        int width = bih.biWidth;
        int height = bih.biHeight; // WARNING: height can be < 0 => check each call using 'height'
                                   // as a parameter
        int alloc_height = abs(height);
        int bit_count = bih.biBitCount;
        int compression = bih.biCompression;
        int pitch = CalculatePitch(CalculateLine(width, bit_count));

        switch (bit_count) {
        case 1:
        case 4:
        case 8: {
            if ((used_colors <= 0) || (used_colors > CalculateUsedPaletteEntries(bit_count)))
                used_colors = CalculateUsedPaletteEntries(bit_count);

            // allocate enough memory to hold the bitmap (header, palette, pixels) and read the
            // palette

            dib = FreeImage_Allocate(width, alloc_height, bit_count, io);

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information
            FreeImage_SetDotsPerMeterX(dib, bih.biXPelsPerMeter);
            FreeImage_SetDotsPerMeterY(dib, bih.biYPelsPerMeter);

            // load the palette

            io->read_proc(FreeImage_GetPalette(dib), used_colors * sizeof(RGBQUAD), 1, handle);
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
            RGBQUAD *pal = FreeImage_GetPalette(dib);
            for (int i = 0; i < used_colors; i++) {
                INPLACESWAP(pal[i].rgbRed, pal[i].rgbBlue);
            }
#endif

            // seek to the actual pixel data.
            // this is needed because sometimes the palette is larger than the entries it contains
            // predicts

            if (bitmap_bits_offset > (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
                                      + (used_colors * sizeof(RGBQUAD))))
                io->seek_proc(handle, bitmap_bits_offset, SEEK_SET);

            // read the pixel data

            switch (compression) {
            case BI_RGB:
                LoadPixelData(io, handle, dib, height, pitch, bit_count);
                return dib;

            case BI_RLE4:
                if (LoadPixelDataRLE4(io, handle, width, height, dib)) {
                    return dib;
                } else {
                    throw "Error encountered while decoding RLE4 BMP data";
                }
                break;

            case BI_RLE8:
                if (LoadPixelDataRLE8(io, handle, width, height, dib)) {
                    return dib;
                } else {
                    throw "Error encountered while decoding RLE8 BMP data";
                }
                break;

            default:
                throw "compression type not supported";
            }
        } break; // 1-, 4-, 8-bit

        case 16: {
            if (bih.biCompression == BI_BITFIELDS) {
                DWORD bitfields[3];

                io->read_proc(bitfields, 3 * sizeof(DWORD), 1, handle);

                dib = FreeImage_Allocate(width, alloc_height, bit_count, bitfields[0], bitfields[1],
                                         bitfields[2], io);
            } else {
                dib = FreeImage_Allocate(width, alloc_height, bit_count, FI16_555_RED_MASK,
                                         FI16_555_GREEN_MASK, FI16_555_BLUE_MASK, io);
            }

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information
            FreeImage_SetDotsPerMeterX(dib, bih.biXPelsPerMeter);
            FreeImage_SetDotsPerMeterY(dib, bih.biYPelsPerMeter);

            // load pixel data and swap as needed if OS is Big Endian
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            return dib;
        } break; // 16-bit

        case 24:
        case 32: {
            if (bih.biCompression == BI_BITFIELDS) {
                DWORD bitfields[3];

                io->read_proc(bitfields, 3 * sizeof(DWORD), 1, handle);

                dib = FreeImage_Allocate(width, alloc_height, bit_count, bitfields[0], bitfields[1],
                                         bitfields[2], io);
            } else {
                if (bit_count == 32) {
                    dib = FreeImage_Allocate(width, alloc_height, bit_count, FI_RGBA_RED_MASK,
                                             FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, io);
                } else {
                    dib = FreeImage_Allocate(width, alloc_height, bit_count, FI_RGBA_RED_MASK,
                                             FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, io);
                }
            }

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information
            FreeImage_SetDotsPerMeterX(dib, bih.biXPelsPerMeter);
            FreeImage_SetDotsPerMeterY(dib, bih.biYPelsPerMeter);

            // Skip over the optional palette
            // A 24 or 32 bit DIB may contain a palette for faster color reduction

            if (used_colors > 0) {
                io->seek_proc(handle, used_colors * sizeof(RGBQUAD), SEEK_CUR);
            } else if ((bih.biCompression != BI_BITFIELDS)
                       && (bitmap_bits_offset
                           > sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))) {
                io->seek_proc(handle, bitmap_bits_offset, SEEK_SET);
            }

            // read in the bitmap bits
            // load pixel data and swap as needed if OS is Big Endian
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            // check if the bitmap contains transparency, if so enable it in the header

            return dib;
        } break; // 24-, 32-bit
        }
    } catch (const char *message) {
        if (dib) {
            FreeImage_Unload(dib);
        }
        if (message) {
            FreeImage_OutputMessageProc(s_format_id, message, io);
        }
    }

    return nullptr;
}

// --------------------------------------------------------------------------

static FIBITMAP *LoadOS22XBMP(FreeImageIO *io, fi_handle handle, int flags,
                              unsigned bitmap_bits_offset)
{
    FIBITMAP *dib = nullptr;
    (void)flags;
    try {
        // load the info header

        BITMAPINFOHEADER bih;

        io->read_proc(&bih, sizeof(BITMAPINFOHEADER), 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
        SwapInfoHeader(&bih);
#endif

        // keep some general information about the bitmap

        int used_colors = bih.biClrUsed;
        int width = bih.biWidth;
        int height = bih.biHeight; // WARNING: height can be < 0 => check each read_proc using
                                   // 'height' as a parameter
        int alloc_height = abs(height);
        int bit_count = bih.biBitCount;
        int compression = bih.biCompression;
        int pitch = CalculatePitch(CalculateLine(width, bit_count));

        switch (bit_count) {
        case 1:
        case 4:
        case 8: {
            if ((used_colors <= 0) || (used_colors > CalculateUsedPaletteEntries(bit_count)))
                used_colors = CalculateUsedPaletteEntries(bit_count);

            // allocate enough memory to hold the bitmap (header, palette, pixels) and read the
            // palette

            dib = FreeImage_Allocate(width, alloc_height, bit_count, io);

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information
            FreeImage_SetDotsPerMeterX(dib, bih.biXPelsPerMeter);
            FreeImage_SetDotsPerMeterY(dib, bih.biYPelsPerMeter);

            // load the palette

            io->seek_proc(handle, sizeof(BITMAPFILEHEADER) + bih.biSize, SEEK_SET);

            RGBQUAD *pal = FreeImage_GetPalette(dib);

            for (int count = 0; count < used_colors; count++) {
                FILE_BGR bgr;

                io->read_proc(&bgr, sizeof(FILE_BGR), 1, handle);

                pal[count].rgbRed = bgr.r;
                pal[count].rgbGreen = bgr.g;
                pal[count].rgbBlue = bgr.b;
            }

            // seek to the actual pixel data.
            // this is needed because sometimes the palette is larger than the entries it contains
            // predicts

            if (bitmap_bits_offset
                > (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (used_colors * 3)))
                io->seek_proc(handle, bitmap_bits_offset, SEEK_SET);

            // read the pixel data

            switch (compression) {
            case BI_RGB:
                // load pixel data
                LoadPixelData(io, handle, dib, height, pitch, bit_count);
                return dib;

            case BI_RLE4:
                if (LoadPixelDataRLE4(io, handle, width, height, dib)) {
                    return dib;
                } else {
                    throw "Error encountered while decoding RLE4 BMP data";
                }
                break;

            case BI_RLE8:
                if (LoadPixelDataRLE8(io, handle, width, height, dib)) {
                    return dib;
                } else {
                    throw "Error encountered while decoding RLE8 BMP data";
                }
                break;

            default:
                throw "compression type not supported";
            }
        }

        case 16: {
            if (bih.biCompression == 3) {
                DWORD bitfields[3];

                io->read_proc(bitfields, 3 * sizeof(DWORD), 1, handle);

                dib = FreeImage_Allocate(width, alloc_height, bit_count, bitfields[0], bitfields[1],
                                         bitfields[2], io);
            } else {
                dib = FreeImage_Allocate(width, alloc_height, bit_count, FI16_555_RED_MASK,
                                         FI16_555_GREEN_MASK, FI16_555_BLUE_MASK, io);
            }

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information
            FreeImage_SetDotsPerMeterX(dib, bih.biXPelsPerMeter);
            FreeImage_SetDotsPerMeterY(dib, bih.biYPelsPerMeter);

            if (bitmap_bits_offset
                > (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (used_colors * 3))) {
                io->seek_proc(handle, bitmap_bits_offset, SEEK_SET);
            }

            // load pixel data and swap as needed if OS is Big Endian
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            return dib;
        }

        case 24:
        case 32: {
            if (bit_count == 32) {
                dib = FreeImage_Allocate(width, alloc_height, bit_count, FI_RGBA_RED_MASK,
                                         FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, io);
            } else {
                dib = FreeImage_Allocate(width, alloc_height, bit_count, FI_RGBA_RED_MASK,
                                         FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, io);
            }

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information
            FreeImage_SetDotsPerMeterX(dib, bih.biXPelsPerMeter);
            FreeImage_SetDotsPerMeterY(dib, bih.biYPelsPerMeter);

            // Skip over the optional palette
            // A 24 or 32 bit DIB may contain a palette for faster color reduction

            if (bitmap_bits_offset
                > (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (used_colors * 3)))
                io->seek_proc(handle, bitmap_bits_offset, SEEK_SET);

            // read in the bitmap bits
            // load pixel data and swap as needed if OS is Big Endian
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            return dib;
        }
        }
    } catch (const char *message) {
        if (dib)
            FreeImage_Unload(dib);

        FreeImage_OutputMessageProc(s_format_id, message, io);
    }

    return nullptr;
}

// --------------------------------------------------------------------------

static FIBITMAP *LoadOS21XBMP(FreeImageIO *io, fi_handle handle, int flags,
                              unsigned bitmap_bits_offset)
{
    FIBITMAP *dib = nullptr;
    (void)flags;
    try {
        BITMAPINFOOS2_1X_HEADER bios2_1x;

        io->read_proc(&bios2_1x, sizeof(BITMAPINFOOS2_1X_HEADER), 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
        SwapOS21XHeader(&bios2_1x);
#endif
        // keep some general information about the bitmap

        int used_colors = 0;
        int width = bios2_1x.biWidth;
        int height = bios2_1x.biHeight; // WARNING: height can be < 0 => check each read_proc using
                                        // 'height' as a parameter
        int alloc_height = abs(height);
        int bit_count = bios2_1x.biBitCount;
        int pitch = CalculatePitch(CalculateLine(width, bit_count));

        switch (bit_count) {
        case 1:
        case 4:
        case 8: {
            used_colors = CalculateUsedPaletteEntries(bit_count);

            // allocate enough memory to hold the bitmap (header, palette, pixels) and read the
            // palette

            dib = FreeImage_Allocate(width, alloc_height, bit_count, io);

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information to default values (72 dpi in english units)
            FreeImage_SetDotsPerMeterX(dib, 2835);
            FreeImage_SetDotsPerMeterY(dib, 2835);

            // load the palette

            RGBQUAD *pal = FreeImage_GetPalette(dib);

            for (int count = 0; count < used_colors; count++) {
                FILE_BGR bgr;

                io->read_proc(&bgr, sizeof(FILE_BGR), 1, handle);

                pal[count].rgbRed = bgr.r;
                pal[count].rgbGreen = bgr.g;
                pal[count].rgbBlue = bgr.b;
            }

            // Skip over the optional palette
            // A 24 or 32 bit DIB may contain a palette for faster color reduction

            io->seek_proc(handle, bitmap_bits_offset, SEEK_SET);

            // read the pixel data

            // load pixel data
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            return dib;
        }

        case 16: {
            dib = FreeImage_Allocate(width, alloc_height, bit_count, FI16_555_RED_MASK,
                                     FI16_555_GREEN_MASK, FI16_555_BLUE_MASK, io);

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information to default values (72 dpi in english units)
            FreeImage_SetDotsPerMeterX(dib, 2835);
            FreeImage_SetDotsPerMeterY(dib, 2835);

            // load pixel data and swap as needed if OS is Big Endian
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            return dib;
        }

        case 24:
        case 32: {
            if (bit_count == 32) {
                dib = FreeImage_Allocate(width, alloc_height, bit_count, FI_RGBA_RED_MASK,
                                         FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, io);
            } else {
                dib = FreeImage_Allocate(width, alloc_height, bit_count, FI_RGBA_RED_MASK,
                                         FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, io);
            }

            if (dib == nullptr)
                throw "DIB allocation failed";

            // set resolution information to default values (72 dpi in english units)
            FreeImage_SetDotsPerMeterX(dib, 2835);
            FreeImage_SetDotsPerMeterY(dib, 2835);

            // Skip over the optional palette
            // A 24 or 32 bit DIB may contain a palette for faster color reduction

            // load pixel data and swap as needed if OS is Big Endian
            LoadPixelData(io, handle, dib, height, pitch, bit_count);

            // check if the bitmap contains transparency, if so enable it in the header

            return dib;
        }
        }
    } catch (const char *message) {
        if (dib)
            FreeImage_Unload(dib);

        FreeImage_OutputMessageProc(s_format_id, message, io);
    }

    return nullptr;
}

// ==========================================================
// Plugin Implementation
// ==========================================================

// ----------------------------------------------------------

static FIBITMAP *DoLoadBMP(FreeImageIO *io, fi_handle handle, int flags)
{
    if (handle != nullptr) {
        BITMAPFILEHEADER bitmapfileheader;
        DWORD type = 0;
        BYTE magic[2];

        // we use this offset value to make seemingly absolute seeks relative in the file

        long offset_in_file = io->tell_proc(handle);

        // read the magic

        io->read_proc(&magic, sizeof(magic), 1, handle);

        // compare the magic with the number we know

        // somebody put a comment here explaining the purpose of this loop
        while (memcmp(&magic, "BA", 2) == 0) {
            io->read_proc(&bitmapfileheader.bfSize, sizeof(DWORD), 1, handle);
            io->read_proc(&bitmapfileheader.bfReserved1, sizeof(WORD), 1, handle);
            io->read_proc(&bitmapfileheader.bfReserved2, sizeof(WORD), 1, handle);
            io->read_proc(&bitmapfileheader.bfOffBits, sizeof(DWORD), 1, handle);
            io->read_proc(&magic, sizeof(magic), 1, handle);
        }

        // read the fileheader

        io->seek_proc(handle, (0 - (int)sizeof(magic)), SEEK_CUR);
        io->read_proc(&bitmapfileheader, (int)sizeof(BITMAPFILEHEADER), 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
        SwapFileHeader(&bitmapfileheader);
#endif

        // read the first byte of the infoheader

        io->read_proc(&type, sizeof(DWORD), 1, handle);
        io->seek_proc(handle, 0 - (int)sizeof(DWORD), SEEK_CUR);
#ifdef FREEIMAGE_BIGENDIAN
        SwapLong(&type);
#endif

        // call the appropriate load function for the found bitmap type

        if (type == 40)
            return LoadWindowsBMP(io, handle, flags, offset_in_file + bitmapfileheader.bfOffBits);

        if (type == 12)
            return LoadOS21XBMP(io, handle, flags, offset_in_file + bitmapfileheader.bfOffBits);

        if (type <= 64)
            return LoadOS22XBMP(io, handle, flags, offset_in_file + bitmapfileheader.bfOffBits);

        char buf[256];
        sprintf(buf, "unknown bmp subtype with id %d", type);
        FreeImage_OutputMessageProc(s_format_id, buf, io);
    }

    return nullptr;
}

template <quint32 TBitWidth>
struct SPaletteIndexer
{
    static inline uint32_t IndexOf(const uint8_t *inData, uint32_t inPos)
    {
        uint32_t divisor = 8 / TBitWidth;
        uint32_t byte = inPos / divisor;
        uint32_t modulus = inPos % divisor;
        uint32_t shift = TBitWidth * modulus;
        uint32_t mask = (1 << TBitWidth) - 1;
        mask = mask << shift;
        uint32_t byteData = inData[byte];
        return (byteData & mask) >> shift;
    }
};

template <>
struct SPaletteIndexer<1>
{
    static inline uint32_t IndexOf(const uint8_t *inData, uint32_t inPos)
    {
        uint32_t byte = (inPos / 8);
        uint32_t bit = 1 << (7 - (inPos % 8));
        uint32_t byteData = inData[byte];
        return (byteData & bit) ? 1 : 0;
    }
};

template <>
struct SPaletteIndexer<8>
{
    static inline uint32_t IndexOf(const uint8_t *inData, uint32_t inPos)
    {
        uint32_t byte = inPos;
        uint32_t bit = 0xFF;
        uint32_t byteData = inData[byte];
        return byteData & bit;
    }
};

static inline void assignQuad(uint8_t *dest, const RGBQUAD &quad)
{
    dest[0] = quad.rgbRed;
    dest[1] = quad.rgbGreen;
    dest[2] = quad.rgbBlue;
}

template <quint32 bitCount>
inline void LoadPalettized(bool inFlipY, const RGBQUAD *palette, void *data, uint8_t *newData,
                           int width, int height, int components, int transparentIndex)
{
    const uint8_t *oldData = (const uint8_t *)data;
    int pitch = CalculatePitch(CalculateLine(width, bitCount));
    for (uint32_t h = 0; h < (uint32_t)height; ++h) {
        uint32_t relHeight = h;
        if (inFlipY)
            relHeight = ((uint32_t)height) - h - 1;
        for (uint32_t w = 0; w < (uint32_t)width; ++w) {
            const uint8_t *dataLine = oldData + pitch * h;
            uint32_t pos = width * relHeight + w;
            uint32_t paletteIndex = SPaletteIndexer<bitCount>::IndexOf(dataLine, w);
            const RGBQUAD &theQuad = palette[paletteIndex];
            uint8_t *writePtr = newData + (pos * components);
            assignQuad(writePtr, theQuad);
            if (paletteIndex == (uint32_t)transparentIndex && components == 4) {
                writePtr[3] = 0;
            }
        }
    }
}

inline int firstHighBit(int data)
{
    if (data == 0)
        return 0;
    int idx = 0;
    while ((data % 2) == 0) {
        data = data >> 1;
        ++idx;
    }
    return idx;
}

struct SMaskData
{
    uint32_t mask;
    uint32_t shift;
    uint32_t max;

    SMaskData(int inMask)
    {
        mask = inMask;
        shift = firstHighBit(mask);
        max = mask >> shift;
    }

    inline uint8_t MapColor(uint32_t color) const
    {
        uint32_t intermediateValue = (color & mask) >> shift;
        return (uint8_t)((intermediateValue * 255) / max);
    }
};

template <quint32>
struct ColorAccess
{
};

template <>
struct ColorAccess<16>
{
    static uint32_t GetPixelWidth() { return 2; }
    static uint32_t GetColor(const char8_t *src)
    {
        return (uint32_t) * reinterpret_cast<const quint16 *>(src);
    }
};

template <>
struct ColorAccess<24>
{
    static uint32_t GetPixelWidth() { return 3; }
    static uint32_t GetColor(const char8_t *src)
    {
        return (uint32_t)(*reinterpret_cast<const quint32 *>(src) & 0xFFFFFF);
    }
};

template <>
struct ColorAccess<32>
{
    static uint32_t GetPixelWidth() { return 4; }
    static uint32_t GetColor(const char8_t *src)
    {
        return *reinterpret_cast<const uint32_t *>(src);
    }
};

template <quint32 TBitCount>
inline void LoadMasked(bool inFlipY, qint32 *inMasks, void *data, uint8_t *newData, int width,
                       int height)
{
    const char8_t *oldData = (const char8_t *)data;
    SMaskData rMask(inMasks[0]);
    SMaskData gMask(inMasks[1]);
    SMaskData bMask(inMasks[2]);
    for (int h = 0; h < height; ++h) {
        int relHeight = h;
        if (inFlipY)
            relHeight = height - h - 1;
        for (int w = 0; w < width; ++w) {
            int pos = width * relHeight + w;
            const char8_t *readPtr = oldData + (pos * ColorAccess<TBitCount>::GetPixelWidth());
            uint8_t *writePtr = newData + (pos * 3);
            uint32_t colorVal = ColorAccess<TBitCount>::GetColor(readPtr);
            writePtr[0] = rMask.MapColor(colorVal);
            writePtr[1] = gMask.MapColor(colorVal);
            writePtr[2] = bMask.MapColor(colorVal);
        }
    }
}

void SLoadedTexture::FreeImagePostProcess(bool inFlipY)
{
    // We always convert 32 bit RGBA
    if (m_ExtendedFormat != ExtendedTextureFormats::NoExtendedFormat) {

        quint32 stride = 3 * width;
        format = QDemonRenderTextureFormats::RGB8;
        components = 3;
        if (m_ExtendedFormat == ExtendedTextureFormats::Palettized && m_TransparentPaletteIndex > -1
            && m_TransparentPaletteIndex < 256) {
            stride = 4 * width;
            components = 4;
            format = QDemonRenderTextureFormats::RGBA8;
        }
        quint32 byteSize = height * stride;
        uint8_t *newData =
            (uint8_t *)m_Allocator.allocate(byteSize, "texture data", __FILE__, __LINE__);
        if (format == QDemonRenderTextureFormats::RGBA8)
            memSet(newData, 255, byteSize);
        switch (m_ExtendedFormat) {
        case ExtendedTextureFormats::Palettized: {
            RGBQUAD *palette = (RGBQUAD *)m_Palette;
            switch (m_BitCount) {
            case 1:
                LoadPalettized<1>(inFlipY, palette, data, newData, width, height, components,
                                  m_TransparentPaletteIndex);
                break;
            case 2:
                LoadPalettized<2>(inFlipY, palette, data, newData, width, height, components,
                                  m_TransparentPaletteIndex);
                break;
            case 4:
                LoadPalettized<4>(inFlipY, palette, data, newData, width, height, components,
                                  m_TransparentPaletteIndex);
                break;
            case 8:
                LoadPalettized<8>(inFlipY, palette, data, newData, width, height, components,
                                  m_TransparentPaletteIndex);
                break;
            default:
                Q_ASSERT(false);
                memSet(newData, 0, byteSize);
                break;
            }
        } break;
        case ExtendedTextureFormats::CustomRGB: {
            switch (m_BitCount) {
            case 16:
                LoadMasked<16>(inFlipY, m_CustomMasks, data, newData, width, height);
                break;
            case 24:
                LoadMasked<24>(inFlipY, m_CustomMasks, data, newData, width, height);
                break;
            case 32:
                LoadMasked<32>(inFlipY, m_CustomMasks, data, newData, width, height);
                break;
            default:
                Q_ASSERT(false);
                memSet(newData, 0, byteSize);
                break;
            }
        } break;
        default:
            Q_ASSERT(false);
            memSet(newData, 0, byteSize);
            break;
        }
        m_Allocator.deallocate(data);
        if (m_Palette)
            m_Allocator.deallocate(m_Palette);
        data = newData;
        m_Palette = nullptr;
        m_BitCount = 0;
        this->dataSizeInBytes = byteSize;
        m_ExtendedFormat = ExtendedTextureFormats::NoExtendedFormat;
    }
}

SLoadedTexture *SLoadedTexture::LoadBMP(ISeekableIOStream &inStream, bool inFlipY,
                                        NVFoundationBase &inFnd,
                                        QDemonRenderContextType renderContextType)
{
    Q_UNUSED(renderContextType)
    FreeImageIO theIO(inFnd.getAllocator(), inFnd);
    SLoadedTexture *retval = DoLoadBMP(&theIO, &inStream, 0);
    if (retval)
        retval->FreeImagePostProcess(inFlipY);
    return retval;
}
