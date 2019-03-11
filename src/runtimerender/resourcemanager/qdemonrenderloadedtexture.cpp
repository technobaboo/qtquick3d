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

QDemonRef<QDemonLoadedTexture> QDemonLoadedTexture::loadQImage(const QString &inPath, qint32 flipVertical, QDemonRenderContextType renderContextType)
{
    Q_UNUSED(flipVertical)
    Q_UNUSED(renderContextType)
    QDemonRef<QDemonLoadedTexture> retval(nullptr);
    QImage image(inPath);
    image = image.mirrored();
    image = image.rgbSwapped();
    retval = new QDemonLoadedTexture;
    retval->width = image.width();
    retval->height = image.height();
    retval->components = image.pixelFormat().channelCount();
    retval->image = image;
    retval->data = (void *)retval->image.bits();
    retval->dataSizeInBytes = image.sizeInBytes();
    retval->setFormatFromComponents();
    return retval;
}

namespace {

bool scanImageForAlpha(const void *inData, quint32 inWidth, quint32 inHeight, quint32 inPixelSizeInBytes, quint8 inAlphaSizeInBits)
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

    for (quint32 rowIdx = 0; rowIdx < inHeight && !hasAlpha; ++rowIdx) {
        for (quint32 idx = 0; idx < inWidth && !hasAlpha; ++idx, rowPtr += inPixelSizeInBytes) {
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
    switch (format.format) {
    case QDemonRenderTextureFormat::SRGB8A8:
    case QDemonRenderTextureFormat::RGBA8:
        if (!data) // dds
            return true;

        return scanImageForAlpha(data, width, height, 4, 8);
    // Scan the image.
    case QDemonRenderTextureFormat::SRGB8:
    case QDemonRenderTextureFormat::RGB8:
        return false;
    case QDemonRenderTextureFormat::RGB565:
        return false;
    case QDemonRenderTextureFormat::RGBA5551:
        if (!data) { // dds
            return true;
        } else {
            return scanImageForAlpha(data, width, height, 2, 1);
        }
    case QDemonRenderTextureFormat::Alpha8:
        return true;
    case QDemonRenderTextureFormat::Luminance8:
        return false;
    case QDemonRenderTextureFormat::LuminanceAlpha8:
        if (!data) // dds
            return true;

        return scanImageForAlpha(data, width, height, 2, 8);
    case QDemonRenderTextureFormat::RGB_DXT1:
        return false;
    case QDemonRenderTextureFormat::RGBA_DXT3:
    case QDemonRenderTextureFormat::RGBA_DXT1:
    case QDemonRenderTextureFormat::RGBA_DXT5:
        return false;
    case QDemonRenderTextureFormat::RGB9E5:
        return false;
    case QDemonRenderTextureFormat::RG32F:
    case QDemonRenderTextureFormat::RGB32F:
    case QDemonRenderTextureFormat::RGBA16F:
    case QDemonRenderTextureFormat::RGBA32F:
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
        qCWarning(PERF_WARNING, "Image %s has non multiple of four width or height; perf hit for scaling", inPath);
        if (data) {
            quint32 newWidth = QDemonTextRendererInterface::nextMultipleOf4(width);
            quint32 newHeight = QDemonTextRendererInterface::nextMultipleOf4(height);
            quint32 newDataSize = newWidth * newHeight * components;
            quint8 *newData = static_cast<quint8 *>(::malloc(newDataSize));
            QDemonImageScaler theScaler;
            if (components == 4) {
                theScaler.fastExpandRowsAndColumns((unsigned char *)data, width, height, newData, newWidth, newHeight);
            } else
                theScaler.expandRowsAndColumns((unsigned char *)data, width, height, newData, newWidth, newHeight, components);

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
                                                         const QDemonRenderContextType &renderContextType)
{
    if (inPath.isEmpty())
        return nullptr;

    QDemonRef<QDemonLoadedTexture> theLoadedImage = nullptr;
    QSharedPointer<QIODevice> theStream(inFactory.getStreamForFile(inPath));
    QString fileName;
    inFactory.getPathForFile(inPath, fileName);
    if (theStream && inPath.size() > 3) {
        if (inPath.endsWith(QStringLiteral("png"), Qt::CaseInsensitive) || inPath.endsWith(QStringLiteral("jpg"), Qt::CaseInsensitive)
            || inPath.endsWith(QStringLiteral("peg"), Qt::CaseInsensitive)
            || inPath.endsWith(QStringLiteral("ktx"), Qt::CaseInsensitive) || inPath.endsWith(QStringLiteral("gif"), Qt::CaseInsensitive)
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
