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

#include <qdemonrendertexttextureatlas.h>
#include <qdemontextrenderer.h>
#include <Qt3DSAtomic.h>
#include <Qt3DSFoundation.h>
#include <Qt3DSBroadcastingAllocator.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

namespace {

struct STextTextureAtlas : public ITextTextureAtlas
{
    static const qint32 TEXTURE_ATLAS_DIM =
            256; // if you change this you need to adjust Qt3DSOnscreenTextRenderer size as well

    QDemonScopedRefCounted<ITextRenderer> m_TextRenderer;
    QDemonScopedRefCounted<QDemonRenderContext> m_RenderContext;

    STextTextureAtlas(ITextRenderer &inRenderer,
                      QDemonRenderContext &inRenderContext)
        : m_TextRenderer(inRenderer)
        , m_RenderContext(inRenderContext)
        , m_TextureAtlasInitialized(false)
        , m_textureAtlas(nullptr)
    {
    }

    virtual ~STextTextureAtlas()
    {
        // if (m_textureAtlas) {
        //     m_textureAtlas->release();
        // }
    }

    TTextRenderAtlasDetailsAndTexture RenderText(const STextRenderInfo &inText) override
    {
        SRenderTextureAtlasDetails theDetails = m_TextRenderer->RenderText(inText);

        return TTextRenderAtlasDetailsAndTexture(theDetails, m_textureAtlas);
    }

    bool IsInitialized() override { return m_TextureAtlasInitialized && m_textureAtlas; }

    TTextTextureAtlasDetailsAndTexture PrepareTextureAtlas() override
    {
        if (!m_TextureAtlasInitialized && !m_textureAtlas) {
            // create the texture atlas entries
            qint32 count = m_TextRenderer->CreateTextureAtlas();

            m_textureAtlas = m_RenderContext->CreateTexture2D();
            if (m_textureAtlas && count) {
                m_TextureAtlasInitialized = true;
                //m_textureAtlas->addRef();
                // if you change the size you need to adjust Qt3DSOnscreenTextRenderer too
                if (m_RenderContext->GetRenderContextType() == QDemonRenderContextValues::GLES2) {
                    m_textureAtlas->SetTextureData(QDemonDataRef<quint8>(), 0, TEXTURE_ATLAS_DIM,
                                                   TEXTURE_ATLAS_DIM, QDemonRenderTextureFormats::RGBA8);
                } else {
                    m_textureAtlas->SetTextureData(QDemonDataRef<quint8>(), 0, TEXTURE_ATLAS_DIM,
                                                   TEXTURE_ATLAS_DIM, QDemonRenderTextureFormats::Alpha8);
                }
                m_textureAtlas->SetMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
                m_textureAtlas->SetMinFilter(QDemonRenderTextureMinifyingOp::Linear);
                m_textureAtlas->SetTextureWrapS(QDemonRenderTextureCoordOp::ClampToEdge);
                m_textureAtlas->SetTextureWrapT(QDemonRenderTextureCoordOp::ClampToEdge);
                STextureDetails texTexDetails = m_textureAtlas->GetTextureDetails();
                return TTextTextureAtlasDetailsAndTexture(
                            STextTextureAtlasDetails(texTexDetails.m_Height, texTexDetails.m_Height, false,
                                                     count),
                            m_textureAtlas);
            }
        }

        return TTextTextureAtlasDetailsAndTexture(STextTextureAtlasDetails(), nullptr);
    }

private:
    bool m_TextureAtlasInitialized;
    QDemonRenderTexture2D *m_textureAtlas; // this is the actual texture which has application lifetime
};

} // namespace

ITextTextureAtlas &ITextTextureAtlas::CreateTextureAtlas(ITextRenderer &inTextRenderer,
                                                         QDemonRenderContext &inRenderContext)
{
    return *new STextTextureAtlas(inTextRenderer, inRenderContext);
}

QT_END_NAMESPACE
