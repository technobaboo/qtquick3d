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
// GIF Loader and Writer
//
// Design and implementation by
// - Ryan Rubley <ryan@lostreality.org>
// - Raphael Gaquer <raphael.gaquer@alcer.com>
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
#ifdef _MSC_VER
#pragma warning(disable : 4786) // identifier was truncated to 'number' characters
#endif

#include <Qt3DSRenderLoadedTextureFreeImageCompat.h>
// ==========================================================
//   Metadata declarations
// ==========================================================

#define GIF_DISPOSAL_UNSPECIFIED 0
#define GIF_DISPOSAL_LEAVE 1
#define GIF_DISPOSAL_BACKGROUND 2
#define GIF_DISPOSAL_PREVIOUS 3

// ==========================================================
//   Constant/Typedef declarations
// ==========================================================

struct GIFinfo
{
    BOOL read;
    // only really used when reading
    size_t global_color_table_offset;
    int global_color_table_size;
    BYTE background_color;
    eastl::vector<size_t> application_extension_offsets;
    eastl::vector<size_t> comment_extension_offsets;
    eastl::vector<size_t> graphic_control_extension_offsets;
    eastl::vector<size_t> image_descriptor_offsets;

    GIFinfo()
        : read(0)
        , global_color_table_offset(0)
        , global_color_table_size(0)
        , background_color(0)
    {
    }
};

struct PageInfo
{
    PageInfo(int d, int l, int t, int w, int h)
    {
        disposal_method = d;
        left = (WORD)l;
        top = (WORD)t;
        width = (WORD)w;
        height = (WORD)h;
    }
    int disposal_method;
    WORD left, top, width, height;
};

// GIF defines a max of 12 bits per code
#define MAX_LZW_CODE 4096

class StringTable
{
public:
    StringTable();
    ~StringTable();
    void Initialize(int minCodeSize);
    BYTE *FillInputBuffer(int len);
    void CompressStart(int bpp, int width);
    int CompressEnd(BYTE *buf); // 0-4 bytes
    bool Compress(BYTE *buf, int *len);
    bool Decompress(BYTE *buf, int *len);
    void Done(void);

protected:
    bool m_done;

    int m_minCodeSize, m_clearCode, m_endCode, m_nextCode;

    int m_bpp, m_slack; // Compressor information

    int m_prefix; // Compressor state variable
    int m_codeSize, m_codeMask; // Compressor/Decompressor state variables
    int m_oldCode; // Decompressor state variable
    int m_partial, m_partialSize; // Compressor/Decompressor bit buffer

    int firstPixelPassed; // A specific flag that indicates if the first pixel
    // of the whole image had already been read

    QString m_strings[MAX_LZW_CODE]; // This is what is really the "string table" data for the
                                           // Decompressor
    int *m_strmap;

    // input buffer
    BYTE *m_buffer;
    int m_bufferSize, m_bufferRealSize, m_bufferPos, m_bufferShift;

    void ClearCompressorTable(void);
    void ClearDecompressorTable(void);
};

#define GIF_PACKED_LSD_HAVEGCT 0x80
#define GIF_PACKED_LSD_COLORRES 0x70
#define GIF_PACKED_LSD_GCTSORTED 0x08
#define GIF_PACKED_LSD_GCTSIZE 0x07
#define GIF_PACKED_ID_HAVELCT 0x80
#define GIF_PACKED_ID_INTERLACED 0x40
#define GIF_PACKED_ID_LCTSORTED 0x20
#define GIF_PACKED_ID_RESERVED 0x18
#define GIF_PACKED_ID_LCTSIZE 0x07
#define GIF_PACKED_GCE_RESERVED 0xE0
#define GIF_PACKED_GCE_DISPOSAL 0x1C
#define GIF_PACKED_GCE_WAITINPUT 0x02
#define GIF_PACKED_GCE_HAVETRANS 0x01

