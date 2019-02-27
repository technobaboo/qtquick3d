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
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrenderindexbuffer.h>
#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtGui/QVector4D>
#include <QtGui/QGuiApplication>

struct ShaderArgs
{
    float mvp[16];
    QSharedPointer<QDemonRenderTexture2D> texture;
    QSharedPointer<QDemonRenderShaderProgram> shader;
    ShaderArgs() {}
};
class RenderToTexture : public QDemonRenderExample
{
    QSharedPointer<QDemonRenderContext> m_Context;
    QSharedPointer<QDemonRenderVertexBuffer> mVertexBuffer;
    QSharedPointer<QDemonRenderIndexBuffer> mIndexBuffer;
    QSharedPointer<QDemonRenderInputAssembler> mInputAssembler;
    // Simple shader
    QSharedPointer<QDemonRenderShaderProgram> mSimpleShader;
    // Simple shader with texture lookup.
    QSharedPointer<QDemonRenderShaderProgram> mSimpleShaderTex;

    QSharedPointer<QDemonRenderFrameBuffer> mFrameBuffer;
    QSharedPointer<QDemonRenderTexture2D> mColorBuffer;
    QSharedPointer<QDemonRenderTexture2D> mDepthBuffer;

    quint32 mFBWidth;
    quint32 mFBHeight;

    bool m_viewportDirty = true;

    ShaderArgs mShaderArgs;
    float frus[16];
    float model[16];
    float rot[9];
    qint64 m_elapsedTime = 0;

public:
    RenderToTexture()
        : mFBWidth(400)
        , mFBHeight(400)
    {

    }
    void setupMVP(const QVector3D &translation)
    {
        float *mvp(mShaderArgs.mvp);
        ::memcpy(mvp, frus, 16 * sizeof(float));
        QDemonGl2DemoMatrixMultiply(mvp, model);
        QDemonGl2DemoMatrixTranslate(mvp, translation.x(), translation.y(), translation.z());
        QDemonGl2DemoMatrixMultiply_4x4_3x3(mvp, rot);
    }
    void DrawIndexedArrays(const QVector3D &translation)
    {
        setupMVP(translation);
        m_Context->setActiveShader(mShaderArgs.shader);
        mShaderArgs.shader->setPropertyValue("mat_mvp",
                                             *reinterpret_cast<QMatrix4x4 *>(mShaderArgs.mvp));
        mShaderArgs.shader->setPropertyValue("image0", mShaderArgs.texture.data());
        m_Context->draw(QDemonRenderDrawMode::Triangles, mIndexBuffer->getNumIndices(), 0);
    }

    // QDemonRenderExample interface
public:
    void initialize() override
    {
        m_Context = QDemonRenderContext::createGl(format());
        mInputAssembler = QDemonRenderExampleTools::createBox(m_Context, mVertexBuffer, mIndexBuffer);
        mSimpleShader = QDemonRenderExampleTools::createSimpleShader(m_Context);
        mSimpleShaderTex = QDemonRenderExampleTools::createSimpleShaderTex(m_Context);
        // If you don't want the depth buffer information back out of the system, then you can
        // do this.
        // mDepthBuffer = m_Context.CreateRenderBuffer( QDemonRenderRenderBufferFormats::Depth16,
        // mFBWidth, mFBHeight );

        m_Context->setInputAssembler(mInputAssembler);

        mDepthBuffer = m_Context->createTexture2D();
        mDepthBuffer->setTextureData(QDemonDataRef<quint8>(), 0, mFBWidth, mFBHeight,
                                     QDemonRenderTextureFormats::Depth16);
        mColorBuffer = m_Context->createTexture2D();
        mColorBuffer->setTextureData(QDemonDataRef<quint8>(), 0, mFBWidth, mFBHeight,
                                     QDemonRenderTextureFormats::RGBA8);
        if (mDepthBuffer && mColorBuffer) {
            // Creating objects tends to Bind them to their active state hooks.
            // So to protect the rest of the system against what they are doing (if we care), we
            // need
            // to push the current state
            // Auto-binds the framebuffer.
            mFrameBuffer = m_Context->createFrameBuffer();
            mFrameBuffer->attach(QDemonRenderFrameBufferAttachments::Color0, mColorBuffer);
            mFrameBuffer->attach(QDemonRenderFrameBufferAttachments::Depth, mDepthBuffer);
            Q_ASSERT(mFrameBuffer->isComplete());

            m_Context->setRenderTarget(nullptr);
        }
        mColorBuffer->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        mColorBuffer->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        m_Context->setDepthTestEnabled(true);
        m_Context->setDepthWriteEnabled(true);
        m_Context->setClearColor(QVector4D(.3f, .3f, .3f, .3f));
        // Setup various matrici
        QDemonGl2DemoMatrixIdentity(model);
        QDemonGl2DemoMatrixIdentity(frus);
        QDemonGl2DemoMatrixFrustum(frus, -1, 1, -1, 1, 1, 10);
        QDemonGl2DemoMatrixTranslate(model, 0, 0, -4);
        mShaderArgs.texture = mColorBuffer;
    }
    void drawFrame(qint64 delta) override {
        m_elapsedTime += delta;
        QDemonGl2DemoMatrixRotate_create3x3(rot, (float)m_elapsedTime * 0.1f, .707f, .707f, 0);
        QDemonRenderClearFlags clearFlags(QDemonRenderClearValues::Color | QDemonRenderClearValues::Depth);
        // render to frame buffer
        {
            QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> framebuffer(
                *m_Context.data(),
                &QDemonRenderContext::getRenderTarget,
                &QDemonRenderContext::setRenderTarget,
                mFrameBuffer);
            QDemonRenderContextScopedProperty<QRect> viewport(
                *m_Context.data(), &QDemonRenderContext::getViewport, &QDemonRenderContext::setViewport,
                QRect(0, 0, mFBWidth, mFBHeight));
            QDemonRenderContextScopedProperty<QVector4D> clearColor(
                *m_Context.data(), &QDemonRenderContext::getClearColor, &QDemonRenderContext::setClearColor,
                QVector4D(1.0f, .6f, .6f, 1.6f));
            m_Context->clear(clearFlags);
            mShaderArgs.shader = mSimpleShader;
            DrawIndexedArrays(QVector3D());
        }
        if (m_viewportDirty) {
            m_Context->setViewport(QRect(0, 0, this->width(), this->height()));
            m_viewportDirty = false;
        }

        m_Context->clear(clearFlags);
        mShaderArgs.texture = mColorBuffer;
        mShaderArgs.shader = mSimpleShaderTex;

        DrawIndexedArrays(QVector3D(-2.f, 0.f, 0.f));

        mShaderArgs.texture = mDepthBuffer;
        DrawIndexedArrays(QVector3D(2.f, 0.f, 0.f));
    }

protected:
    void resizeEvent(QResizeEvent *) override
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

    RenderToTexture renderToTexture;
    renderToTexture.setFormat(fmt);
    renderToTexture.show();

    return app.exec();
}
