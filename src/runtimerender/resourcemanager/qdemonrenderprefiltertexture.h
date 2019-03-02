/****************************************************************************
**
** Copyright (C) 2008-2016 NVIDIA Corporation.
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
#ifndef QDEMON_RENDER_PREFILTER_TEXTURE_H
#define QDEMON_RENDER_PREFILTER_TEXTURE_H

#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrenderloadedtexture.h>

QT_BEGIN_NAMESPACE

class QDemonRenderPrefilterTexture
{
public:
    QDemonRenderPrefilterTexture(QDemonRef<QDemonRenderContext> inQDemonRenderContext,
                                 qint32 inWidth,
                                 qint32 inHeight,
                                 QDemonRef<QDemonRenderTexture2D> inTexture,
                                 QDemonRenderTextureFormats::Enum inDestFormat);
    virtual ~QDemonRenderPrefilterTexture();

    virtual void build(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormats::Enum inFormat) = 0;

    static QDemonRef<QDemonRenderPrefilterTexture> create(QDemonRef<QDemonRenderContext> inQDemonRenderContext,
                                                               qint32 inWidth,
                                                               qint32 inHeight,
                                                               QDemonRef<QDemonRenderTexture2D> inTexture,
                                                               QDemonRenderTextureFormats::Enum inDestFormat);

protected:
    QDemonRef<QDemonRenderTexture2D> m_texture2D;
    QDemonRenderTextureFormats::Enum m_internalFormat;
    QDemonRenderTextureFormats::Enum m_destinationFormat;

    qint32 m_width;
    qint32 m_height;
    qint32 m_maxMipMapLevel;
    qint32 m_sizeOfFormat;
    qint32 m_sizeOfInternalFormat;
    qint32 m_internalNoOfComponent;
    qint32 m_noOfComponent;
    QDemonRef<QDemonRenderContext> m_renderContext;
};

class QDemonRenderPrefilterTextureCPU : public QDemonRenderPrefilterTexture
{
public:
    QDemonRenderPrefilterTextureCPU(QDemonRef<QDemonRenderContext> inQDemonRenderContext,
                                    qint32 inWidth,
                                    qint32 inHeight,
                                    QDemonRef<QDemonRenderTexture2D> inTexture,
                                    QDemonRenderTextureFormats::Enum inDestFormat);

    void build(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormats::Enum inFormat) override;

    QDemonTextureData createBsdfMipLevel(QDemonTextureData &inCurMipLevel, QDemonTextureData &inPrevMipLevel, qint32 width, qint32 height);

    int wrapMod(int a, int base);
    void getWrappedCoords(int &sX, int &sY, int width, int height);
};

class QDemonRenderPrefilterTextureCompute : public QDemonRenderPrefilterTexture
{
public:
    QDemonRenderPrefilterTextureCompute(QDemonRef<QDemonRenderContext> inQDemonRenderContext,
                                        qint32 inWidth,
                                        qint32 inHeight,
                                        QDemonRef<QDemonRenderTexture2D> inTexture,
                                        QDemonRenderTextureFormats::Enum inDestFormat);
    ~QDemonRenderPrefilterTextureCompute() override;

    void build(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormats::Enum inFormat) override;

private:
    void createLevel0Tex(void *inTextureData, qint32 inTextureDataSize, QDemonRenderTextureFormats::Enum inFormat);

    QDemonRef<QDemonRenderShaderProgram> m_bsdfProgram;
    QDemonRef<QDemonRenderShaderProgram> m_uploadProgram_RGBA8;
    QDemonRef<QDemonRenderShaderProgram> m_uploadProgram_RGB8;
    QDemonRef<QDemonRenderTexture2D> m_level0Tex;
    bool m_textureCreated;

    void createComputeProgram(QDemonRef<QDemonRenderContext> context);
    QDemonRef<QDemonRenderShaderProgram> getOrCreateUploadComputeProgram(QDemonRef<QDemonRenderContext> context, QDemonRenderTextureFormats::Enum inFormat);
};
QT_END_NAMESPACE

#endif
