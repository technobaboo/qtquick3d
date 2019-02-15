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
#include "qdemonrendertexttexturecache.h"
#include "qdemontextrenderer.h"

#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemoninvasivelinkedlist.h>

QT_BEGIN_NAMESPACE

uint qHash(const QDemonTextRenderInfo &inInfo)
{
    uint retval = qHash(inInfo.text);
    retval = retval ^ qHash(inInfo.font);
    retval = retval ^ qHash(inInfo.fontSize);
    retval = retval ^ qHash(inInfo.horizontalAlignment);
    retval = retval ^ qHash(inInfo.verticalAlignment);
    retval = retval ^ qHash(inInfo.leading);
    retval = retval ^ qHash(inInfo.tracking);
    retval = retval ^ qHash(inInfo.dropShadow);
    retval = retval ^ qHash(inInfo.dropShadowStrength);
    retval = retval ^ qHash(inInfo.dropShadowOffset);
    retval = retval ^ qHash(inInfo.dropShadowHorizontalAlignment);
    retval = retval ^ qHash(inInfo.dropShadowVerticalAlignment);
    retval = retval ^ qHash(inInfo.enableAcceleratedFont);
    return retval;
}

uint qHash(const QDemonTextRenderInfoAndHash &inInfo)
{
    return inInfo.m_hashcode;
}

namespace {

struct QDemonTextCacheNode
{
    QDemonTextCacheNode *previousSibling;
    QDemonTextCacheNode *nextSibling;
    QDemonTextRenderInfoAndHash renderInfo;
    TTPathObjectAndTexture textInfo;
    quint32 frameCount;

    QDemonTextCacheNode(const QDemonTextRenderInfoAndHash &inRenderInfo, const TTPathObjectAndTexture &inTextInfo)
        : previousSibling(nullptr)
        , nextSibling(nullptr)
        , renderInfo(inRenderInfo)
        , textInfo(inTextInfo)
        , frameCount(0)
    {
    }
};

typedef QHash<QDemonTextRenderInfoAndHash, QDemonTextCacheNode *> QDemonTextureInfoHash;

DEFINE_INVASIVE_LIST(QDemonTextCacheNode);
IMPLEMENT_INVASIVE_LIST(QDemonTextCacheNode, previousSibling, nextSibling);

struct QDemonTextTextureCache : public QDemonTextTextureCacheInterface
{
    QSharedPointer<QDemonTextRendererInterface> textRenderer;
    QDemonTextureInfoHash textureCache;
    QDemonTextCacheNodeList textCacheNodeList;
    quint32 highWaterMark;
    quint32 frameCount;
    quint32 textureTotalBytes;
    QSharedPointer<QDemonRenderContext> renderContext;
    bool canUsePathRendering; ///< true if we use hardware accelerated font rendering

    QDemonTextTextureCache(QSharedPointer<QDemonTextRendererInterface> inRenderer, QSharedPointer<QDemonRenderContext> inRenderContext)
        : textRenderer(inRenderer)
        , highWaterMark(0x100000)
        , frameCount(0)
        , textureTotalBytes(0)
        , renderContext(inRenderContext)
    {
        // hardware accelerate font rendering not ready yet
        canUsePathRendering = (renderContext->isPathRenderingSupported() && renderContext->isProgramPipelineSupported());
    }

    virtual ~QDemonTextTextureCache() override
    {
        auto iter = textCacheNodeList.begin();
        const auto end = textCacheNodeList.end();
        while (iter != end) {
            delete &iter;
            ++iter;
        }
    }

    static inline quint32 getNumBytes(QDemonRenderTexture2D &inTexture)
    {
        QDemonTextureDetails theDetails(inTexture.getTextureDetails());
        return theDetails.width * theDetails.height * QDemonRenderTextureFormats::getSizeofFormat(theDetails.format);
    }

    QSharedPointer<QDemonRenderTexture2D> invalidateLastItem()
    {
        QSharedPointer<QDemonRenderTexture2D> nextTexture;
        if (textCacheNodeList.empty() == false) {
            QDemonTextCacheNode *theEnd = textCacheNodeList.back_ptr();
            if (theEnd->frameCount != frameCount) {
                nextTexture = theEnd->textInfo.second.second;
                //STextureDetails theDetails = nextTexture->GetTextureDetails();
                textureTotalBytes -= getNumBytes(*nextTexture.data());
                textCacheNodeList.remove(*theEnd);
                // copy the key because the next statement will destroy memory
                textureCache.remove(theEnd->renderInfo);
                delete theEnd;
            }
        }
        return nextTexture;
    }

