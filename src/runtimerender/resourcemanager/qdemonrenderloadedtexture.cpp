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
#include <QtDemonRuntimeRender/qdemonrenderloadedtexture.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrenderimagescaler.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtGui/QImage>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRef<QDemonLoadedTexture> QDemonLoadedTexture::loadQImage(const QString &inPath, qint32 flipVertical,
                                           QDemonRenderContextType renderContextType)
{
    Q_UNUSED(flipVertical)
    Q_UNUSED(renderContextType)
    QDemonRef<QDemonLoadedTexture> retval(nullptr);
    QImage image(inPath);
    image = image.mirrored();
    image = image.rgbSwapped();
    retval.reset(new QDemonLoadedTexture);
    retval->width = image.width();
    retval->height = image.height();
    retval->components = image.pixelFormat().channelCount();
    retval->image = image;
    retval->data = (void*)retval->image.bits();
    retval->dataSizeInBytes = image.sizeInBytes();
    retval->setFormatFromComponents();
    return retval;
}


namespace {

/**
        !!Large section of code ripped from FreeImage!!

*/
// ----------------------------------------------------------
//   Structures used by DXT textures
// ----------------------------------------------------------
typedef struct tagColor8888
{
    quint8 b;
    quint8 g;
    quint8 r;
    quint8 a;
} Color8888;

typedef struct tagColor565
{
    quint16 b : 5;
    quint16 g : 6;
    quint16 r : 5;
} Color565;

typedef struct tagDXTColBlock
{
    Color565 colors[2];
    quint8 row[4];
} DXTColBlock;

typedef struct tagDXTAlphaBlockExplicit
{
    quint16 row[4];
} DXTAlphaBlockExplicit;

typedef struct tagDXTAlphaBlock3BitLinear
{
    quint8 alpha[2];
    quint8 data[6];
} DXTAlphaBlock3BitLinear;

typedef struct tagDXT1Block
{
    DXTColBlock color;
} DXT1Block;

typedef struct tagDXT3Block
{ // also used by dxt2
    DXTAlphaBlockExplicit alpha;
    DXTColBlock color;
} DXT3Block;

typedef struct tagDXT5Block
{ // also used by dxt4
    DXTAlphaBlock3BitLinear alpha;
    DXTColBlock color;
} DXT5Block;

static void getBlockColors(const DXTColBlock &block, Color8888 colors[4], bool isDXT1)
{
    int i;
    for (i = 0; i < 2; i++) {
        colors[i].a = 0xff;
        colors[i].r = (quint8)(block.colors[i].r * 0xff / 0x1f);
        colors[i].g = (quint8)(block.colors[i].g * 0xff / 0x3f);
        colors[i].b = (quint8)(block.colors[i].b * 0xff / 0x1f);
    }

    quint16 *wCol = (quint16 *)block.colors;
    if (wCol[0] > wCol[1] || !isDXT1) {
        // 4 color block
        for (i = 0; i < 2; i++) {
            colors[i + 2].a = 0xff;
            colors[i + 2].r =
                (quint8)((quint16(colors[0].r) * (2 - i) + quint16(colors[1].r) * (1 + i)) / 3);
            colors[i + 2].g =
                (quint8)((quint16(colors[0].g) * (2 - i) + quint16(colors[1].g) * (1 + i)) / 3);
            colors[i + 2].b =
                (quint8)((quint16(colors[0].b) * (2 - i) + quint16(colors[1].b) * (1 + i)) / 3);
        }
    } else {
        // 3 color block, number 4 is transparent
        colors[2].a = 0xff;
        colors[2].r = (quint8)((quint16(colors[0].r) + quint16(colors[1].r)) / 2);
        colors[2].g = (quint8)((quint16(colors[0].g) + quint16(colors[1].g)) / 2);
        colors[2].b = (quint8)((quint16(colors[0].b) + quint16(colors[1].b)) / 2);

        colors[3].a = 0x00;
        colors[3].g = 0x00;
        colors[3].b = 0x00;
        colors[3].r = 0x00;
    }
}

struct DXT_INFO_1
{
    typedef DXT1Block Block;
    enum { isDXT1 = 1, bytesPerBlock = 8 };
};

struct DXT_INFO_3
{
    typedef DXT3Block Block;
    enum { isDXT1 = 1, bytesPerBlock = 16 };
};

struct DXT_INFO_5
{
    typedef DXT5Block Block;
    enum { isDXT1 = 1, bytesPerBlock = 16 };
};

template <class INFO>
class DXT_BLOCKDECODER_BASE
{
protected:
    Color8888 m_colors[4];
    const typename INFO::Block *m_pBlock;
    unsigned m_colorRow;

public:
    void setup(const quint8 *pBlock)
    {
        m_pBlock = (const typename INFO::Block *)pBlock;
        getBlockColors(m_pBlock->color, m_colors, INFO::isDXT1);
    }

