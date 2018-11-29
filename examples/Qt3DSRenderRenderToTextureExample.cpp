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
#include "Qt3DSRenderExample.h"
#include "Qt3DSRenderExampleTools.h"
#include "render/Qt3DSRenderFrameBuffer.h"
#include "render/Qt3DSRenderTexture2D.h"
#include "render/Qt3DSRenderIndexBuffer.h"
#include "render/Qt3DSRenderVertexBuffer.h"
#include "render/Qt3DSRenderFrameBuffer.h"
#include "render/Qt3DSRenderRenderBuffer.h"
#include "foundation/Qt3DSVec4.h"

using namespace qt3ds;
using namespace qt3ds::render;

namespace {
struct ShaderArgs
{
    float mvp[16];
    NVRenderTexture2DPtr texture;
    NVRenderVertFragShaderPtr shader;
    ShaderArgs() {}
};
class RenderToTexture : public NVRenderExample
{
    NVRenderContext &m_Context;
    NVScopedRefCounted<NVRenderVertexBuffer> mVertexBuffer;
    NVScopedRefCounted<NVRenderIndexBuffer> mIndexBuffer;
    // Simple shader
    NVScopedRefCounted<NVRenderVertFragShader> mSimpleShader;
    // Simple shader with texture lookup.
    NVScopedRefCounted<NVRenderVertFragShader> mSimpleShaderTex;

    NVScopedRefCounted<NVRenderFrameBuffer> mFrameBuffer;
    NVScopedRefCounted<NVRenderTexture2D> mColorBuffer;
    NVScopedRefCounted<NVRenderTexture2D> mDepthBuffer;

    NVRenderHandle mGroupId;
    QT3DSU32 mFBWidth;
    QT3DSU32 mFBHeight;

    ShaderArgs mShaderArgs;
    float frus[16];
    float model[16];
    float rot[9];

public:
    RenderToTexture(NVRenderContext &context)
        : m_Context(context)
        , mFBWidth(400)
        , mFBHeight(400)
    {
        NVRenderExampleTools::createBox(m_Context, mVertexBuffer.mPtr, mIndexBuffer.mPtr);
        mVertexBuffer->addRef();
        mIndexBuffer->addRef();
        mSimpleShader = NVRenderExampleTools::createSimpleShader(m_Context);
        mSimpleShaderTex = NVRenderExampleTools::createSimpleShaderTex(m_Context);
        // If you don't want the depth buffer information back out of the system, then you can
        // do this.
        // mDepthBuffer = m_Context.CreateRenderBuffer( NVRenderRenderBufferFormats::Depth16,
        // mFBWidth, mFBHeight );

        mDepthBuffer = m_Context.CreateTexture2D();
        mDepthBuffer->SetTextureData(NVDataRef<QT3DSU8>(), 0, mFBWidth, mFBHeight,
                                     NVRenderTextureFormats::Depth16);
        mColorBuffer = m_Context.CreateTexture2D();
        mColorBuffer->SetTextureData(NVDataRef<QT3DSU8>(), 0, mFBWidth, mFBHeight,
                                     NVRenderTextureFormats::RGBA8);
        if (mDepthBuffer.mPtr && mColorBuffer.mPtr) {
            // Creating objects tends to Bind them to their active state hooks.
            // So to protect the rest of the system against what they are doing (if we care), we
            // need
            // to push the current state
            // Auto-binds the framebuffer.
            mFrameBuffer = m_Context.CreateFrameBuffer();
            mFrameBuffer->Attach(NVRenderFrameBufferAttachments::Color0, *mColorBuffer.mPtr);
            mFrameBuffer->Attach(NVRenderFrameBufferAttachments::Depth, *mDepthBuffer.mPtr);
            QT3DS_ASSERT(mFrameBuffer->IsComplete());

            m_Context.SetRenderTarget(NULL);
        }
        mColorBuffer->SetMinFilter(NVRenderTextureMinifyingOp::Linear);
        mColorBuffer->SetMagFilter(NVRenderTextureMagnifyingOp::Linear);
        m_Context.SetVertexBuffer(mVertexBuffer);
        m_Context.SetIndexBuffer(mIndexBuffer);
        m_Context.SetDepthTestEnabled(true);
        m_Context.SetDepthWriteEnabled(true);
        m_Context.SetClearColor(QT3DSVec4(.3f));
        // Setup various matrici
        NvGl2DemoMatrixIdentity(model);
        NvGl2DemoMatrixIdentity(frus);
        NvGl2DemoMatrixFrustum(frus, -1, 1, -1, 1, 1, 10);
        NvGl2DemoMatrixTranslate(model, 0, 0, -4);
        mShaderArgs.texture = mColorBuffer.mPtr;
    }
    void setupMVP(QT3DSVec3 translation)
    {
        float *mvp(mShaderArgs.mvp);
        memCopy(mvp, frus, 16 * sizeof(float));
        NvGl2DemoMatrixMultiply(mvp, model);
        NvGl2DemoMatrixTranslate(mvp, translation.x, translation.y, translation.z);
        NvGl2DemoMatrixMultiply_4x4_3x3(mvp, rot);
    }
    void DrawIndexedArrays(QT3DSVec3 translation)
    {
        setupMVP(translation);
        m_Context.SetActiveShader(mShaderArgs.shader);
        mShaderArgs.shader->Bind();
        mShaderArgs.shader->SetPropertyValue("mat_mvp",
                                             *reinterpret_cast<QT3DSMat44 *>(mShaderArgs.mvp));
        mShaderArgs.shader->SetPropertyValue("image0", mShaderArgs.texture);
        m_Context.Draw(NVRenderDrawMode::Triangles, mIndexBuffer->GetNumIndices(), 0);
    }

    virtual void drawFrame(double currentSeconds)
    {
        NvGl2DemoMatrixRotate_create3x3(rot, (float)currentSeconds * 50, .707f, .707f, 0);
        NVRenderClearFlags clearFlags(NVRenderClearValues::Color | NVRenderClearValues::Depth);
        // render to frame buffer
        {
            NVRenderContextScopedProperty<NVRenderFrameBufferPtr> __framebuffer(
                m_Context, &NVRenderContext::GetRenderTarget, &NVRenderContext::SetRenderTarget,
                mFrameBuffer);
            NVRenderContextScopedProperty<NVRenderRect> __viewport(
                m_Context, &NVRenderContext::GetViewport, &NVRenderContext::SetViewport,
                NVRenderRect(0, 0, mFBWidth, mFBHeight));
            NVRenderContextScopedProperty<QT3DSVec4> __clearColor(
                m_Context, &NVRenderContext::GetClearColor, &NVRenderContext::SetClearColor,
                QT3DSVec4(.6f));
            m_Context.Clear(clearFlags);
            mShaderArgs.shader = mSimpleShader;
            DrawIndexedArrays(QT3DSVec3(0.f));
        }
        m_Context.Clear(clearFlags);
        mShaderArgs.texture = mColorBuffer;
        mShaderArgs.shader = mSimpleShaderTex;

        DrawIndexedArrays(QT3DSVec3(-2.f, 0.f, 0.f));

        mShaderArgs.texture = mDepthBuffer;
        DrawIndexedArrays(QT3DSVec3(2.f, 0.f, 0.f));
    }
    virtual QT3DSU32 getRuntimeInSeconds()
    {
        return mSimpleShader.mPtr && mSimpleShaderTex.mPtr ? 5 : 0;
    }
    virtual void release() { NVDelete(m_Context.GetFoundation(), this); }
};
}

QT3DS_RENDER_REGISTER_EXAMPLE(RenderToTexture);