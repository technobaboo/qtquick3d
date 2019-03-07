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
#include "../shared/renderexampletools.h"
#include <QtGui/QVector4D>
#include <QtGui/QMatrix4x4>
#include <QtDemonRender/QDemonRenderContext>
#include <QtDemonRender/QDemonRenderShaderProgram>
#include <QtDemonRender/qdemonrenderindexbuffer.h>
#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtGui/QGuiApplication>

struct ShaderArguments
{
    QMatrix4x4 mMatrix;
};

class SpinningCube : public QDemonRenderExample
{
public:
    SpinningCube()
    {

    }
    ~SpinningCube() override
    {
    }

    void initialize() override
    {
        m_context = QDemonRenderContext::createGl(format());
        m_inputAssembler = QDemonRenderExampleTools::createBox(m_context, m_vertexBuffer, m_indexBuffer);
        m_shader = QDemonRenderExampleTools::createSimpleShader(m_context);
        if (m_shader) {
            m_context->setActiveShader(m_shader);
        }
        m_context->setDepthTestEnabled(true);
        m_context->setDepthWriteEnabled(true);
        frus.frustum(-1, 1, -1, 1, 1, 10);
        model.translate(0, 0, -4);
    }

    virtual void drawFrame(qint64 delta) override
    {
        m_elapsedTime += delta;
        QMatrix4x4 rot;
        rot.rotate((float)m_elapsedTime * 0.1f, .707f, .707f, 0);
        QMatrix4x4 mvp;
        mvp *= frus;
        mvp *= model;
        mvp *= rot;;

        if (m_viewportDirty) {
            m_context->setViewport(QRect(0, 0, this->width(), this->height()));
            m_viewportDirty = false;
        }

        m_context->clear(
            QDemonRenderClearFlags(QDemonRenderClearValues::Color | QDemonRenderClearValues::Depth));
        m_context->setInputAssembler(m_inputAssembler);

        m_shader->setPropertyValue("mat_mvp", mvp);
        m_context->draw(QDemonRenderDrawMode::Triangles, m_indexBuffer->getNumIndices(), 0);
    }
private:
    QDemonRef<QDemonRenderContext> m_context;
    QDemonRef<QDemonRenderVertexBuffer> m_vertexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> m_indexBuffer;
    QDemonRef<QDemonRenderInputAssembler> m_inputAssembler;
    QDemonRef<QDemonRenderShaderProgram> m_shader;
    QMatrix4x4 frus;
    QMatrix4x4 model;
    qint64 m_elapsedTime = 0;
    bool m_viewportDirty = true;


    // QWindow interface
protected:
    void resizeEvent(QResizeEvent *)
    {
        m_viewportDirty = true;
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Advanced: Try 4.3 core (so we get compute shaders for instance)
    fmt.setVersion(4, 3);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    SpinningCube spinningCube;
    spinningCube.setFormat(fmt);
    spinningCube.show();

    return app.exec();
}