#define GIF_BLOCK_IMAGE_DESCRIPTOR 0x2C
#define GIF_BLOCK_EXTENSION 0x21
#define GIF_BLOCK_TRAILER 0x3B

#define GIF_EXT_PLAINTEXT 0x01
#define GIF_EXT_GRAPHIC_CONTROL 0xF9
#define GIF_EXT_COMMENT 0xFE
#define GIF_EXT_APPLICATION 0xFF

#define GIF_INTERLACE_PASSES 4
static int g_GifInterlaceOffset[GIF_INTERLACE_PASSES] = { 0, 4, 2, 1 };
static int g_GifInterlaceIncrement[GIF_INTERLACE_PASSES] = { 8, 8, 4, 2 };

StringTable::StringTable()
{
    m_buffer = nullptr;
    firstPixelPassed = 0; // Still no pixel read
    // Maximum number of entries in the map is MAX_LZW_CODE * 256
    // (aka 2**12 * 2**8 => a 20 bits key)
    // This Map could be optmized to only handle MAX_LZW_CODE * 2**(m_bpp)
    m_strmap = (int *)new int[1 << 20];
}

StringTable::~StringTable()
{
    if (m_buffer != nullptr) {
        delete[] m_buffer;
    }
    if (m_strmap != nullptr) {
        delete[] m_strmap;
        m_strmap = nullptr;
    }
}

void StringTable::Initialize(int minCodeSize)
{
    m_done = false;

    m_bpp = 8;
    m_minCodeSize = minCodeSize;
    m_clearCode = 1 << m_minCodeSize;
    if (m_clearCode > MAX_LZW_CODE) {
        m_clearCode = MAX_LZW_CODE;
    }
    m_endCode = m_clearCode + 1;

    m_partial = 0;
    m_partialSize = 0;

    m_bufferSize = 0;
    ClearCompressorTable();
    ClearDecompressorTable();
}

BYTE *StringTable::FillInputBuffer(int len)
{
    if (m_buffer == nullptr) {
        m_buffer = new BYTE[len];
        m_bufferRealSize = len;
    } else if (len > m_bufferRealSize) {
        delete[] m_buffer;
        m_buffer = new BYTE[len];
        m_bufferRealSize = len;
    }
    m_bufferSize = len;
    m_bufferPos = 0;
    m_bufferShift = 8 - m_bpp;
    return m_buffer;
}

void StringTable::CompressStart(int bpp, int width)
{
    m_bpp = bpp;
    m_slack = (8 - ((width * bpp) % 8)) % 8;

    m_partial |= m_clearCode << m_partialSize;
    m_partialSize += m_codeSize;
    ClearCompressorTable();
}

int StringTable::CompressEnd(BYTE *buf)
{
    int len = 0;

    // output code for remaining prefix
    m_partial |= m_prefix << m_partialSize;
    m_partialSize += m_codeSize;
    while (m_partialSize >= 8) {
        *buf++ = (BYTE)m_partial;
        m_partial >>= 8;
        m_partialSize -= 8;
        len++;
    }

    // add the end of information code and flush the entire buffer out
    m_partial |= m_endCode << m_partialSize;
    m_partialSize += m_codeSize;
    while (m_partialSize > 0) {
        *buf++ = (BYTE)m_partial;
        m_partial >>= 8;
        m_partialSize -= 8;
        len++;
    }

    // most this can be is 4 bytes.  7 bits in m_partial to start + 12 for the
    // last code + 12 for the end code = 31 bits total.
    return len;
}

