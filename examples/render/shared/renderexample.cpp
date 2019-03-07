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
#include "renderexample.h"
#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <cmath>

#include <QtGui/QSurfaceFormat>

QDemonRenderExample::QDemonRenderExample(QWindow *parent)
    : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setWidth(1280);
    setHeight(720);
    m_frameTimer.start();
}

QDemonRenderExample::~QDemonRenderExample()
{
    delete m_context;
}

void QDemonRenderExample::renderLater()
{
    requestUpdate();
}

void QDemonRenderExample::renderNow()
{
    if (!m_isIntialized) {
        preInit();
        initialize();
        m_isIntialized = true;
    }
    m_context->makeCurrent(this);
    drawFrame(m_frameTimer.elapsed());
    m_frameTimer.restart();
    m_context->swapBuffers(this);
    m_context->doneCurrent();
    if (m_autoUpdate)
        renderLater();
}

bool QDemonRenderExample::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void QDemonRenderExample::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void QDemonRenderExample::preInit()
{
    m_context = new QOpenGLContext();
    m_context->setFormat(requestedFormat());
    m_context->create();

    if (!m_context->makeCurrent(this))
        qDebug("fail");
}


// Math stuff

int eq(float a, float b)
{
    float diff = a - b;
    if (diff < 0) {
        diff = -diff;
    }
    return diff <= eps;
}
