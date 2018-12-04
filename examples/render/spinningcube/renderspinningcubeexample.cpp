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
#include "render/Qt3DSRenderVertexBuffer.h"
#include "render/Qt3DSRenderIndexBuffer.h"
#include "render/NVRenderVertFragShader.h"
#include "render_util/NVRenderUtils.h"
#include "Qt3DSRenderExampleTools.h"
#include "foundation/Qt3DSMat44.h"

using namespace qt3ds;
using namespace qt3ds::render;

#pragma warning(disable : 4189)
#pragma warning(disable : 4100)

namespace {

struct ShaderArguments
{
    QT3DSMat44 mMatrix;
};

class SpinningCube : public NVRenderExample
{
    NVRenderContext &m_Context;
    NVScopedRefCounted<NVRenderVertexBuffer> mVertexBuffer;
    NVScopedRefCounted<NVRenderIndexBuffer> mIndexBuffer;
    NVScopedRefCounted<NVRenderVertFragShader> mShader;
    NVRenderHandle mShaderArgGroupId;
    float frus[16];
    float model[16];
    float rot[9];

public:
    SpinningCube(NVRenderContext &ctx)
        : m_Context(ctx)
    {
        NVRenderExampleTools::createBox(ctx, mVertexBuffer.mPtr, mIndexBuffer.mPtr, false);
        mVertexBuffer->addRef();
        mIndexBuffer->addRef();
        mShader = NVRenderExampleTools::createSimpleShader(ctx);
        ctx.SetViewport(NVRenderRect(0, 0, 400, 400));
        // These properties will get applied just before render no matter what
        // so we can just use the default settings here.
        ctx.SetVertexBuffer(mVertexBuffer);
        ctx.SetIndexBuffer(mIndexBuffer);
        if (mShader) {
            ctx.SetActiveShader(mShader);
        }
        ctx.SetDepthTestEnabled(true);
        ctx.SetDepthWriteEnabled(true);
        NvGl2DemoMatrixIdentity(model);
        NvGl2DemoMatrixIdentity(frus);
        NvGl2DemoMatrixFrustum(frus, -1, 1, -1, 1, 1, 10);
        NvGl2DemoMatrixTranslate(model, 0, 0, -4);
    }
    virtual void drawFrame(double currentSeconds)
    {
        NvGl2DemoMatrixRotate_create3x3(rot, (float)currentSeconds * 50, .707f, .707f, 0);
        float mvp[16];
        NvGl2DemoMatrixIdentity(mvp);
        NvGl2DemoMatrixMultiply(mvp, frus);
        NvGl2DemoMatrixMultiply(mvp, model);
        NvGl2DemoMatrixMultiply_4x4_3x3(mvp, rot);

        NVConstDataRef<QT3DSU8> instance((QT3DSU8 *)mvp, 16 * sizeof(float));
        m_Context.Clear(
            NVRenderClearFlags(NVRenderClearValues::Color | NVRenderClearValues::Depth));
        mShader->SetPropertyValue("mat_mvp", *reinterpret_cast<QT3DSMat44 *>(mvp));
        m_Context.Draw(NVRenderDrawMode::Triangles, mIndexBuffer->GetNumIndices(), 0);
    }
    virtual QT3DSU32 getRuntimeInSeconds() { return mShader.mPtr ? 5 : 0; }
    virtual void release() { NVDelete(m_Context.GetFoundation(), this); }
};
}
QT3DS_RENDER_REGISTER_EXAMPLE(SpinningCube);