bool StringTable::Compress(BYTE *buf, int *len)
{
    if (m_bufferSize == 0 || m_done) {
        return false;
    }

    int mask = (1 << m_bpp) - 1;
    BYTE *bufpos = buf;
    while (m_bufferPos < m_bufferSize) {
        // get the current pixel value
        char ch = (char)((m_buffer[m_bufferPos] >> m_bufferShift) & mask);

        // The next prefix is :
        // <the previous LZW code (on 12 bits << 8)> | <the code of the current pixel (on 8 bits)>
        int nextprefix = (((m_prefix) << 8) & 0xFFF00) + (ch & 0x000FF);
        if (firstPixelPassed) {

            if (m_strmap[nextprefix] > 0) {
                m_prefix = m_strmap[nextprefix];
            } else {
                m_partial |= m_prefix << m_partialSize;
                m_partialSize += m_codeSize;
                // grab full bytes for the output buffer
                while (m_partialSize >= 8 && bufpos - buf < *len) {
                    *bufpos++ = (BYTE)m_partial;
                    m_partial >>= 8;
                    m_partialSize -= 8;
                }

                // add the code to the "table map"
                m_strmap[nextprefix] = m_nextCode;

                // increment the next highest valid code, increase the code size
                if (m_nextCode == (1 << m_codeSize)) {
                    m_codeSize++;
                }
                m_nextCode++;

                // if we're out of codes, restart the string table
                if (m_nextCode == MAX_LZW_CODE) {
                    m_partial |= m_clearCode << m_partialSize;
                    m_partialSize += m_codeSize;
                    ClearCompressorTable();
                }

                // Only keep the 8 lowest bits (prevent problems with "negative chars")
                m_prefix = ch & 0x000FF;
            }

            // increment to the next pixel
            if (m_bufferShift > 0
                && !(m_bufferPos + 1 == m_bufferSize && m_bufferShift <= m_slack)) {
                m_bufferShift -= m_bpp;
            } else {
                m_bufferPos++;
                m_bufferShift = 8 - m_bpp;
            }

            // jump out here if the output buffer is full
            if (bufpos - buf == *len) {
                return true;
            }

        } else {
            // Specific behavior for the first pixel of the whole image

            firstPixelPassed = 1;
            // Only keep the 8 lowest bits (prevent problems with "negative chars")
            m_prefix = ch & 0x000FF;

            // increment to the next pixel
            if (m_bufferShift > 0
                && !(m_bufferPos + 1 == m_bufferSize && m_bufferShift <= m_slack)) {
                m_bufferShift -= m_bpp;
            } else {
                m_bufferPos++;
                m_bufferShift = 8 - m_bpp;
            }

            // jump out here if the output buffer is full
            if (bufpos - buf == *len) {
                return true;
            }
        }
    }

    m_bufferSize = 0;
    *len = (int)(bufpos - buf);

    return true;
}

bool StringTable::Decompress(BYTE *buf, int *len)
{
    if (m_bufferSize == 0 || m_done) {
        return false;
    }

    BYTE *bufpos = buf;
    for (; m_bufferPos < m_bufferSize; m_bufferPos++) {
        m_partial |= (int)m_buffer[m_bufferPos] << m_partialSize;
        m_partialSize += 8;
        while (m_partialSize >= m_codeSize) {
            int code = m_partial & m_codeMask;
            m_partial >>= m_codeSize;
            m_partialSize -= m_codeSize;

            if (code > m_nextCode || (m_nextCode == MAX_LZW_CODE && code != m_clearCode)
                || code == m_endCode) {
                m_done = true;
                *len = (int)(bufpos - buf);
                return true;
            }
            if (code == m_clearCode) {
                ClearDecompressorTable();
                continue;
            }

            // add new string to string table, if not the first pass since a clear code
            if (m_oldCode != MAX_LZW_CODE) {
                m_strings[m_nextCode] =
                    m_strings[m_oldCode] + m_strings[code == m_nextCode ? m_oldCode : code][0];
            }

            if ((int)m_strings[code].size() > *len - (bufpos - buf)) {
                // out of space, stuff the code back in for next time
                m_partial <<= m_codeSize;
                m_partialSize += m_codeSize;
                m_partial |= code;
                m_bufferPos++;
                *len = (int)(bufpos - buf);
                return true;
            }

            // output the string into the buffer
            memcpy(bufpos, m_strings[code].data(), m_strings[code].size());
            bufpos += m_strings[code].size();

            // increment the next highest valid code, add a bit to the mask if we need to increase
            // the code size
            if (m_oldCode != MAX_LZW_CODE && m_nextCode < MAX_LZW_CODE) {
                if (++m_nextCode < MAX_LZW_CODE) {
                    if ((m_nextCode & m_codeMask) == 0) {
                        m_codeSize++;
                        m_codeMask |= m_nextCode;
                    }
                }
            }

            m_oldCode = code;
        }
    }

    m_bufferSize = 0;
    *len = (int)(bufpos - buf);

    return true;
}

