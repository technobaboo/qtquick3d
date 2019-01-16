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
#ifndef QDEMON_RENDER_PIXEL_GRAPHICS_RENDERER_H
#define QDEMON_RENDER_PIXEL_GRAPHICS_RENDERER_H

#include <QtCore/QVector>
#include <QtCore/QSharedPointer>
#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>

QT_BEGIN_NAMESPACE
// Pixel graphics are graphics described in pixels.
// Colors are expected to be non-premultiplied, we use ROP
// hardware to do the alpha multiply into the color.

struct SPGGraphObject;
class IQDemonRenderContext;
class IPixelGraphicsRenderer
{
public:
    virtual ~IPixelGraphicsRenderer() {}

    // Renders the node to the current viewport.
    virtual void Render(const QVector<SPGGraphObject *> &inObjects) = 0;

    static QSharedPointer<IPixelGraphicsRenderer> CreateRenderer(QSharedPointer<IQDemonRenderContext> ctx);
};
QT_END_NAMESPACE

#endif
