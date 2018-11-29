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
#include "Qt3DSRenderExampleTools.h"
#include "render/Qt3DSRenderIndexBuffer.h"
#include "render/Qt3DSRenderVertexBuffer.h"
#include "render_util/NVRenderUtils.h"
#include "foundation/Qt3DSVec3.h"
#include "foundation/Qt3DSVec4.h"

using namespace qt3ds;
using namespace qt3ds::render;

struct BoxFace
{
    QT3DSVec3 positions[4];
    QT3DSVec3 normal;
};

static const BoxFace g_BoxFaces[] = {
    { // Z+
      QT3DSVec3(-1, -1, 1), QT3DSVec3(-1, 1, 1), QT3DSVec3(1, 1, 1), QT3DSVec3(1, -1, 1), QT3DSVec3(0, 0, 1) },
    { // X+
      QT3DSVec3(1, -1, 1), QT3DSVec3(1, 1, 1), QT3DSVec3(1, 1, -1), QT3DSVec3(1, -1, -1), QT3DSVec3(1, 0, 0) },
    { // Z-
      QT3DSVec3(1, -1, -1), QT3DSVec3(1, 1, -1), QT3DSVec3(-1, 1, -1), QT3DSVec3(-1, -1, -1),
      QT3DSVec3(0, 0, -1) },
    { // X-
      QT3DSVec3(-1, -1, -1), QT3DSVec3(-1, 1, -1), QT3DSVec3(-1, 1, 1), QT3DSVec3(-1, -1, 1),
      QT3DSVec3(-1, 0, 0) },
    { // Y+
      QT3DSVec3(-1, 1, 1), QT3DSVec3(-1, 1, -1), QT3DSVec3(1, 1, -1), QT3DSVec3(1, 1, 1), QT3DSVec3(0, 1, 0) },
    { // Y-
      QT3DSVec3(-1, -1, -1), QT3DSVec3(-1, -1, 1), QT3DSVec3(1, -1, 1), QT3DSVec3(1, -1, -1), QT3DSVec3(0, -1, 0) }
};

static const QT3DSVec3 g_BoxUVs[] = {
    QT3DSVec3(0, 1, 0), QT3DSVec3(0, 0, 0), QT3DSVec3(1, 0, 0), QT3DSVec3(1, 1, 0),
};

