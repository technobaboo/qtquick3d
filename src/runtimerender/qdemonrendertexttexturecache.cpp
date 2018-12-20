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

uint qHash(const STextRenderInfo &inInfo)
{
    uint retval = qHash(inInfo.m_Text);
    retval = retval ^ qHash(inInfo.m_Font);
    retval = retval ^ qHash(inInfo.m_FontSize);
    retval = retval ^ qHash(inInfo.m_HorizontalAlignment);
    retval = retval ^ qHash(inInfo.m_VerticalAlignment);
    retval = retval ^ qHash(inInfo.m_Leading);
    retval = retval ^ qHash(inInfo.m_Tracking);
    retval = retval ^ qHash(inInfo.m_DropShadow);
    retval = retval ^ qHash(inInfo.m_DropShadowStrength);
    retval = retval ^ qHash(inInfo.m_DropShadowOffset);
    retval = retval ^ qHash(inInfo.m_DropShadowHorizontalAlignment);
    retval = retval ^ qHash(inInfo.m_DropShadowVerticalAlignment);
    retval = retval ^ qHash(inInfo.m_EnableAcceleratedFont);
    return retval;
}

namespace {
struct STextRenderInfoAndHash
{
    STextRenderInfo m_Info;
    float m_ScaleFactor;
    uint m_Hashcode;
    STextRenderInfoAndHash(const STextRenderInfo &inInfo, float inScaleFactor)
        : m_Info(inInfo)
        , m_ScaleFactor(inScaleFactor)
        , m_Hashcode(qHash(inInfo) ^ qHash(inScaleFactor))
    {
    }
    bool operator==(const STextRenderInfoAndHash &inOther) const
    {
        return     m_Info.m_Text == inOther.m_Info.m_Text
                && m_Info.m_Font == inOther.m_Info.m_Font
                && m_Info.m_FontSize == inOther.m_Info.m_FontSize
                && m_Info.m_HorizontalAlignment == inOther.m_Info.m_HorizontalAlignment
                && m_Info.m_VerticalAlignment == inOther.m_Info.m_VerticalAlignment
                && m_Info.m_Leading == inOther.m_Info.m_Leading
                && m_Info.m_Tracking == inOther.m_Info.m_Tracking
                && m_Info.m_DropShadow == inOther.m_Info.m_DropShadow
                && m_Info.m_DropShadowStrength == inOther.m_Info.m_DropShadowStrength
                && m_Info.m_DropShadowOffset == inOther.m_Info.m_DropShadowOffset
                && m_Info.m_DropShadowHorizontalAlignment == inOther.m_Info.m_DropShadowHorizontalAlignment
                && m_Info.m_DropShadowVerticalAlignment == inOther.m_Info.m_DropShadowVerticalAlignment
                && m_Info.m_EnableAcceleratedFont == inOther.m_Info.m_EnableAcceleratedFont
                && m_ScaleFactor == inOther.m_ScaleFactor;
    }
};
}

uint qHash(const STextRenderInfoAndHash &inInfo)
{
    return inInfo.m_Hashcode;
}

namespace {

struct STextCacheNode
{
    STextCacheNode *m_PreviousSibling;
    STextCacheNode *m_NextSibling;
    STextRenderInfoAndHash m_RenderInfo;
    TTPathObjectAndTexture m_TextInfo;
    quint32 m_FrameCount;

    STextCacheNode(const STextRenderInfoAndHash &inRenderInfo, const TTPathObjectAndTexture &inTextInfo)
        : m_PreviousSibling(nullptr)
        , m_NextSibling(nullptr)
        , m_RenderInfo(inRenderInfo)
        , m_TextInfo(inTextInfo)
        , m_FrameCount(0)
    {
    }
};

typedef QHash<STextRenderInfoAndHash, STextCacheNode *> TTextureInfoHash;

DEFINE_INVASIVE_LIST(TextCacheNode);
IMPLEMENT_INVASIVE_LIST(TextCacheNode, m_PreviousSibling, m_NextSibling);

struct STextTextureCache : public ITextTextureCache
{
    QSharedPointer<ITextRenderer> m_TextRenderer;
    TTextureInfoHash m_TextureCache;
    TTextCacheNodeList m_LRUList;
    quint32 m_HighWaterMark;
    quint32 m_FrameCount;
    quint32 m_TextureTotalBytes;
    QSharedPointer<QDemonRenderContext> m_RenderContext;
    bool m_CanUsePathRendering; ///< true if we use hardware accelerated font rendering

    STextTextureCache(QSharedPointer<ITextRenderer> inRenderer, QSharedPointer<QDemonRenderContext> inRenderContext)
        : m_TextRenderer(inRenderer)
        , m_HighWaterMark(0x100000)
        , m_FrameCount(0)
        , m_TextureTotalBytes(0)
        , m_RenderContext(inRenderContext)
    {
        // hardware accelerate font rendering not ready yet
        m_CanUsePathRendering = (m_RenderContext->IsPathRenderingSupported() && m_RenderContext->IsProgramPipelineSupported());
    }

