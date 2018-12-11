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
#ifndef QDEMON_RENDER_LOADED_TEXTURE_H
#define QDEMON_RENDER_LOADED_TEXTURE_H

#include <QtDemonRuntimeRender/qdemonsimpletypes.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRuntimeRender/qdemonrenderloadedtexturedds.h>
#include <QtGui/QImage>


QT_BEGIN_NAMESPACE
class IInputStreamFactory;

struct STextureData
{
    void *data;
    quint32 dataSizeInBytes;
    QDemonRenderTextureFormats::Enum format;
    STextureData()
        : data(nullptr)
        , dataSizeInBytes(0)
        , format(QDemonRenderTextureFormats::Unknown)
    {
    }
};
struct ExtendedTextureFormats
{
    enum Enum {
        NoExtendedFormat = 0,
        Palettized,
        CustomRGB,
    };
};
// Utility class used for loading image data from disk.
// Supports jpg, png, and dds.
struct SLoadedTexture : public NVReleasable
{
private:
    ~SLoadedTexture();

public:
    qint32 width;
    qint32 height;
    qint32 components;
    void *data;
    QImage image;
    quint32 dataSizeInBytes;
    QDemonRenderTextureFormats::Enum format;
    QDemonDDSImage *dds;
    ExtendedTextureFormats::Enum m_ExtendedFormat;
    // Used for palettized images.
    void *m_Palette;
    qint32 m_CustomMasks[3];
    int m_BitCount;
    char m_BackgroundColor[3];
    uint8_t *m_TransparencyTable;
    int32_t m_TransparentPaletteIndex;

    SLoadedTexture()
        : width(0)
        , height(0)
        , components(0)
        , data(nullptr)
        , image(0)
        , dataSizeInBytes(0)
        , format(QDemonRenderTextureFormats::RGBA8)
        , dds(nullptr)
        , m_ExtendedFormat(ExtendedTextureFormats::NoExtendedFormat)
        , m_Palette(nullptr)
        , m_BitCount(0)
        , m_TransparencyTable(nullptr)
        , m_TransparentPaletteIndex(-1)
    {
        m_CustomMasks[0] = 0;
        m_CustomMasks[1] = 0;
        m_CustomMasks[2] = 0;
        m_BackgroundColor[0] = 0;
        m_BackgroundColor[1] = 0;
        m_BackgroundColor[2] = 0;
    }
    void setFormatFromComponents()
    {
        switch (components) {
        case 1: // undefined, but in this context probably luminance
            format = QDemonRenderTextureFormats::Luminance8;
            break;
        case 2:
            format = QDemonRenderTextureFormats::LuminanceAlpha8;
            break;
        case 3:
            format = QDemonRenderTextureFormats::RGB8;
            break;

        default:
            // fallthrough intentional
        case 4:
            format = QDemonRenderTextureFormats::RGBA8;
            break;
        }
    }

    void EnsureMultiplerOfFour(const char *inPath);
    // Returns true if this image has a pixel less than 255.
    bool ScanForTransparency();

    // Not all video cards support dxt compression.  Giving the last image allows
    // this object to potentially reuse the memory
    STextureData DecompressDXTImage(int inMipMapIdx, STextureData *inOptLastImage = nullptr);
    void ReleaseDecompressedTexture(STextureData inImage);

    static SLoadedTexture *Load(const QString &inPath,
                                IInputStreamFactory &inFactory, bool inFlipY = true,
                                QDemonRenderContextType renderContextType
                                = QDemonRenderContextValues::NullContext);
    static SLoadedTexture *LoadDDS(IInStream &inStream, qint32 flipVertical,
                                   QDemonRenderContextType renderContextType);
    static SLoadedTexture *LoadBMP(ISeekableIOStream &inStream, bool inFlipY,
                                   QDemonRenderContextType renderContextType);
    static SLoadedTexture *LoadGIF(ISeekableIOStream &inStream, bool inFlipY,
                                   QDemonRenderContextType renderContextType);
    static SLoadedTexture *LoadHDR(ISeekableIOStream &inStream,
                                   QDemonRenderContextType renderContextType);

    static SLoadedTexture *LoadQImage(const QString &inPath, qint32 flipVertical,
                                      QDemonRenderContextType renderContextType);

private:
    // Implemented in the bmp loader.
    void FreeImagePostProcess(bool inFlipY);
};
QT_END_NAMESPACE

#endif
