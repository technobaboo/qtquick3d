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
#ifndef QDEMON_RENDER_TEXT_H
#define QDEMON_RENDER_TEXT_H

#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrendertexttypes.h>

QT_BEGIN_NAMESPACE
class QDemonRenderTexture2D;
class QDemonRenderPathFontItem;
class QDemonRenderPathFontSpecification;

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderText : public QDemonRenderNode, public QDemonTextRenderInfo
{
    Q_DISABLE_COPY(QDemonRenderText)
    // Change any of these properties and you can expect
    // that the text will force an expensive re-layer and render.
    // For these you need to set TextDirty.

    // These properties can change every frame with no additional cost.
    QVector3D m_textColor;
    // Setup and utilized by the rendering system
    QDemonRef<QDemonRenderTexture2D> m_textTexture;
    QDemonTextTextureDetails m_textTextureDetails;
    // used for nv path rendering
    QDemonRef<QDemonRenderPathFontItem> m_pathFontItem;
    QDemonRef<QDemonRenderPathFontSpecification> m_pathFontDetails;

    QDemonBounds3 m_bounds;

    QDemonRenderText();
    ~QDemonRenderText();

    QDemonBounds3 getTextBounds() const;
};
QT_END_NAMESPACE
#endif