void StringTable::Done(void)
{
    m_done = true;
}

void StringTable::ClearCompressorTable(void)
{
    if (m_strmap) {
        memset(m_strmap, 0xFF, sizeof(unsigned int) * (1 << 20));
    }
    m_nextCode = m_endCode + 1;

    m_prefix = 0;
    m_codeSize = m_minCodeSize + 1;
}

void StringTable::ClearDecompressorTable(void)
{
    for (int i = 0; i < m_clearCode; i++) {
        m_strings[i].resize(1);
        m_strings[i][0] = (char)i;
    }
    m_nextCode = m_endCode + 1;

    m_codeSize = m_minCodeSize + 1;
    m_codeMask = (1 << m_codeSize) - 1;
    m_oldCode = MAX_LZW_CODE;
}

// ==========================================================
// Plugin Interface
// ==========================================================

static int s_format_id;

// ==========================================================
// Plugin Implementation
// ==========================================================

static BOOL DLL_CALLCONV Validate(FreeImageIO *io, fi_handle handle)
{
    char buf[6];
    if (io->read_proc(buf, 6, 1, handle) < 1) {
        return FALSE;
    }

    BOOL bResult = FALSE;
    if (!strncmp(buf, "GIF", 3)) {
        if (buf[3] >= '0' && buf[3] <= '9' && buf[4] >= '0' && buf[4] <= '9' && buf[5] >= 'a'
            && buf[5] <= 'z') {
            bResult = TRUE;
        }
    }

    io->seek_proc(handle, -6, SEEK_CUR);

    return bResult;
}

// ----------------------------------------------------------

