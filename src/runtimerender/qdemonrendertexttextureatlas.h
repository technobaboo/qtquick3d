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
#ifndef QDEMON_RENDER_TEXT_TEXTURE_ATLAS_H
#define QDEMON_RENDER_TEXT_TEXTURE_ATLAS_H

#include <QtDemonRuntimeRender/qdemonrendertext.h>

QT_BEGIN_NAMESPACE
class QDemonTextRendererInterface;
class QDemonRenderContext;

typedef QPair<QDemonTextTextureAtlasDetails, QDemonRef<QDemonRenderTexture2D>>
TTextTextureAtlasDetailsAndTexture;
typedef QPair<QDemonRenderTextureAtlasDetails, QDemonRef<QDemonRenderTexture2D>>
TTextRenderAtlasDetailsAndTexture;

class QDemonTextTextureAtlasInterface
{
protected:
    virtual ~QDemonTextTextureAtlasInterface();
public:
    virtual TTextRenderAtlasDetailsAndTexture renderText(const QDemonTextRenderInfo &inText) = 0;
    virtual bool isInitialized() = 0;
    virtual TTextTextureAtlasDetailsAndTexture prepareTextureAtlas() = 0;

    static QDemonRef<QDemonTextTextureAtlasInterface> createTextureAtlas(QDemonRef<QDemonTextRendererInterface> inTextRenderer,
                                                                              QDemonRef<QDemonRenderContext> inRenderContext);
};
QT_END_NAMESPACE
#endif