    void setY(int y) { m_colorRow = m_pBlock->color.row[y]; }

    void getColor(int x, int y, Color8888 &color)
    {
        Q_UNUSED(y)
        unsigned bits = (m_colorRow >> (x * 2)) & 3;
        color = m_colors[bits];
        std::swap(color.r, color.b);
    }
};

class DXT_BLOCKDECODER_1 : public DXT_BLOCKDECODER_BASE<DXT_INFO_1>
{
public:
    typedef DXT_INFO_1 INFO;
};

class DXT_BLOCKDECODER_3 : public DXT_BLOCKDECODER_BASE<DXT_INFO_3>
{
public:
    typedef DXT_BLOCKDECODER_BASE<DXT_INFO_3> base;
    typedef DXT_INFO_3 INFO;

protected:
    unsigned m_alphaRow;

public:
    void setY(int y)
    {
        base::setY(y);
        m_alphaRow = m_pBlock->alpha.row[y];
    }

    void getColor(int x, int y, Color8888 &color)
    {
        base::getColor(x, y, color);
        const unsigned bits = (m_alphaRow >> (x * 4)) & 0xF;
        color.a = (quint8)((bits * 0xFF) / 0xF);
    }
};

class DXT_BLOCKDECODER_5 : public DXT_BLOCKDECODER_BASE<DXT_INFO_5>
{
public:
    typedef DXT_BLOCKDECODER_BASE<DXT_INFO_5> base;
    typedef DXT_INFO_5 INFO;

protected:
    unsigned m_alphas[8];
    unsigned m_alphaBits;
    int m_offset;

public:
    void setup(const quint8 *pBlock)
    {
        base::setup(pBlock);

        const DXTAlphaBlock3BitLinear &block = m_pBlock->alpha;
        m_alphas[0] = block.alpha[0];
        m_alphas[1] = block.alpha[1];
        if (m_alphas[0] > m_alphas[1]) {
            // 8 alpha block
            for (int i = 0; i < 6; i++) {
                m_alphas[i + 2] = ((6 - i) * m_alphas[0] + (1 + i) * m_alphas[1] + 3) / 7;
            }
        } else {
            // 6 alpha block
            for (int i = 0; i < 4; i++) {
                m_alphas[i + 2] = ((4 - i) * m_alphas[0] + (1 + i) * m_alphas[1] + 2) / 5;
            }
            m_alphas[6] = 0;
            m_alphas[7] = 0xFF;
        }
    }

    void setY(int y)
    {
        base::setY(y);
        int i = y / 2;
        const DXTAlphaBlock3BitLinear &block = m_pBlock->alpha;
        m_alphaBits = unsigned(block.data[0 + i * 3]) | (unsigned(block.data[1 + i * 3]) << 8)
            | (unsigned(block.data[2 + i * 3]) << 16);
        m_offset = (y & 1) * 12;
    }