static void *DLL_CALLCONV Open(FreeImageIO *io, fi_handle handle)
{
    GIFinfo *info = new GIFinfo;
    if (info == nullptr) {
        return nullptr;
    }
    BOOL read = TRUE;

    // 25/02/2008 MDA:	Not safe to memset GIFinfo structure with VS 2008 (safe iterators),
    //					perform initialization in constructor instead.
    // memset(info, 0, sizeof(GIFinfo));

    info->read = read;
    try {
        // Header
        if (!Validate(io, handle)) {
            throw "Not a GIF file";
        }
        io->seek_proc(handle, 6, SEEK_CUR);

        // Logical Screen Descriptor
        io->seek_proc(handle, 4, SEEK_CUR);
        BYTE packed;
        if (io->read_proc(&packed, 1, 1, handle) < 1) {
            throw "EOF reading Logical Screen Descriptor";
        }
        if (io->read_proc(&info->background_color, 1, 1, handle) < 1) {
            throw "EOF reading Logical Screen Descriptor";
        }
        io->seek_proc(handle, 1, SEEK_CUR);

        // Global Color Table
        if (packed & GIF_PACKED_LSD_HAVEGCT) {
            info->global_color_table_offset = io->tell_proc(handle);
            info->global_color_table_size = 2 << (packed & GIF_PACKED_LSD_GCTSIZE);
            io->seek_proc(handle, 3 * info->global_color_table_size, SEEK_CUR);
        }

        // Scan through all the rest of the blocks, saving offsets
        size_t gce_offset = 0;
        BYTE block = 0;
        while (block != GIF_BLOCK_TRAILER) {
            if (io->read_proc(&block, 1, 1, handle) < 1) {
                throw "EOF reading blocks";
            }
            if (block == GIF_BLOCK_IMAGE_DESCRIPTOR) {
                info->image_descriptor_offsets.push_back(io->tell_proc(handle));
                // GCE may be 0, meaning no GCE preceded this ID
                info->graphic_control_extension_offsets.push_back(gce_offset);
                gce_offset = 0;

                io->seek_proc(handle, 8, SEEK_CUR);
                if (io->read_proc(&packed, 1, 1, handle) < 1) {
                    throw "EOF reading Image Descriptor";
                }

                // Local Color Table
                if (packed & GIF_PACKED_ID_HAVELCT) {
                    io->seek_proc(handle, 3 * (2 << (packed & GIF_PACKED_ID_LCTSIZE)), SEEK_CUR);
                }

                // LZW Minimum Code Size
                io->seek_proc(handle, 1, SEEK_CUR);
            } else if (block == GIF_BLOCK_EXTENSION) {
                BYTE ext;
                if (io->read_proc(&ext, 1, 1, handle) < 1) {
                    throw "EOF reading extension";
                }

                if (ext == GIF_EXT_GRAPHIC_CONTROL) {
                    // overwrite previous offset if more than one GCE found before an ID
                    gce_offset = io->tell_proc(handle);
                } else if (ext == GIF_EXT_COMMENT) {
                    info->comment_extension_offsets.push_back(io->tell_proc(handle));
                } else if (ext == GIF_EXT_APPLICATION) {
                    info->application_extension_offsets.push_back(io->tell_proc(handle));
                }
            } else if (block == GIF_BLOCK_TRAILER) {
                continue;
            } else {
                throw "Invalid GIF block found";
            }

            // Data Sub-blocks
            BYTE len;
            if (io->read_proc(&len, 1, 1, handle) < 1) {
                throw "EOF reading sub-block";
            }
            while (len != 0) {
                io->seek_proc(handle, len, SEEK_CUR);
                if (io->read_proc(&len, 1, 1, handle) < 1) {
                    throw "EOF reading sub-block";
                }
            }
        }
    } catch (const char *msg) {
        FreeImage_OutputMessageProc(s_format_id, msg, io);
        delete info;
        return nullptr;
    }

    return info;
}

