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
#ifndef QDEMON_RENDER_TEXT_TEXTURE_CACHE_H
#define QDEMON_RENDER_TEXT_TEXTURE_CACHE_H

#include <QtDemonRuntimeRender/qdemonrendertext.h>

QT_BEGIN_NAMESPACE

class ITextRenderer;
class QDemonRenderContext;

typedef QPair<QSharedPointer<QDemonRenderPathFontSpecification>, QSharedPointer<QDemonRenderPathFontItem>> TPathFontSpecAndPathObject;
typedef QPair<STextTextureDetails, QSharedPointer<QDemonRenderTexture2D>> TTextTextureDetailsAndTexture;
typedef QPair<TPathFontSpecAndPathObject, TTextTextureDetailsAndTexture> TTPathObjectAndTexture;

class ITextTextureCache
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

    static ITextTextureCache &CreateTextureCache(QSharedPointer<ITextRenderer> inTextRenderer, QSharedPointer<QDemonRenderContext> inRenderContext);
};
QT_END_NAMESPACE
#endif