void NVRenderExampleTools::createBox(NVRenderContext &context,
                                     NVRenderVertexBuffer *&outVertexBuffer,
                                     NVRenderIndexBuffer *&outIndexBuffer, bool releaseMemory)
{
    const QT3DSU32 numVerts = 24;
    const QT3DSU32 numIndices = 36;
    QT3DSVec3 extents = QT3DSVec3(1, 1, 1);

    NVRenderVertexBufferEntry entries[] = {
        NVRenderVertexBufferEntry("attr_pos", NVRenderComponentTypes::QT3DSF32, 3, 0),
        NVRenderVertexBufferEntry("attr_norm", NVRenderComponentTypes::QT3DSF32, 3, 3 * sizeof(QT3DSF32)),
        NVRenderVertexBufferEntry("attr_uv", NVRenderComponentTypes::QT3DSF32, 2, 6 * sizeof(QT3DSF32)),
    };

    QT3DSU32 bufStride = 8 * sizeof(QT3DSF32);
    QT3DSU32 bufSize = bufStride * numVerts;
    outVertexBuffer = context.CreateVertexBuffer(
        NVRenderBufferUsageType::Static, NVConstDataRef<NVRenderVertexBufferEntry>(entries, 3), 0,
        releaseMemory ? 0 : bufSize);
    QT3DS_ASSERT(bufStride == outVertexBuffer->GetStride());
    NVDataRef<QT3DSU8> vertData;
    if (releaseMemory)
        vertData = NVDataRef<QT3DSU8>(
            (QT3DSU8 *)QT3DS_ALLOC(context.GetFoundation().getAllocator(), bufSize, "VertexBufferData"),
            bufSize);
    else
        vertData = outVertexBuffer->LockBuffer(bufSize);
    QT3DSU8 *positions = (QT3DSU8 *)vertData.begin();
    QT3DSU8 *normals = positions + 3 * sizeof(QT3DSF32);
    QT3DSU8 *uvs = normals + 3 * sizeof(QT3DSF32);

    for (QT3DSU32 i = 0; i < 6; i++) {
        const BoxFace &bf = g_BoxFaces[i];
        for (QT3DSU32 j = 0; j < 4; j++) {
            QT3DSVec3 &p = *(QT3DSVec3 *)positions;
            positions = ((QT3DSU8 *)positions) + bufStride;
            QT3DSVec3 &n = *(QT3DSVec3 *)normals;
            normals = ((QT3DSU8 *)normals) + bufStride;
            QT3DSF32 *uv = (QT3DSF32 *)uvs;
            uvs = ((QT3DSU8 *)uvs) + bufStride;
            n = bf.normal;
            p = bf.positions[j].multiply(extents);
            uv[0] = g_BoxUVs[j].x;
            uv[1] = g_BoxUVs[j].y;
        }
    }

    if (releaseMemory) {
        outVertexBuffer->SetBuffer(vertData, false);
        context.GetFoundation().getAllocator().deallocate(vertData.begin());
    } else
        outVertexBuffer->UnlockBuffer();

    bufSize = numIndices * sizeof(QT3DSU16);
    outIndexBuffer =
        context.CreateIndexBuffer(NVRenderBufferUsageType::Static, NVRenderComponentTypes::QT3DSU16,
                                  releaseMemory ? 0 : bufSize);
    NVDataRef<QT3DSU8> indexData;
    if (releaseMemory)
        indexData = NVDataRef<QT3DSU8>(
            (QT3DSU8 *)QT3DS_ALLOC(context.GetFoundation().getAllocator(), bufSize, "IndexData"),
            bufSize);
    else
        indexData = outIndexBuffer->LockBuffer(bufSize);
    QT3DSU16 *indices = reinterpret_cast<QT3DSU16 *>(indexData.begin());
    for (QT3DSU8 i = 0; i < 6; i++) {
        const QT3DSU16 base = i * 4;
        *(indices++) = base + 0;
        *(indices++) = base + 1;
        *(indices++) = base + 2;
        *(indices++) = base + 0;
        *(indices++) = base + 2;
        *(indices++) = base + 3;
    }
    if (releaseMemory) {
        outIndexBuffer->SetBuffer(indexData, false);
        context.GetFoundation().getAllocator().deallocate(indexData.begin());
    } else
        outIndexBuffer->UnlockBuffer();
}

namespace {

inline NVConstDataRef<QT3DSI8> toRef(const char *data)
{
    size_t len = strlen(data) + 1;
    return NVConstDataRef<QT3DSI8>((const QT3DSI8 *)data, len);
}

static void dumpShaderOutput(NVRenderContext &ctx,
                             const NVRenderVertFragCompilationResult &compResult)
{
    if (!isTrivial(compResult.mFragCompilationOutput)) {
        ctx.GetFoundation().error(QT3DS_WARN, "Frag output:\n%s", compResult.mFragCompilationOutput);
    }
    if (!isTrivial(compResult.mVertCompilationOutput)) {
        ctx.GetFoundation().error(QT3DS_WARN, "Vert output:\n%s", compResult.mVertCompilationOutput);
    }
    if (!isTrivial(compResult.mLinkOutput)) {
        ctx.GetFoundation().error(QT3DS_WARN, "Link output:\n%s", compResult.mLinkOutput);
    }
}

NVRenderVertFragShader *compileAndDump(NVRenderContext &ctx, const char *name,
                                       const char *vertShader, const char *fragShader)
{
    NVRenderVertFragCompilationResult compResult =
        ctx.CompileSource(name, toRef(vertShader), toRef(fragShader));
    dumpShaderOutput(ctx, compResult);
    return compResult.mShader;
}
}

NVRenderVertFragShader *NVRenderExampleTools::createSimpleShader(NVRenderContext &ctx)
{
    return compileAndDump(ctx, "SimpleShader", getSimpleVertShader(), getSimpleFragShader());
}

NVRenderVertFragShader *NVRenderExampleTools::createSimpleShaderTex(NVRenderContext &ctx)
{
    return compileAndDump(ctx, "SimpleShader", getSimpleVertShader(), getSimpleFragShaderTex());
}