    void getColor(int x, int y, Color8888 &color)
    {
        base::getColor(x, y, color);
        unsigned bits = (m_alphaBits >> (x * 3 + m_offset)) & 7;
        color.a = (quint8)m_alphas[bits];
        std::swap(color.r, color.b);
    }
};

template <class DECODER>
void decodeDXTBlock(quint8 *dstData, const quint8 *srcBlock, long dstPitch, int bw, int bh)
{
    DECODER decoder;
    decoder.Setup(srcBlock);
    for (int y = 0; y < bh; y++) {
        // Note that this assumes the pointer is pointing to the *last* valid start
        // row.
        quint8 *dst = dstData - y * dstPitch;
        decoder.SetY(y);
        for (int x = 0; x < bw; x++) {
            decoder.GetColor(x, y, (Color8888 &)*dst);
            dst += 4;
        }
    }
}

struct QDemonTextureDataWriter
{
    quint32 m_width;
    quint32 m_height;
    quint32 m_stride;
    quint32 m_numComponents;
    QDemonTextureData &m_textureData;
    QDemonTextureDataWriter(quint32 w, quint32 h, bool hasA, QDemonTextureData &inTd)
        : m_width(w)
        , m_height(h)
        , m_stride(hasA ? m_width * 4 : m_width * 3)
        , m_numComponents(hasA ? 4 : 3)
        , m_textureData(inTd)
    {
        quint32 dataSize = m_stride * m_height;
        if (dataSize > m_textureData.dataSizeInBytes) {
            ::free(m_textureData.data);
            m_textureData.data = malloc(dataSize);
            m_textureData.dataSizeInBytes = dataSize;
        }
        memZero(m_textureData.data, m_textureData.dataSizeInBytes);
        m_textureData.format = hasA ? QDemonRenderTextureFormats::RGBA8 : QDemonRenderTextureFormats::RGB8;
    }

    void writePixel(quint32 X, quint32 Y, quint8 *pixelData)
    {
        if (X < m_width && Y < m_height) {
            char *textureData = reinterpret_cast<char *>(m_textureData.data);
            quint32 offset = Y * m_stride + X * m_numComponents;

            for (quint32 idx = 0; idx < m_numComponents; ++idx)
                Q_ASSERT(textureData[offset + idx] == 0);

            ::memcpy(textureData + offset, pixelData, m_numComponents);
        }
    }

