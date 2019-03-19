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
    QMatrix4x4 mvp;
    QDemonRef<QDemonRenderTexture2D> texture;
    QDemonRef<QDemonRenderShaderProgram> shader;
    ShaderArgs() {}
};
class RenderToTexture : public QDemonRenderExample
{
    QDemonRef<QDemonRenderContext> m_Context;
    QDemonRef<QDemonRenderVertexBuffer> mVertexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> mIndexBuffer;
    QDemonRef<QDemonRenderInputAssembler> mInputAssembler;
    // Simple shader
    QDemonRef<QDemonRenderShaderProgram> mSimpleShader;
    // Simple shader with texture lookup.
    QDemonRef<QDemonRenderShaderProgram> mSimpleShaderTex;

    QDemonRef<QDemonRenderFrameBuffer> mFrameBuffer;
    QDemonRef<QDemonRenderTexture2D> mColorBuffer;
    QDemonRef<QDemonRenderTexture2D> mDepthBuffer;

    quint32 mFBWidth{400};
    quint32 mFBHeight{400};

    bool m_viewportDirty = true;

    ShaderArgs mShaderArgs;
    QMatrix4x4 frus;
    QMatrix4x4 model;
    QMatrix4x4 rot;
    qint64 m_elapsedTime = 0;

public:
    RenderToTexture() = default;
    void setupMVP(const QVector3D &translation)
    {
        mShaderArgs.mvp = frus;
        mShaderArgs.mvp *= model;
        mShaderArgs.mvp.translate(translation);
        mShaderArgs.mvp *= rot;
    }
    void DrawIndexedArrays(const QVector3D &translation)
    {
        setupMVP(translation);
        m_Context->setActiveShader(mShaderArgs.shader);
        mShaderArgs.shader->setPropertyValue("mat_mvp", mShaderArgs.mvp);
        mShaderArgs.shader->setPropertyValue("image0", mShaderArgs.texture.data());
        m_Context->draw(QDemonRenderDrawMode::Triangles, mIndexBuffer->numIndices(), 0);
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

        mDepthBuffer = new QDemonRenderTexture2D(m_Context);
        mDepthBuffer->setTextureData(QDemonByteRef(), 0, mFBWidth, mFBHeight,
                                     QDemonRenderTextureFormat::Depth16);
        mColorBuffer = new QDemonRenderTexture2D(m_Context);
        mColorBuffer->setTextureData(QDemonByteRef(), 0, mFBWidth, mFBHeight,
                                     QDemonRenderTextureFormat::RGBA8);
        if (mDepthBuffer && mColorBuffer) {
            // Creating objects tends to Bind them to their active state hooks.
            // So to protect the rest of the system against what they are doing (if we care), we
            // need
            // to push the current state
            // Auto-binds the framebuffer.
            mFrameBuffer = new QDemonRenderFrameBuffer(m_Context);
            mFrameBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, mColorBuffer);
            mFrameBuffer->attach(QDemonRenderFrameBufferAttachment::Depth, mDepthBuffer);
            Q_ASSERT(mFrameBuffer->isComplete());

            m_Context->setRenderTarget(nullptr);
        }
        mColorBuffer->setMinFilter(QDemonRenderTextureMinifyingOp::Linear);
        mColorBuffer->setMagFilter(QDemonRenderTextureMagnifyingOp::Linear);
        m_Context->setDepthTestEnabled(true);
        m_Context->setDepthWriteEnabled(true);
        m_Context->setClearColor(QVector4D(.3f, .3f, .3f, .3f));
        // Setup various matrici
        frus.frustum(-1, 1, -1, 1, 1, 10);
        model.translate(0, 0, -4);
        mShaderArgs.texture = mColorBuffer;
    }
    void drawFrame(qint64 delta) override {
        m_elapsedTime += delta;
        rot = QMatrix4x4();
        rot.rotate((float)m_elapsedTime * 0.1f, .707f, .707f, 0);
        QDemonRenderClearFlags clearFlags(QDemonRenderClearValues::Color | QDemonRenderClearValues::Depth);
        // render to frame buffer
        {
            QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> framebuffer(
                *m_Context.data(),
                &QDemonRenderContext::renderTarget,
                &QDemonRenderContext::setRenderTarget,
                mFrameBuffer);
            QDemonRenderContextScopedProperty<QRect> viewport(
                *m_Context.data(), &QDemonRenderContext::viewport, &QDemonRenderContext::setViewport,
                QRect(0, 0, mFBWidth, mFBHeight));
            QDemonRenderContextScopedProperty<QVector4D> clearColor(
                *m_Context.data(), &QDemonRenderContext::clearColor, &QDemonRenderContext::setClearColor,
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
