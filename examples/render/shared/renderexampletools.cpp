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
#include "renderexampletools.h"

#include <QtDemonRender/qdemonrenderindexbuffer.h>
#include <QtDemonRender/qdemonrendervertexbuffer.h>

#include <QtGui/QVector3D>


struct BoxFace
{
    QVector3D positions[4];
    QVector3D normal;
};

static const BoxFace g_BoxFaces[] = {
    { // Z+
      QVector3D(-1, -1, 1), QVector3D(-1, 1, 1), QVector3D(1, 1, 1), QVector3D(1, -1, 1), QVector3D(0, 0, 1) },
    { // X+
      QVector3D(1, -1, 1), QVector3D(1, 1, 1), QVector3D(1, 1, -1), QVector3D(1, -1, -1), QVector3D(1, 0, 0) },
    { // Z-
      QVector3D(1, -1, -1), QVector3D(1, 1, -1), QVector3D(-1, 1, -1), QVector3D(-1, -1, -1),
      QVector3D(0, 0, -1) },
    { // X-
      QVector3D(-1, -1, -1), QVector3D(-1, 1, -1), QVector3D(-1, 1, 1), QVector3D(-1, -1, 1),
      QVector3D(-1, 0, 0) },
    { // Y+
      QVector3D(-1, 1, 1), QVector3D(-1, 1, -1), QVector3D(1, 1, -1), QVector3D(1, 1, 1), QVector3D(0, 1, 0) },
    { // Y-
      QVector3D(-1, -1, -1), QVector3D(-1, -1, 1), QVector3D(1, -1, 1), QVector3D(1, -1, -1), QVector3D(0, -1, 0) }
};

static const QVector3D g_BoxUVs[] = {
    QVector3D(0, 1, 0), QVector3D(0, 0, 0), QVector3D(1, 0, 0), QVector3D(1, 1, 0),
};

void QDemonRenderExampleTools::createBox(QDemonRenderContext &context,
                                     QDemonRenderVertexBuffer *&outVertexBuffer,
                                     QDemonRenderIndexBuffer *&outIndexBuffer, bool releaseMemory)
{
    const quint32 numVerts = 24;
    const quint32 numIndices = 36;
    QVector3D extents = QVector3D(1, 1, 1);

    QDemonRenderVertexBufferEntry entries[] = {
        QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentTypes::Float, 3, 0),
        QDemonRenderVertexBufferEntry("attr_norm", QDemonRenderComponentTypes::Float, 3, 3 * sizeof(float)),
        QDemonRenderVertexBufferEntry("attr_uv", QDemonRenderComponentTypes::Float, 2, 6 * sizeof(float)),
    };

    quint32 bufStride = 8 * sizeof(float);
    quint32 bufSize = bufStride * numVerts;
    outVertexBuffer = context.CreateVertexBuffer(
        QDemonRenderBufferUsageType::Static, QDemonConstDataRef<QDemonRenderVertexBufferEntry>(entries, 3), 0,
        releaseMemory ? 0 : bufSize);
    Q_ASSERT(bufStride == outVertexBuffer->GetStride());
    QDemonDataRef<quint8> vertData;
    if (releaseMemory)
        vertData = QDemonDataRef<quint8>(static_cast<quint8 *>(::malloc(bufSize)), bufSize);
    else
        vertData = outVertexBuffer->LockBuffer(bufSize);
    quint8 *positions = (quint8 *)vertData.begin();
    quint8 *normals = positions + 3 * sizeof(float);
    quint8 *uvs = normals + 3 * sizeof(float);

    for (quint32 i = 0; i < 6; i++) {
        const BoxFace &bf = g_BoxFaces[i];
        for (quint32 j = 0; j < 4; j++) {
            QVector3D &p = *(QVector3D *)positions;
            positions = ((quint8 *)positions) + bufStride;
            QVector3D &n = *(QVector3D *)normals;
            normals = ((quint8 *)normals) + bufStride;
            float *uv = (float *)uvs;
            uvs = ((quint8 *)uvs) + bufStride;
            n = bf.normal;
            p = bf.positions[j].multiply(extents);
            uv[0] = g_BoxUVs[j].x();
            uv[1] = g_BoxUVs[j].y();
        }
    }

    if (releaseMemory) {
        outVertexBuffer->SetBuffer(vertData, false);
        context.GetFoundation().getAllocator().deallocate(vertData.begin());
    } else
        outVertexBuffer->UnlockBuffer();

    bufSize = numIndices * sizeof(quint16);
    outIndexBuffer =
        context.CreateIndexBuffer(QDemonRenderBufferUsageType::Static, QDemonRenderComponentTypes::quint16,
                                  releaseMemory ? 0 : bufSize);
    QDemonDataRef<quint8> indexData;
    if (releaseMemory)
        indexData = QDemonDataRef<quint8>(
            (quint8 *)QT3DS_ALLOC(context.GetFoundation().getAllocator(), bufSize, "IndexData"),
            bufSize);
    else
        indexData = outIndexBuffer->LockBuffer(bufSize);
    quint16 *indices = reinterpret_cast<quint16 *>(indexData.begin());
    for (quint8 i = 0; i < 6; i++) {
        const quint16 base = i * 4;
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

inline QDemonConstDataRef<qint8> toRef(const char *data)
{
    size_t len = strlen(data) + 1;
    return QDemonConstDataRef<qint8>((const qint8 *)data, len);
}

static void dumpShaderOutput(QDemonRenderContext &ctx, const QDemonRenderVertFragCompilationResult &compResult)
{
    if (!isTrivial(compResult.mFragCompilationOutput)) {
        qWarning("Frag output:\n%s", compResult.mFragCompilationOutput);
    }
    if (!isTrivial(compResult.mVertCompilationOutput)) {
        qWarning("Vert output:\n%s", compResult.mVertCompilationOutput);
    }
    if (!isTrivial(compResult.mLinkOutput)) {
        qWarning("Link output:\n%s", compResult.mLinkOutput);
    }
}

QDemonRenderVertFragShader *compileAndDump(QDemonRenderContext &ctx, const char *name, const char *vertShader, const char *fragShader)
{
    QDemonRenderVertFragCompilationResult compResult =
        ctx.CompileSource(name, toRef(vertShader), toRef(fragShader));
    dumpShaderOutput(ctx, compResult);
    return compResult.mShader;
}
}

QDemonRenderVertFragShader *QDemonRenderExampleTools::createSimpleShader(QDemonRenderContext &ctx)
{
    return compileAndDump(ctx, "SimpleShader", getSimpleVertShader(), getSimpleFragShader());
}

QDemonRenderVertFragShader *QDemonRenderExampleTools::createSimpleShaderTex(QDemonRenderContext &ctx)
{
    return compileAndDump(ctx, "SimpleShader", getSimpleVertShader(), getSimpleFragShaderTex());
}
