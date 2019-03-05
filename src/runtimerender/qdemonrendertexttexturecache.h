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

class QDemonTextRendererInterface;
class QDemonRenderContext;

typedef QPair<QDemonRef<QDemonRenderPathFontSpecification>, QDemonRef<QDemonRenderPathFontItem>> TPathFontSpecAndPathObject;
typedef QPair<QDemonTextTextureDetails, QDemonRef<QDemonRenderTexture2D>> TTextTextureDetailsAndTexture;
typedef QPair<TPathFontSpecAndPathObject, TTextTextureDetailsAndTexture> TTPathObjectAndTexture;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonTextTextureCacheInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonTextTextureCacheInterface() {}
    virtual TTPathObjectAndTexture renderText(const QDemonTextRenderInfo &inText, float inScaleFactor) = 0;
    // We may have one more texture in cache than this byte count, but this will be the limiting
    // factor.
    virtual quint32 getCacheHighWaterBytes() const = 0;
    virtual void setCacheHighWaterBytes(quint32 inNumBytes) = 0;

    virtual void beginFrame() = 0;
    // We need to know the frame rhythm because we can't release anything that was touched this
    // frame.
    virtual void endFrame() = 0;

    static QDemonRef<QDemonTextTextureCacheInterface> createTextureCache(QDemonRef<QDemonTextRendererInterface> inTextRenderer,
                                                                         QDemonRef<QDemonRenderContext> inRenderContext);
};

struct QDemonTextRenderInfoAndHash
{
    QDemonTextRenderInfo m_info;
    float m_scaleFactor;
    uint m_hashcode;
    QDemonTextRenderInfoAndHash(const QDemonTextRenderInfo &inInfo, float inScaleFactor);
    bool operator==(const QDemonTextRenderInfoAndHash &inOther) const
    {
        return m_info.text == inOther.m_info.text && m_info.font == inOther.m_info.font
                && qFuzzyCompare(m_info.fontSize, inOther.m_info.fontSize)
                && m_info.horizontalAlignment == inOther.m_info.horizontalAlignment
                && m_info.verticalAlignment == inOther.m_info.verticalAlignment
                && qFuzzyCompare(m_info.leading, inOther.m_info.leading)
                && qFuzzyCompare(m_info.tracking, inOther.m_info.tracking) && m_info.dropShadow == inOther.m_info.dropShadow
                && qFuzzyCompare(m_info.dropShadowStrength, inOther.m_info.dropShadowStrength)
                && qFuzzyCompare(m_info.dropShadowOffset, inOther.m_info.dropShadowOffset)
                && m_info.dropShadowHorizontalAlignment == inOther.m_info.dropShadowHorizontalAlignment
                && m_info.dropShadowVerticalAlignment == inOther.m_info.dropShadowVerticalAlignment
                && m_info.enableAcceleratedFont == inOther.m_info.enableAcceleratedFont
                && qFuzzyCompare(m_scaleFactor, inOther.m_scaleFactor);
    }
};

QT_END_NAMESPACE
#endif