static FIBITMAP *DLL_CALLCONV DoLoadGIF(FreeImageIO *io, fi_handle handle, int flags, void *data)
{
    if (data == nullptr) {
        return nullptr;
    }
    (void)flags;
    GIFinfo *info = (GIFinfo *)data;

    int page = 0;
    FIBITMAP *dib = nullptr;
    try {
        bool have_transparent = false, no_local_palette = false, interlaced = false;
        int disposal_method = GIF_DISPOSAL_LEAVE, transparent_color = 0;
        WORD left, top, width, height;
        BYTE packed, b;
        WORD w;

        // Image Descriptor
        io->seek_proc(handle, (long)info->image_descriptor_offsets[page], SEEK_SET);
        io->read_proc(&left, 2, 1, handle);
        io->read_proc(&top, 2, 1, handle);
        io->read_proc(&width, 2, 1, handle);
        io->read_proc(&height, 2, 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
        SwapShort(&left);
        SwapShort(&top);
        SwapShort(&width);
        SwapShort(&height);
#endif
        io->read_proc(&packed, 1, 1, handle);
        interlaced = (packed & GIF_PACKED_ID_INTERLACED) ? true : false;
        no_local_palette = (packed & GIF_PACKED_ID_HAVELCT) ? false : true;

        int bpp = 8;
        if (!no_local_palette) {
            int size = 2 << (packed & GIF_PACKED_ID_LCTSIZE);
            if (size <= 2)
                bpp = 1;
            else if (size <= 16)
                bpp = 4;
        } else if (info->global_color_table_offset != 0) {
            if (info->global_color_table_size <= 2)
                bpp = 1;
            else if (info->global_color_table_size <= 16)
                bpp = 4;
        }
        dib = FreeImage_Allocate(width, height, bpp, io);
        if (dib == nullptr) {
            throw "DIB allocated failed";
        }

        // Palette
        RGBQUAD *pal = FreeImage_GetPalette(dib);
        if (!no_local_palette) {
            int size = 2 << (packed & GIF_PACKED_ID_LCTSIZE);

            int i = 0;
            while (i < size) {
                io->read_proc(&pal[i].rgbRed, 1, 1, handle);
                io->read_proc(&pal[i].rgbGreen, 1, 1, handle);
                io->read_proc(&pal[i].rgbBlue, 1, 1, handle);
                i++;
            }
        } else if (info->global_color_table_offset != 0) {
            long pos = io->tell_proc(handle);
            io->seek_proc(handle, (long)info->global_color_table_offset, SEEK_SET);

            int i = 0;
            while (i < info->global_color_table_size) {
                io->read_proc(&pal[i].rgbRed, 1, 1, handle);
                io->read_proc(&pal[i].rgbGreen, 1, 1, handle);
                io->read_proc(&pal[i].rgbBlue, 1, 1, handle);
                i++;
            }

            io->seek_proc(handle, pos, SEEK_SET);
        } else {
            // its legal to have no palette, but we're going to generate *something*
            for (int i = 0; i < 256; i++) {
                pal[i].rgbRed = (BYTE)i;
                pal[i].rgbGreen = (BYTE)i;
                pal[i].rgbBlue = (BYTE)i;
            }
        }

        // LZW Minimum Code Size
        io->read_proc(&b, 1, 1, handle);
        StringTable *stringtable = new StringTable;
        stringtable->Initialize(b);

        // Image Data Sub-blocks
        int x = 0, xpos = 0, y = 0, shift = 8 - bpp, mask = (1 << bpp) - 1, interlacepass = 0;
        BYTE *scanline = FreeImage_GetScanLine(dib, height - 1);
        BYTE buf[4096];
        io->read_proc(&b, 1, 1, handle);
        while (b) {
            io->read_proc(stringtable->FillInputBuffer(b), b, 1, handle);
            int size = sizeof(buf);
            while (stringtable->Decompress(buf, &size)) {
                for (int i = 0; i < size; i++) {
                    scanline[xpos] |= (buf[i] & mask) << shift;
                    if (shift > 0) {
                        shift -= bpp;
                    } else {
                        xpos++;
                        shift = 8 - bpp;
                    }
                    if (++x >= width) {
                        if (interlaced) {
                            y += g_GifInterlaceIncrement[interlacepass];
                            if (y >= height && ++interlacepass < GIF_INTERLACE_PASSES) {
                                y = g_GifInterlaceOffset[interlacepass];
                            }
                        } else {
                            y++;
                        }
                        if (y >= height) {
                            stringtable->Done();
                            break;
                        }
                        x = xpos = 0;
                        shift = 8 - bpp;
                        scanline = FreeImage_GetScanLine(dib, height - y - 1);
                    }
                }
                size = sizeof(buf);
            }
            io->read_proc(&b, 1, 1, handle);
        }

        if (page == 0) {
            quint32 idx;

            // Logical Screen Descriptor
            io->seek_proc(handle, 6, SEEK_SET);
            WORD logicalwidth, logicalheight;
            io->read_proc(&logicalwidth, 2, 1, handle);
            io->read_proc(&logicalheight, 2, 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
            SwapShort(&logicalwidth);
            SwapShort(&logicalheight);
#endif

            // Global Color Table
            if (info->global_color_table_offset != 0) {
                RGBQUAD globalpalette[256];
                io->seek_proc(handle, (long)info->global_color_table_offset, SEEK_SET);
                int i = 0;
                while (i < info->global_color_table_size) {
                    io->read_proc(&globalpalette[i].rgbRed, 1, 1, handle);
                    io->read_proc(&globalpalette[i].rgbGreen, 1, 1, handle);
                    io->read_proc(&globalpalette[i].rgbBlue, 1, 1, handle);
                    globalpalette[i].rgbReserved = 0;
                    i++;
                }
                // background color
                if (info->background_color < info->global_color_table_size) {
                    FreeImage_SetBackgroundColor(dib, &globalpalette[info->background_color]);
                }
            }

            // Application Extension
            LONG loop = 1; // If no AE with a loop count is found, the default must be 1
            for (idx = 0; idx < info->application_extension_offsets.size(); idx++) {
                io->seek_proc(handle, (long)info->application_extension_offsets[idx], SEEK_SET);
                io->read_proc(&b, 1, 1, handle);
                if (b == 11) { // All AEs start with an 11 byte sub-block to determine what type of
                               // AE it is
                    char buf[11];
                    io->read_proc(buf, 11, 1, handle);
                    if (!memcmp(buf, "NETSCAPE2.0", 11)
                        || !memcmp(buf, "ANIMEXTS1.0",
                                   11)) { // Not everybody recognizes ANIMEXTS1.0 but it is valid
                        io->read_proc(&b, 1, 1, handle);
                        if (b == 3) { // we're supposed to have a 3 byte sub-block now
                            io->read_proc(&b, 1, 1,
                                          handle); // this should be 0x01 but isn't really important
                            io->read_proc(&w, 2, 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
                            SwapShort(&w);
#endif
                            loop = w;
                            if (loop > 0)
                                loop++;
                            break;
                        }
                    }
                }
            }
        }

        // Graphic Control Extension
        if (info->graphic_control_extension_offsets[page] != 0) {
            io->seek_proc(handle, (long)(info->graphic_control_extension_offsets[page] + 1),
                          SEEK_SET);
            io->read_proc(&packed, 1, 1, handle);
            io->read_proc(&w, 2, 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
            SwapShort(&w);
#endif
            io->read_proc(&b, 1, 1, handle);
            have_transparent = (packed & GIF_PACKED_GCE_HAVETRANS) ? true : false;
            disposal_method = (packed & GIF_PACKED_GCE_DISPOSAL) >> 2;

            transparent_color = b;
            if (have_transparent) {
                int size = 1 << bpp;
                if (transparent_color <= size) {
                    BYTE table[256];
                    memset(table, 0xFF, size);
                    table[transparent_color] = 0;
                    FreeImage_SetTransparencyTable(dib, table, size);
                    dib->m_TransparentPaletteIndex = b;
                }
            }
        }
        b = (BYTE)disposal_method;

        delete stringtable;

    } catch (const char *msg) {
        if (dib != nullptr) {
            FreeImage_Unload(dib);
        }
        FreeImage_OutputMessageProc(s_format_id, msg, io);
        return nullptr;
    }

    return dib;
}

static void DLL_CALLCONV Close(void *data)
{
    if (data == nullptr) {
        return;
    }
    GIFinfo *info = (GIFinfo *)data;
    delete info;
}

SLoadedTexture *SLoadedTexture::LoadGIF(ISeekableIOStream &inStream, bool inFlipY,
                                        NVFoundationBase &inFnd,
                                        QDemonRenderContextType renderContextType)
{
    Q_UNUSED(renderContextType)
    FreeImageIO theIO(inFnd.getAllocator(), inFnd);
    void *gifData = Open(&theIO, &inStream);
    if (gifData) {
        SLoadedTexture *retval = DoLoadGIF(&theIO, &inStream, 0, gifData);
        Close(gifData);
        if (retval)
            retval->FreeImagePostProcess(inFlipY);
        return retval;
    }
    return nullptr;
}