    // Incoming pixels are assumed to be RGBA or RGBX, 32 bit in any case
    void writeBlock(quint32 X, quint32 Y, quint32 width, quint32 height, quint8 *pixelData)
    {
        quint32 offset = 0;
        for (quint32 yidx = 0; yidx < height; ++yidx) {
            for (quint32 xidx = 0; xidx < width; ++xidx, offset += 4) {
                writePixel(X + xidx, Y + (height - yidx - 1), pixelData + offset);
            }
        }
    }
    bool finished() { return false; }
};

//struct QDemonTextureAlphaScanner
//{
//    bool &alpha;

//    void writeBlock(quint32 X, quint32 Y, quint32 width, quint32 height, quint8 *pixelData)
//    {
//        Q_UNUSED(X)
//        Q_UNUSED(Y)
//        quint32 offset = 0;
//        for (quint32 yidx = 0; yidx < height; ++yidx) {
//            for (quint32 xidx = 0; xidx < width; ++xidx, offset += 4) {
//                if (pixelData[offset + 3] < 255)
//                    alpha = true;
//            }
//        }
//    }

//    // If we detect alpha we can stop right there.
//    bool finished() { return alpha; }
//};
// Scan the dds image's mipmap 0 level for alpha.
template <class DECODER, class TWriterType>
static void decompressDds(void *inSrc,
                          quint32 inDataSize,
                          quint32 inWidth,
                          quint32 inHeight,
                          TWriterType ioWriter)
{
    typedef typename DECODER::INFO INFO;
    typedef typename INFO::Block Block;
    (void)inDataSize;

    const quint8 *pbSrc = (const quint8 *)inSrc;
    // Each DX block is composed of 16 pixels.  Free image decodes those
    // pixels into a 4x4 block of data.
    quint8 pbDstData[4 * 4 * 4];
    // The decoder decodes backwards
    // So we need to point to the last line.
    quint8 *pbDst = pbDstData + 48;

    int width = (int)inWidth;
    int height = (int)inHeight;
    int lineStride = 16;
    for (int y = 0; y < height && ioWriter.Finished() == false; y += 4) {
        int yPixels = qMin(height - y, 4);
        for (int x = 0; x < width && ioWriter.Finished() == false; x += 4) {
            int xPixels = qMin(width - x, 4);
            decodeDXTBlock<DECODER>(pbDst, pbSrc, lineStride, xPixels, yPixels);
            pbSrc += INFO::bytesPerBlock;
            ioWriter.WriteBlock(x, y, xPixels, yPixels, pbDstData);
        }
    }
}

bool scanImageForAlpha(const void *inData,
                       quint32 inWidth,
                       quint32 inHeight,
                       quint32 inPixelSizeInBytes,
                       quint8 inAlphaSizeInBits)
{
    const quint8 *rowPtr = reinterpret_cast<const quint8 *>(inData);
    bool hasAlpha = false;
    if (inAlphaSizeInBits == 0)
        return hasAlpha;
    if (inPixelSizeInBytes != 2 && inPixelSizeInBytes != 4) {
        Q_ASSERT(false);
        return false;
    }
    if (inAlphaSizeInBits > 8) {
        Q_ASSERT(false);
        return false;
    }

    quint32 alphaRightShift = inPixelSizeInBytes * 8 - inAlphaSizeInBits;
    quint32 maxAlphaValue = (1 << inAlphaSizeInBits) - 1;

    for (quint32 rowIdx = 0; rowIdx < inHeight && hasAlpha == false; ++rowIdx) {
        for (quint32 idx = 0; idx < inWidth && hasAlpha == false;
             ++idx, rowPtr += inPixelSizeInBytes) {
            quint32 pixelValue = 0;
            if (inPixelSizeInBytes == 2)
                pixelValue = *(reinterpret_cast<const quint16 *>(rowPtr));
            else
                pixelValue = *(reinterpret_cast<const quint32 *>(rowPtr));
            pixelValue = pixelValue >> alphaRightShift;
            if (pixelValue < maxAlphaValue)
                hasAlpha = true;
        }
    }
    return hasAlpha;
}
}

QDemonLoadedTexture::~QDemonLoadedTexture()
{
    if (data && image.sizeInBytes() <= 0) {
        ::free(data);
    }
    if (m_palette)
        ::free(m_palette);
    if (m_transparencyTable)
        ::free(m_transparencyTable);
}

bool QDemonLoadedTexture::scanForTransparency()
{
    switch (format) {
    case QDemonRenderTextureFormats::SRGB8A8:
    case QDemonRenderTextureFormats::RGBA8:
        if (!data) // dds
            return true;

        return scanImageForAlpha(data, width, height, 4, 8);
    // Scan the image.
    case QDemonRenderTextureFormats::SRGB8:
    case QDemonRenderTextureFormats::RGB8:
        return false;
    case QDemonRenderTextureFormats::RGB565:
        return false;
    case QDemonRenderTextureFormats::RGBA5551:
        if (!data) { // dds
            return true;
        } else {
            return scanImageForAlpha(data, width, height, 2, 1);
        }
    case QDemonRenderTextureFormats::Alpha8:
        return true;
    case QDemonRenderTextureFormats::Luminance8:
        return false;
    case QDemonRenderTextureFormats::LuminanceAlpha8:
        if (!data) // dds
            return true;

        return scanImageForAlpha(data, width, height, 2, 8);
    case QDemonRenderTextureFormats::RGB_DXT1:
        return false;
    case QDemonRenderTextureFormats::RGBA_DXT3:
    case QDemonRenderTextureFormats::RGBA_DXT1:
    case QDemonRenderTextureFormats::RGBA_DXT5:
        return false;
    case QDemonRenderTextureFormats::RGB9E5:
        return false;
    case QDemonRenderTextureFormats::RG32F:
    case QDemonRenderTextureFormats::RGB32F:
    case QDemonRenderTextureFormats::RGBA16F:
    case QDemonRenderTextureFormats::RGBA32F:
        // PKC TODO : For now, since IBL will be the main consumer, we'll just pretend there's no
        // alpha.
        // Need to do a proper scan down the line, but doing it for floats is a little different
        // from
        // integer scans.
        return false;
    default:
        break;
    }
    Q_ASSERT(false);
    return false;
}