    TTPathObjectAndTexture renderText(const QDemonTextRenderInfo &inText, float inScaleFactor) override
    {
        QDemonTextRenderInfoAndHash theKey(inText, inScaleFactor);
        QDemonTextureInfoHash::iterator theFind(textureCache.find(QDemonTextRenderInfoAndHash(inText, inScaleFactor)));
        QDemonTextCacheNode *retval = nullptr;
        if (theFind != textureCache.end()) {
            retval = theFind.value();
            textCacheNodeList.remove(*retval);
        } else {
            QSharedPointer<QDemonRenderTexture2D> nextTexture;
            if (textureTotalBytes >= highWaterMark && textCacheNodeList.empty() == false)
                nextTexture = invalidateLastItem();

            if (nextTexture.isNull())
                nextTexture = renderContext->createTexture2D();

            QSharedPointer<QDemonRenderPathFontItem> nextPathFontItemObject;
            QSharedPointer<QDemonRenderPathFontSpecification> nextPathFontObject;
            // HW acceleration for fonts not supported
            //if (m_CanUsePathRendering && inText.m_EnableAcceleratedFont) {
            //    nextPathFontItemObject = m_RenderContext->CreatePathFontItem();
            //    nextPathFontObject = m_RenderContext->CreatePathFontSpecification(inText.m_Font);
            //}

            QDemonTextRenderInfo theTextInfo(inText);
            theTextInfo.fontSize *= inScaleFactor;
            QDemonTextTextureDetails theDetails;


            // HW acceleration for fonts not supported
            //if (!m_CanUsePathRendering || !inText.m_EnableAcceleratedFont)
            theDetails = textRenderer->renderText(theTextInfo, *nextTexture.data());
            //else
            //    theDetails = m_TextRenderer->RenderText(theTextInfo, *nextPathFontItemObject.mPtr,
            //                                            *nextPathFontObject.mPtr);

            if (std::fabs(inScaleFactor - 1.0f) > .001f) {
                TTPathObjectAndTexture theCanonicalDetails = renderText(inText, 1.0f);
                theDetails.scaleFactor.setX(float(theDetails.textWidth) / theCanonicalDetails.second.first.textWidth);
                theDetails.scaleFactor.setY(float(theDetails.textHeight) / theCanonicalDetails.second.first.textHeight);
            }
            theKey = QDemonTextRenderInfoAndHash(inText, inScaleFactor);
            retval = new QDemonTextCacheNode(theKey, TTPathObjectAndTexture(
                                            TPathFontSpecAndPathObject(nextPathFontObject, nextPathFontItemObject),
                                            TTextTextureDetailsAndTexture(theDetails, nextTexture)));
            textureCache.insert(theKey, retval);
            if (!canUsePathRendering)
                textureTotalBytes += getNumBytes(*(retval->textInfo.second.second.data()));
        }
        retval->frameCount = frameCount;
        textCacheNodeList.push_front(*retval);
        return retval->textInfo;
    }
    // We may have one more texture in cache than this byte count, but this will be the limiting
    // factor.
    quint32 getCacheHighWaterBytes() const override { return highWaterMark; }
    // default cache size is 10 MB.
    void setCacheHighWaterBytes(quint32 inByteCount) override { highWaterMark = inByteCount; }

    void beginFrame() override {}
    void endFrame() override
    {
        // algorithm is resistant to rollover.
        ++frameCount;
        // Release any texture that put us over the limit.
        // This almost guarantees thrashing if the limit is set too low.  Enable at your
        // own risk at *TEST CAREFULLY*
        /*
        while( m_TextureTotalBytes >= m_HighWaterMark && m_LRUList.empty() == false )
                InvalidateLastItem();
        */
    }
};
}

QSharedPointer<QDemonTextTextureCacheInterface> QDemonTextTextureCacheInterface::createTextureCache(QSharedPointer<QDemonTextRendererInterface> inTextRenderer,
                                                                                                    QSharedPointer<QDemonRenderContext> inRenderContext)
{
    return QSharedPointer<QDemonTextTextureCacheInterface>(new QDemonTextTextureCache(inTextRenderer, inRenderContext));
}

QDemonTextRenderInfoAndHash::QDemonTextRenderInfoAndHash(const QDemonTextRenderInfo &inInfo,
                                                         float inScaleFactor)
    : m_info(inInfo)
    , m_scaleFactor(inScaleFactor)
    , m_hashcode(qHash(inInfo) ^ qHash(inScaleFactor))
{
}

QT_END_NAMESPACE
