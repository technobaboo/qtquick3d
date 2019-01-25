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
#ifndef QDEMON_RENDER_TEXT_TEXTURE_CACHE_H
#define QDEMON_RENDER_TEXT_TEXTURE_CACHE_H

#include <QtDemonRuntimeRender/qdemonrendertext.h>

QT_BEGIN_NAMESPACE

class ITextRenderer;
class QDemonRenderContext;

typedef QPair<QSharedPointer<QDemonRenderPathFontSpecification>, QSharedPointer<QDemonRenderPathFontItem>> TPathFontSpecAndPathObject;
typedef QPair<STextTextureDetails, QSharedPointer<QDemonRenderTexture2D>> TTextTextureDetailsAndTexture;
typedef QPair<TPathFontSpecAndPathObject, TTextTextureDetailsAndTexture> TTPathObjectAndTexture;

class Q_DEMONRUNTIMERENDER_EXPORT ITextTextureCache
{
protected:
    virtual ~ITextTextureCache() {}
public:
    virtual TTPathObjectAndTexture RenderText(const STextRenderInfo &inText, float inScaleFactor) = 0;
    // We may have one more texture in cache than this byte count, but this will be the limiting
    // factor.
    virtual quint32 GetCacheHighWaterBytes() const = 0;
    virtual void SetCacheHighWaterBytes(quint32 inNumBytes) = 0;

    virtual void BeginFrame() = 0;
    // We need to know the frame rhythm because we can't release anything that was touched this
    // frame.
    virtual void EndFrame() = 0;

    static QSharedPointer<ITextTextureCache> CreateTextureCache(QSharedPointer<ITextRenderer> inTextRenderer, QSharedPointer<QDemonRenderContext> inRenderContext);
};

struct STextRenderInfoAndHash
{
    STextRenderInfo m_Info;
    float m_ScaleFactor;
    uint m_Hashcode;
    STextRenderInfoAndHash(const STextRenderInfo &inInfo, float inScaleFactor);
    bool operator==(const STextRenderInfoAndHash &inOther) const
    {
        return     m_Info.m_Text == inOther.m_Info.m_Text
                && m_Info.m_Font == inOther.m_Info.m_Font
                && qFuzzyCompare(m_Info.m_FontSize, inOther.m_Info.m_FontSize)
                && m_Info.m_HorizontalAlignment == inOther.m_Info.m_HorizontalAlignment
                && m_Info.m_VerticalAlignment == inOther.m_Info.m_VerticalAlignment
                && qFuzzyCompare(m_Info.m_Leading, inOther.m_Info.m_Leading)
                && qFuzzyCompare(m_Info.m_Tracking, inOther.m_Info.m_Tracking)
                && m_Info.m_DropShadow == inOther.m_Info.m_DropShadow
                && qFuzzyCompare(m_Info.m_DropShadowStrength, inOther.m_Info.m_DropShadowStrength)
                && qFuzzyCompare(m_Info.m_DropShadowOffset, inOther.m_Info.m_DropShadowOffset)
                && m_Info.m_DropShadowHorizontalAlignment == inOther.m_Info.m_DropShadowHorizontalAlignment
                && m_Info.m_DropShadowVerticalAlignment == inOther.m_Info.m_DropShadowVerticalAlignment
                && m_Info.m_EnableAcceleratedFont == inOther.m_Info.m_EnableAcceleratedFont
                && qFuzzyCompare(m_ScaleFactor, inOther.m_ScaleFactor);
    }
};

QT_END_NAMESPACE
#endif
