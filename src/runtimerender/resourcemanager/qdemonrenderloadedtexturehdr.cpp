/****************************************************************************
**
** Copyright (C) 2014 NVIDIA Corporation.
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
// Radiance RGBE .HDR Loader
// Decodes Radiance RGBE HDR image into FP16 texture buffer.
//
// Implementation by
//	Parashar Krishnamachari (parashark@nvidia.com)
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
#include <QtDemonRender/qdemonrenderbasetypes.h>

typedef unsigned char RGBE[4];
#define R 0
#define G 1
#define B 2
#define E 3

#define MINELEN 8 // minimum scanline length for encoding
#define MAXELEN 0x7fff // maximum scanline length for encoding

static int s_format_id;

static float convertComponent(int exponent, int val)
{
    float v = val / (256.0f);
    float d = powf(2.0f, (float)exponent - 128.0f);
    return v * d;
}

static void decrunchScanlineOld(FreeImageIO *io, fi_handle handle, RGBE *scanline, int width)
{
    int i;
    int rshift = 0;

    while (width > 0) {
        io->read_proc(scanline, 4, 1, handle);

        // The older version of RLE encodes the length in the exponent.
        // and marks a run with 1, 1, 1 in RGB.  This is differentiated from
        // a raw value of 1, 1, 1, by having a exponent of 0;
        if (scanline[0][R] == 1 && scanline[0][G] == 1 && scanline[0][B] == 1) {
            for (i = (scanline[0][E] << rshift); i > 0; --i) {
                memcpy(&scanline[0][0], &scanline[-1][0], 4);
                ++scanline;
                --width;
            }
            rshift += 8;
        } else {
            ++scanline;
            --width;
            rshift = 0;
        }
    }
}

static void decrunchScanline(FreeImageIO *io, fi_handle handle, RGBE *scanline, int width)
{
    if ((width < MINELEN) || (width > MAXELEN)) {
        decrunchScanlineOld(io, handle, scanline, width);
        return;
    }

    char c;
    io->read_proc(&c, 1, 1, handle);
    if (c != 2) {
        io->seek_proc(handle, -1, SEEK_CUR);
        decrunchScanlineOld(io, handle, scanline, width);
        return;
    }

    io->read_proc(&(scanline[0][G]), 1, 1, handle);
    io->read_proc(&(scanline[0][B]), 1, 1, handle);
    io->read_proc(&c, 1, 1, handle);

    if (scanline[0][G] != 2 || scanline[0][B] & 128) {
        scanline[0][R] = 2;
        scanline[0][E] = c;
        decrunchScanlineOld(io, handle, scanline + 1, width - 1);
    }

    // RLE-encoded version does a separate buffer for each channel per scanline
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < width;) {
            unsigned char code, val;
            io->read_proc(&code, 1, 1, handle);
            if (code
                > 128) // RLE-encoded run...  read 1 value and copy it forward for some n count.
            {
                code &= 127;
                io->read_proc(&val, 1, 1, handle);
                while (code--)
                    scanline[j++][i] = val;
            } else // Not a run, so we read it as raw data
            {
                // Note -- we store each pixel in memory 4 bytes apart, so we can't just
                // do one long read.
                while (code--)
                    io->read_proc(&(scanline[j++][i]), 1, 1, handle);
            }
        }
    }
}

static void decodeScanlineToTexture(RGBE *scanline, int width, void *outBuf, quint32 offset,
                                    QDemonRenderTextureFormats::Enum inFormat)
{
    float rgbaF32[4];

    for (int i = 0; i < width; ++i) {
        rgbaF32[R] = convertComponent(scanline[i][E], scanline[i][R]);
        rgbaF32[G] = convertComponent(scanline[i][E], scanline[i][G]);
        rgbaF32[B] = convertComponent(scanline[i][E], scanline[i][B]);
        rgbaF32[3] = 1.0f;

        quint8 *target = reinterpret_cast<quint8 *>(outBuf);
        target += offset;
        QDemonRenderTextureFormats::encodeToPixel(
            rgbaF32, target, i * QDemonRenderTextureFormats::getSizeofFormat(inFormat), inFormat);
    }
}

static FIBITMAP *DoLoadHDR(FreeImageIO *io, fi_handle handle,
                           QDemonRenderTextureFormats::Enum inFormat = QDemonRenderTextureFormats::RGB32F)
{
    FIBITMAP *dib = nullptr;
    try {
        if (handle != nullptr) {
            char str[200];
            int i;

            // Make sure it's a Radiance RGBE file
            io->read_proc(str, 10, 1, handle);
            if (memcmp(str, "#?RADIANCE", 10)) {
                throw "Invalid HDR file";
            }

            io->seek_proc(handle, 1, SEEK_CUR);

            // Get the command string (it's not really important for us;  We're always assuming
            // 32bit_rle_rgbe is the format
            // we're just reading it to skip ahead the correct number of bytes).
            i = 0;
            char c = 0, prevC;
            do {
                prevC = c;
                io->read_proc(&c, 1, 1, handle);
                str[i++] = c;
            } while (!(c == 0xa && prevC == 0xa));

            // Get the resolution string (it will be nullptr-terminated for us)
            char res[200];
            i = 0;
            do {
                io->read_proc(&c, 1, 1, handle);
                res[i++] = c;
            } while (c != 0xa);
            res[i] = 0;

            int width, height;
            if (!sscanf(res, "-Y %d +X %d", &height, &width)) {
                throw "Error encountered while loading HDR stream : could not determine image "
                      "resolution!";
            }
            int bytesPerPixel = QDemonRenderTextureFormats::getSizeofFormat(inFormat);
            dib = FreeImage_Allocate(width, height, bytesPerPixel * 8, io);
            if (dib == nullptr) {
                throw "DIB allocation failed";
            }

            dib->format = inFormat;
            dib->components = QDemonRenderTextureFormats::getNumberOfComponent(inFormat);

            // Allocate a scanline worth of RGBE data
            RGBE *scanline = new RGBE[width];
            if (!scanline) {
                throw "Error encountered while loading HDR stream : could not buffer scanlines!";
            }

            // Go through all the scanlines
            for (int y = 0; y < height; ++y) {
                quint32 byteOfs = (height - 1 - y) * width * bytesPerPixel;
                decrunchScanline(io, handle, scanline, width);
                decodeScanlineToTexture(scanline, width, dib->data, byteOfs, inFormat);
            }
        }
        return dib;
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

SLoadedTexture *SLoadedTexture::LoadHDR(ISeekableIOStream &inStream,
                                        QDemonRenderContextType renderContextType)
{
    FreeImageIO theIO();
    SLoadedTexture *retval = nullptr;
    if (renderContextType == QDemonRenderContextValues::GLES2)
        retval = DoLoadHDR(&theIO, &inStream, QDemonRenderTextureFormats::RGBA8);
    else
        retval = DoLoadHDR(&theIO, &inStream, QDemonRenderTextureFormats::RGBA16F);


    // Let's just assume we don't support this just yet.
    //	if ( retval )
    //		retval->FreeImagePostProcess( inFlipY );
    return retval;
}