    virtual ~STextTextureCache() override
    {
        for (TTextCacheNodeList::iterator iter = m_LRUList.begin(), end = m_LRUList.end(); iter != end; ++iter)
            delete &iter;
    }

    static inline quint32 GetNumBytes(QDemonRenderTexture2D &inTexture)
    {
        STextureDetails theDetails(inTexture.GetTextureDetails());
        return theDetails.m_Width * theDetails.m_Height * QDemonRenderTextureFormats::getSizeofFormat(theDetails.m_Format);
    }

    QSharedPointer<QDemonRenderTexture2D> InvalidateLastItem()
    {
        QSharedPointer<QDemonRenderTexture2D> nextTexture;
        if (m_LRUList.empty() == false) {
            STextCacheNode *theEnd = m_LRUList.back_ptr();
            if (theEnd->m_FrameCount != m_FrameCount) {
                nextTexture = theEnd->m_TextInfo.second.second;
                //STextureDetails theDetails = nextTexture->GetTextureDetails();
                m_TextureTotalBytes -= GetNumBytes(*nextTexture.data());
                m_LRUList.remove(*theEnd);
                // copy the key because the next statement will destroy memory
                m_TextureCache.remove(theEnd->m_RenderInfo);
                delete theEnd;
            }
        }
        return nextTexture;
    }

    TTPathObjectAndTexture RenderText(const STextRenderInfo &inText, float inScaleFactor) override
    {
        STextRenderInfoAndHash theKey(inText, inScaleFactor);
        TTextureInfoHash::iterator theFind(m_TextureCache.find(STextRenderInfoAndHash(inText, inScaleFactor)));
        STextCacheNode *retval = nullptr;
        if (theFind != m_TextureCache.end()) {
            retval = theFind.value();
            m_LRUList.remove(*retval);
        } else {
            QSharedPointer<QDemonRenderTexture2D> nextTexture;
            if (m_TextureTotalBytes >= m_HighWaterMark && m_LRUList.empty() == false)
                nextTexture = InvalidateLastItem();

            if (nextTexture.isNull())
                nextTexture = m_RenderContext->CreateTexture2D();

            QSharedPointer<QDemonRenderPathFontItem> nextPathFontItemObject;
            QSharedPointer<QDemonRenderPathFontSpecification> nextPathFontObject;
            // HW acceleration for fonts not supported
            //if (m_CanUsePathRendering && inText.m_EnableAcceleratedFont) {
            //    nextPathFontItemObject = m_RenderContext->CreatePathFontItem();
            //    nextPathFontObject = m_RenderContext->CreatePathFontSpecification(inText.m_Font);
            //}

            STextRenderInfo theTextInfo(inText);
            theTextInfo.m_FontSize *= inScaleFactor;
            STextTextureDetails theDetails;


            // HW acceleration for fonts not supported
            //if (!m_CanUsePathRendering || !inText.m_EnableAcceleratedFont)
            theDetails = m_TextRenderer->RenderText(theTextInfo, *nextTexture.data());
            //else
            //    theDetails = m_TextRenderer->RenderText(theTextInfo, *nextPathFontItemObject.mPtr,
            //                                            *nextPathFontObject.mPtr);

            if (fabs(inScaleFactor - 1.0f) > .001f) {
                TTPathObjectAndTexture theCanonicalDetails = RenderText(inText, 1.0f);
                theDetails.m_ScaleFactor.setX(float(theDetails.m_TextWidth) / theCanonicalDetails.second.first.m_TextWidth);
                theDetails.m_ScaleFactor.setY(float(theDetails.m_TextHeight) / theCanonicalDetails.second.first.m_TextHeight);
            }
            theKey = STextRenderInfoAndHash(inText, inScaleFactor);
            retval = new STextCacheNode(theKey, TTPathObjectAndTexture(
                            TPathFontSpecAndPathObject(nextPathFontObject, nextPathFontItemObject),
                            TTextTextureDetailsAndTexture(theDetails, nextTexture)));
            m_TextureCache.insert(theKey, retval);
            if (!m_CanUsePathRendering)
                m_TextureTotalBytes += GetNumBytes(*(retval->m_TextInfo.second.second.data()));
        }
        retval->m_FrameCount = m_FrameCount;
        m_LRUList.push_front(*retval);
        return retval->m_TextInfo;
    }
    // We may have one more texture in cache than this byte count, but this will be the limiting
    // factor.
    quint32 GetCacheHighWaterBytes() const override { return m_HighWaterMark; }
    // default cache size is 10 MB.
    void SetCacheHighWaterBytes(quint32 inByteCount) override { m_HighWaterMark = inByteCount; }

    void BeginFrame() override {}
    void EndFrame() override
    {
        // algorithm is resistant to rollover.
        ++m_FrameCount;
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

ITextTextureCache &ITextTextureCache::CreateTextureCache(QSharedPointer<ITextRenderer> inTextRenderer,
                                                         QSharedPointer<QDemonRenderContext> inRenderContext)
{
    return *new STextTextureCache(inTextRenderer, inRenderContext);
}

QT_END_NAMESPACE
