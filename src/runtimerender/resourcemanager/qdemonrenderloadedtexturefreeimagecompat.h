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
#pragma once
#ifndef QDEMON_RENDER_LOADED_TEXTURE_FREEIMAGE_COMPAT_H
#define QDEMON_RENDER_LOADED_TEXTURE_FREEIMAGE_COMPAT_H
#include <IOStreams.h>
#include <Qt3DSFoundation.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <QtDemonRuntimeRender/qdemonrenderloadedtexture.h>
#include <stdlib.h>
#ifndef _MACOSX
#ifndef _INTEGRITYPLATFORM
#include <malloc.h>
#endif
#endif

// We use a compatibility layer so we can easily convert freeimage code to load our texture formats
// where necessary.

QT_BEGIN_NAMESPACE

typedef int32_t BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

#define FREEIMAGE_COLORORDER_BGR 0
#define FREEIMAGE_COLORORDER_RGB 1

#define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_BGR

typedef ISeekableIOStream *fi_handle;

struct FreeImageIO
{
    NVAllocatorCallback &m_Allocator;
    NVFoundationBase &m_Foundation;
    int (*read_proc)(void *data, int size, int itemSize, fi_handle handle);
    void (*seek_proc)(fi_handle handle, int offset, int pos);
    int (*tell_proc)(fi_handle handle);
    static inline int reader(void *data, int size, int itemSize, fi_handle handle)
    {
        QDemonDataRef<quint8> theData(toDataRef((quint8 *)data, (quint32)size * itemSize));
        quint32 amount = handle->Read(theData);
        return (int)amount;
    }
    static inline void seeker(fi_handle handle, int offset, int pos)
    {
        SeekPosition::Enum seekPos(SeekPosition::Begin);
        /*
#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0*/
        switch (pos) {
        case 0:
            seekPos = SeekPosition::Begin;
            break;
        case 1:
            seekPos = SeekPosition::Current;
            break;
        case 2:
            seekPos = SeekPosition::End;
            break;
        default:
            Q_ASSERT(false);
            break;
        }
        handle->SetPosition(offset, seekPos);
    }
    static inline int teller(fi_handle handle) { return (int)handle->GetPosition(); }
    FreeImageIO(NVAllocatorCallback &alloc, NVFoundationBase &fnd)
        : m_Allocator(alloc)
        , m_Foundation(fnd)
        , read_proc(reader)
        , seek_proc(seeker)
        , tell_proc(teller)
    {
    }
};

typedef SLoadedTexture FIBITMAP;
inline BYTE *FreeImage_GetBits(FIBITMAP *bmp) { return (BYTE *)bmp->data; }

inline int FreeImage_GetHeight(FIBITMAP *bmp) { return bmp->height; }
inline int FreeImage_GetWidth(FIBITMAP *bmp) { return bmp->width; }

#define INPLACESWAP(x, y) eastl::swap(x, y)
#define MIN(x, y) NVMin(x, y)
#define MAX(x, y) NVMax(x, y)

#define TRUE 1
#define FALSE 0

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
#else
    BYTE rgbRed;
    BYTE rgbGreen;
    BYTE rgbBlue;
#endif // FREEIMAGE_COLORORDER
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagRGBTRIPLE
{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
#else
    BYTE rgbtRed;
    BYTE rgbtGreen;
    BYTE rgbtBlue;
#endif // FREEIMAGE_COLORORDER
} RGBTRIPLE;

typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO, *PBITMAPINFO;

typedef struct tagFILE_RGBA
{
    unsigned char r, g, b, a;
} FILE_RGBA;

typedef struct tagFILE_BGRA
{
    unsigned char b, g, r, a;
} FILE_BGRA;

typedef struct tagFILE_RGB
{
    unsigned char r, g, b;
} FILE_RGB;

typedef struct tagFILE_BGR
{
    unsigned char b, g, r;
} FILE_BGR;

// Indexes for byte arrays, masks and shifts for treating pixels as words ---
// These coincide with the order of RGBQUAD and RGBTRIPLE -------------------

