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

#include "qdemonrendertexttextureatlas.h"

#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

namespace {

struct QDemonTextTextureAtlas : public QDemonTextTextureAtlasInterface
{
    static const qint32 TEXTURE_ATLAS_DIM = 256; // if you change this you need to adjust QDemonOnscreenTextRenderer size as well

    QDemonRef<QDemonTextRendererInterface> m_textRenderer;
    QDemonRef<QDemonRenderContext> m_renderContext;

    QDemonTextTextureAtlas(QDemonRef<QDemonTextRendererInterface> inRenderer, QDemonRef<QDemonRenderContext> inRenderContext)
        : m_textRenderer(inRenderer), m_renderContext(inRenderContext), m_textureAtlasInitialized(false), m_textureAtlas(nullptr)
    {
    }

    virtual ~QDemonTextTextureAtlas() override = default;

    TTextRenderAtlasDetailsAndTexture renderText(const QDemonTextRenderInfo &inText) override
    {
        QDemonRenderTextureAtlasDetails theDetails = m_textRenderer->renderText(inText);

        return TTextRenderAtlasDetailsAndTexture(theDetails, m_textureAtlas);
    }

    bool isInitialized() override { return m_textureAtlasInitialized && m_textureAtlas; }

    TTextTextureAtlasDetailsAndTexture prepareTextureAtlas() override
    {
        if (!m_textureAtlasInitialized && !m_textureAtlas) {
            // create the texture atlas entries
            qint32 count = m_textRenderer->createTextureAtlas();

            m_textureAtlas = m_renderContext->createTexture2D();
            if (m_textureAtlas && count) {
                m_textureAtlasInitialized = true;
                // m_textureAtlas->addRef();
                // if you change the size you need to adjust QDemonOnscreenTextRenderer too
                if (m_renderContext->getRenderContextType() == QDemonRenderContextValues::GLES2) {
                    m_textureAtlas->setTextureData(QDemonDataRef<quint8>(), 0, TEXTURE_ATLAS_DIM, TEXTURE_ATLAS_DIM, QDemonRenderTextureFormats::RGBA8);
                } else {
                    m_textureAtlas->setTextureData(QDemonDataRef<quint8>(), 0, TEXTURE_ATLAS_DIM, TEXTURE_ATLAS_DIM, QDemonRenderTextureFormats::Alpha8);
                }
                m_textureAtlas->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                m_textureAtlas->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                m_textureAtlas->setTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
                m_textureAtlas->setTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
                QDemonTextureDetails texTexDetails = m_textureAtlas->getTextureDetails();
                return TTextTextureAtlasDetailsAndTexture(QDemonTextTextureAtlasDetails(texTexDetails.height,
                                                                                        texTexDetails.height,
                                                                                        false,
                                                                                        count),
                                                          m_textureAtlas);
            }
        }

        return TTextTextureAtlasDetailsAndTexture(QDemonTextTextureAtlasDetails(), nullptr);
    }

private:
    bool m_textureAtlasInitialized;
    QDemonRef<QDemonRenderTexture2D> m_textureAtlas; // this is the actual texture which has application lifetime
};

} // namespace

QDemonTextTextureAtlasInterface::~QDemonTextTextureAtlasInterface() = default;

QDemonRef<QDemonTextTextureAtlasInterface> QDemonTextTextureAtlasInterface::createTextureAtlas(QDemonRef<QDemonTextRendererInterface> inTextRenderer,
                                                                                               QDemonRef<QDemonRenderContext> inRenderContext)
{
    return QDemonRef<QDemonTextTextureAtlasInterface>(new QDemonTextTextureAtlas(inTextRenderer, inRenderContext));
}

QT_END_NAMESPACE
