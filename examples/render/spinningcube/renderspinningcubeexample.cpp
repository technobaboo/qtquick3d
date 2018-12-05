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
        m_context = QDemonRenderContext::CreateGL(format());
        m_inputAssembler = QDemonRenderExampleTools::createBox(m_context.data(), m_vertexBuffer, m_indexBuffer);
        m_shader = QDemonRenderExampleTools::createSimpleShader(m_context.data());
        m_context->SetViewport(QDemonRenderRect(0, 0, 400, 400));
        if (m_shader) {
            m_context->SetActiveShader(m_shader);
        }
        m_context->SetDepthTestEnabled(true);
        m_context->SetDepthWriteEnabled(true);
        QDemonGl2DemoMatrixIdentity(model);
        QDemonGl2DemoMatrixIdentity(frus);
        QDemonGl2DemoMatrixFrustum(frus, -1, 1, -1, 1, 1, 10);
        QDemonGl2DemoMatrixTranslate(model, 0, 0, -4);
    }

    virtual void drawFrame(qint64 delta) override
    {
        m_elapsedTime += delta;
        QDemonGl2DemoMatrixRotate_create3x3(rot, (float)m_elapsedTime * 50, .707f, .707f, 0);
        float mvp[16];
        QDemonGl2DemoMatrixIdentity(mvp);
        QDemonGl2DemoMatrixMultiply(mvp, frus);
        QDemonGl2DemoMatrixMultiply(mvp, model);
        QDemonGl2DemoMatrixMultiply_4x4_3x3(mvp, rot);

        QDemonConstDataRef<quint8> instance((quint8 *)mvp, 16 * sizeof(float));
        m_context->Clear(
            QDemonRenderClearFlags(QDemonRenderClearValues::Color | QDemonRenderClearValues::Depth));
        m_context->SetInputAssembler(m_inputAssembler.data());

        m_shader->SetPropertyValue("mat_mvp", *reinterpret_cast<QMatrix4x4 *>(mvp));
        m_context->Draw(QDemonRenderDrawMode::Triangles, m_indexBuffer->GetNumIndices(), 0);
    }
private:
    QSharedPointer<QDemonRenderContext> m_context;
    QSharedPointer<QDemonRenderVertexBuffer> m_vertexBuffer;
    QSharedPointer<QDemonRenderIndexBuffer> m_indexBuffer;
    QSharedPointer<QDemonRenderInputAssembler> m_inputAssembler;
    QSharedPointer<QDemonRenderShaderProgram> m_shader;
    float frus[16];
    float model[16];
    float rot[9];
    qint64 m_elapsedTime = 0;

};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Advanced: Try 4.3 core (so we get compute shaders for instance)
    fmt.setVersion(4, 3);

    SpinningCube spinningCube;
    spinningCube.setFormat(fmt);
    spinningCube.show();

    return app.exec();
}