#ifndef FREEIMAGE_BIGENDIAN
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
// Little Endian (x86 / MS Windows, Linux) : BGR(A) order
#define FI_RGBA_RED 2
#define FI_RGBA_GREEN 1
#define FI_RGBA_BLUE 0
#define FI_RGBA_ALPHA 3
#define FI_RGBA_RED_MASK 0x00FF0000
#define FI_RGBA_GREEN_MASK 0x0000FF00
#define FI_RGBA_BLUE_MASK 0x000000FF
#define FI_RGBA_ALPHA_MASK 0xFF000000
#define FI_RGBA_RED_SHIFT 16
#define FI_RGBA_GREEN_SHIFT 8
#define FI_RGBA_BLUE_SHIFT 0
#define FI_RGBA_ALPHA_SHIFT 24
#else
// Little Endian (x86 / MaxOSX) : RGB(A) order
#define FI_RGBA_RED 0
#define FI_RGBA_GREEN 1
#define FI_RGBA_BLUE 2
#define FI_RGBA_ALPHA 3
#define FI_RGBA_RED_MASK 0x000000FF
#define FI_RGBA_GREEN_MASK 0x0000FF00
#define FI_RGBA_BLUE_MASK 0x00FF0000
#define FI_RGBA_ALPHA_MASK 0xFF000000
#define FI_RGBA_RED_SHIFT 0
#define FI_RGBA_GREEN_SHIFT 8
#define FI_RGBA_BLUE_SHIFT 16
#define FI_RGBA_ALPHA_SHIFT 24
#endif // FREEIMAGE_COLORORDER
#else
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
// Big Endian (PPC / none) : BGR(A) order
#define FI_RGBA_RED 2
#define FI_RGBA_GREEN 1
#define FI_RGBA_BLUE 0
#define FI_RGBA_ALPHA 3
#define FI_RGBA_RED_MASK 0x0000FF00
#define FI_RGBA_GREEN_MASK 0x00FF0000
#define FI_RGBA_BLUE_MASK 0xFF000000
#define FI_RGBA_ALPHA_MASK 0x000000FF
#define FI_RGBA_RED_SHIFT 8
#define FI_RGBA_GREEN_SHIFT 16
#define FI_RGBA_BLUE_SHIFT 24
#define FI_RGBA_ALPHA_SHIFT 0
#else
// Big Endian (PPC / Linux, MaxOSX) : RGB(A) order
#define FI_RGBA_RED 0
#define FI_RGBA_GREEN 1
#define FI_RGBA_BLUE 2
#define FI_RGBA_ALPHA 3
#define FI_RGBA_RED_MASK 0xFF000000
#define FI_RGBA_GREEN_MASK 0x00FF0000
#define FI_RGBA_BLUE_MASK 0x0000FF00
#define FI_RGBA_ALPHA_MASK 0x000000FF
#define FI_RGBA_RED_SHIFT 24
#define FI_RGBA_GREEN_SHIFT 16
#define FI_RGBA_BLUE_SHIFT 8
#define FI_RGBA_ALPHA_SHIFT 0
#endif // FREEIMAGE_COLORORDER
#endif // FREEIMAGE_BIGENDIAN

#define FI_RGBA_RGB_MASK (FI_RGBA_RED_MASK | FI_RGBA_GREEN_MASK | FI_RGBA_BLUE_MASK)

// The 16bit macros only include masks and shifts, since each color element is not byte aligned

#define FI16_555_RED_MASK 0x7C00
#define FI16_555_GREEN_MASK 0x03E0
#define FI16_555_BLUE_MASK 0x001F
#define FI16_555_RED_SHIFT 10
#define FI16_555_GREEN_SHIFT 5
#define FI16_555_BLUE_SHIFT 0
#define FI16_565_RED_MASK 0xF800
#define FI16_565_GREEN_MASK 0x07E0
#define FI16_565_BLUE_MASK 0x001F
#define FI16_565_RED_SHIFT 11
#define FI16_565_GREEN_SHIFT 5
#define FI16_565_BLUE_SHIFT 0

inline unsigned char HINIBBLE(unsigned char byte) { return byte & 0xF0; }

inline unsigned char LOWNIBBLE(unsigned char byte) { return byte & 0x0F; }

inline int CalculateUsedBits(int bits)
{
    int bit_count = 0;
    unsigned bit = 1;

    for (unsigned i = 0; i < 32; i++) {
        if ((bits & bit) == bit) {
            bit_count++;
        }

        bit <<= 1;
    }

    return bit_count;
}

inline int CalculateLine(int width, int bitdepth) { return ((width * bitdepth) + 7) / 8; }

inline int CalculatePitch(int line) { return (line + 3) & ~3; }

