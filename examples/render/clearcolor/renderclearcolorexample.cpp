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
#include "../shared/renderexample.h"
#include <QtGui/QVector4D>
#include <QtDemonRender/QDemonRenderContext>
#include <QtGui/QGuiApplication>

namespace {
class ClearColor : public QDemonRenderExample
{
    QDemonRef<QDemonRenderContext> m_context;
    qint64 m_elapsedTime = 0;

public:
    ClearColor()
    {
    }

    void initialize() override
    {
        m_context = QDemonRenderContext::createGl(format());
    }

    virtual void drawFrame(qint64 delta) override
    {
        m_elapsedTime += delta;
        // Apply this value immediately but track it so that a later pop will in fact
        // restore this value.
        if (m_elapsedTime < 1000) {
            m_context->setClearColor(QVector4D(.8f, .0f, .0f, 1.f));
        } else if (m_elapsedTime < 2000) {
            m_context->setClearColor(QVector4D(0.f, .0f, 1.f, 1.f));
        } else {
            m_context->setClearColor(QVector4D(0.f, 1.0f, 1.f, 1.f));
            m_elapsedTime = 0;
        }
        m_context->clear(QDemonRenderClearFlags(QDemonRenderClearValues::Color));
    }
};
}


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Advanced: Try 4.3 core (so we get compute shaders for instance)
    fmt.setVersion(4, 3);

    ClearColor clearColor;
    clearColor.setFormat(fmt);
    clearColor.show();

    return app.exec();
}