void QDemonLoadedTexture::ensureMultiplerOfFour(const char *inPath)
{
    if (width % 4 || height % 4) {
        qCWarning(PERF_WARNING,
            "Image %s has non multiple of four width or height; perf hit for scaling", inPath);
        if (data) {
            quint32 newWidth = QDemonTextRendererInterface::nextMultipleOf4(width);
            quint32 newHeight = QDemonTextRendererInterface::nextMultipleOf4(height);
            quint32 newDataSize = newWidth * newHeight * components;
            quint8 *newData = static_cast<quint8 *>(::malloc(newDataSize));
            QDemonImageScaler theScaler;
            if (components == 4) {
                theScaler.fastExpandRowsAndColumns((unsigned char *)data, width, height, newData,
                                                   newWidth, newHeight);
            } else
                theScaler.expandRowsAndColumns((unsigned char *)data, width, height, newData,
                                               newWidth, newHeight, components);

            ::free(data);
            data = newData;
            width = newWidth;
            height = newHeight;
            dataSizeInBytes = newDataSize;
        }
    }
}

void QDemonLoadedTexture::releaseDecompressedTexture(QDemonTextureData inImage)
{
    if (inImage.data)
        ::free(inImage.data);
}

#ifndef EA_PLATFORM_WINDOWS
#define stricmp strcasecmp
#endif

QDemonRef<QDemonLoadedTexture> QDemonLoadedTexture::load(const QString &inPath,
                                                              QDemonInputStreamFactoryInterface &inFactory,
                                                              bool inFlipY,
                                                              QDemonRenderContextType renderContextType)
{
    if (inPath.isEmpty())
        return nullptr;

    QDemonRef<QDemonLoadedTexture> theLoadedImage = nullptr;
    QSharedPointer<QIODevice> theStream(inFactory.getStreamForFile(inPath));
    QString fileName;
    inFactory.getPathForFile(inPath, fileName);
    if (theStream && inPath.size() > 3) {
        if (inPath.endsWith(QStringLiteral("png"), Qt::CaseInsensitive)
                || inPath.endsWith(QStringLiteral("jpg"), Qt::CaseInsensitive)
                || inPath.endsWith(QStringLiteral("peg"), Qt::CaseInsensitive)
                || inPath.endsWith(QStringLiteral("ktx"), Qt::CaseInsensitive)
                || inPath.endsWith(QStringLiteral("gif"), Qt::CaseInsensitive)
                || inPath.endsWith(QStringLiteral("bmp"), Qt::CaseInsensitive)) {
            theLoadedImage = loadQImage(fileName, inFlipY, renderContextType);
//        } else if (inPath.endsWith("dds", Qt::CaseInsensitive)) {
//            theLoadedImage = LoadDDS(theStream, inFlipY, renderContextType);
//        } else if (inPath.endsWith("hdr", Qt::CaseInsensitive)) {
//            theLoadedImage = LoadHDR(theStream, renderContextType);
        } else {
            qCWarning(INTERNAL_ERROR, "Unrecognized image extension: %s", qPrintable(inPath));
        }
    }
    return theLoadedImage;
}

QT_END_NAMESPACE