inline int CalculateUsedPaletteEntries(int bit_count)
{
    if ((bit_count >= 1) && (bit_count <= 8))
        return 1 << bit_count;

    return 0;
}

inline unsigned char *CalculateScanLine(unsigned char *bits, unsigned pitch, int scanline)
{
    return (bits + (pitch * scanline));
}

inline void ReplaceExtension(char *result, const char *filename, const char *extension)
{
    for (size_t i = strlen(filename) - 1; i > 0; --i) {
        if (filename[i] == '.') {
            memcpy(result, filename, i);
            result[i] = '.';
            memcpy(result + i + 1, extension, strlen(extension) + 1);
            return;
        }
    }

    memcpy(result, filename, strlen(filename));
    result[strlen(filename)] = '.';
    memcpy(result + strlen(filename) + 1, extension, strlen(extension) + 1);
}

inline BYTE *FreeImage_GetScanLine(FIBITMAP *bmp, int height)
{
    return CalculateScanLine(
                (BYTE *)bmp->data, CalculatePitch(CalculateLine(bmp->width, bmp->m_BitCount)), height);
}

#define DLL_CALLCONV

// ignored for now.
#define FreeImage_SetDotsPerMeterX(img, dots)
#define FreeImage_SetDotsPerMeterY(img, dots)

inline SLoadedTexture *FreeImage_Allocate(int width, int height, int bit_count, FreeImageIO *io)
{
    SLoadedTexture *theTexture = QDEMON_NEW(io->m_Allocator, SLoadedTexture)(io->m_Allocator);
    int pitch = CalculatePitch(CalculateLine(width, bit_count));
    quint32 dataSize = (quint32)(height * pitch);
    theTexture->dataSizeInBytes = dataSize;
    theTexture->data = io->m_Allocator.allocate(dataSize, "image data", __FILE__, __LINE__);
    memZero(theTexture->data, dataSize);
    theTexture->width = width;
    theTexture->height = height;
    theTexture->m_BitCount = bit_count;
    // If free image asks us for a palette, we change our format at that time.
    theTexture->m_ExtendedFormat = ExtendedTextureFormats::CustomRGB;
    return theTexture;
}

inline SLoadedTexture *FreeImage_Allocate(int width, int height, int bit_count, int rmask,
                                          int gmask, int bmask, FreeImageIO *io)
{
    SLoadedTexture *retval = FreeImage_Allocate(width, height, bit_count, io);
    retval->m_CustomMasks[0] = rmask;
    retval->m_CustomMasks[1] = gmask;
    retval->m_CustomMasks[2] = bmask;
    return retval;
}

inline RGBQUAD *FreeImage_GetPalette(SLoadedTexture *texture)
{
    if (texture->m_Palette == nullptr) {
        texture->m_ExtendedFormat = ExtendedTextureFormats::Palettized;
        quint32 memory = 256 * sizeof(RGBQUAD);
        if (memory) {
            texture->m_Palette =
                    texture->m_Allocator.allocate(memory, "texture palette", __FILE__, __LINE__);
            memZero(texture->m_Palette, memory);
        }
    }
    return (RGBQUAD *)texture->m_Palette;
}

inline void FreeImage_Unload(SLoadedTexture *texture) { texture->release(); }
inline void FreeImage_OutputMessageProc(int, const char *message, FreeImageIO *io)
{
    Q_UNUSED(io);
    qCCritical(INVALID_OPERATION, "Error loading image: %s", message);
}

inline void FreeImage_SetBackgroundColor(SLoadedTexture *texture, RGBQUAD *inColor)
{
    if (inColor) {
        texture->m_BackgroundColor[0] = inColor->rgbRed;
        texture->m_BackgroundColor[1] = inColor->rgbGreen;
        texture->m_BackgroundColor[2] = inColor->rgbBlue;
    } else
        memSet(texture->m_BackgroundColor, 0, 3);
}

inline void FreeImage_SetTransparencyTable(SLoadedTexture *texture, BYTE *table, int size)
{
    if (texture->m_TransparencyTable)
        texture->m_Allocator.deallocate(texture->m_TransparencyTable);
    texture->m_TransparencyTable = nullptr;
    if (table && size) {
        texture->m_TransparencyTable = (uint8_t *)texture->m_Allocator.allocate(
                    size, "texture transparency table", __FILE__, __LINE__);
        memCopy(texture->m_TransparencyTable, table, size);
    }
}
QT_END_NAMESPACE

#endif
