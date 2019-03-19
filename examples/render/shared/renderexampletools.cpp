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
#include <QtDemonRender/qdemonrenderattriblayout.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

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

static const QVector2D g_BoxUVs[] = {
    QVector2D(0, 1), QVector2D(0, 0), QVector2D(1, 0), QVector2D(1, 1),
};

QDemonRef<QDemonRenderInputAssembler> QDemonRenderExampleTools::createBox(QDemonRef<QDemonRenderContext> context,
                                                                               QDemonRef<QDemonRenderVertexBuffer> &outVertexBuffer,
                                                                               QDemonRef<QDemonRenderIndexBuffer> &outIndexBuffer)
{
    const quint32 numVerts = 24;
    const quint32 numIndices = 36;
    QVector3D extents = QVector3D(1, 1, 1);

    // Attribute Layouts
    QDemonRenderVertexBufferEntry entries[] = {
        QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 3, 0),
        QDemonRenderVertexBufferEntry("attr_norm", QDemonRenderComponentType::Float32, 3, 3 * sizeof(float)),
        QDemonRenderVertexBufferEntry("attr_uv", QDemonRenderComponentType::Float32, 2, 6 * sizeof(float)),
    };

    QDemonRef<QDemonRenderAttribLayout> attribLayout = context->createAttributeLayout(toDataView(entries, 3));

    // Vertex Buffer
    struct Vertex {
        QVector3D position;
        QVector3D normal;
        QVector2D uv;
    };
    Q_STATIC_ASSERT(sizeof(Vertex) == 8 * sizeof(float));

    quint32 bufStride = sizeof(Vertex);
    QVector<Vertex> vertices(numVerts);

    Vertex *v =vertices.begin();
    for (quint32 i = 0; i < 6; i++) {
        const BoxFace &bf = g_BoxFaces[i];
        for (quint32 j = 0; j < 4; j++) {
            v->position = bf.positions[j] * extents;
            v->normal = bf.normal;
            v->uv = g_BoxUVs[j];
            ++v;
        }
    }

    auto vertexDataRef = toByteView(vertices);
    outVertexBuffer= new QDemonRenderVertexBuffer(context, QDemonRenderBufferUsageType::Static, bufStride, vertexDataRef);
    Q_ASSERT(bufStride == outVertexBuffer->stride());

    // Index Buffer
    QVector<quint16> indexBuffer(numIndices);
    quint16 *indices = indexBuffer.begin();
    for (quint8 i = 0; i < 6; i++) {
        const quint16 base = i * 4;
        *(indices++) = base + 0;
        *(indices++) = base + 1;
        *(indices++) = base + 2;
        *(indices++) = base + 0;
        *(indices++) = base + 2;
        *(indices++) = base + 3;
    }
    auto indexDataRef = toByteView(indexBuffer);
    outIndexBuffer= new QDemonRenderIndexBuffer(context, QDemonRenderBufferUsageType::Static,
                                               QDemonRenderComponentType::UnsignedInteger16,
                                               indexDataRef);

    quint32 strides = outVertexBuffer->stride();
    quint32 offsets = 0;

    QDemonRef<QDemonRenderInputAssembler> inputAssembler = context->createInputAssembler(attribLayout,
                                                                                              toDataView(&outVertexBuffer, 1),
                                                                                              outIndexBuffer,
                                                                                              toDataView(&strides, 1),
                                                                                              toDataView(&offsets, 1));
    return inputAssembler;
}


namespace {

inline QDemonDataView<qint8> toRef(const char *data)
{
    size_t len = strlen(data) + 1;
    return QDemonDataView<qint8>((const qint8 *)data, len);
}

static void dumpShaderOutput(QDemonRef<QDemonRenderContext> ctx, const QDemonRenderVertFragCompilationResult &compResult)
{
    //    if (!isTrivial(compResult.mFragCompilationOutput)) {
    //        qWarning("Frag output:\n%s", compResult.mFragCompilationOutput);
    //    }
    //    if (!isTrivial(compResult.mVertCompilationOutput)) {
    //        qWarning("Vert output:\n%s", compResult.mVertCompilationOutput);
    //    }
    //    if (!isTrivial(compResult.mLinkOutput)) {
    //        qWarning("Link output:\n%s", compResult.mLinkOutput);
    //    }
}

QDemonRef<QDemonRenderShaderProgram> compileAndDump(QDemonRef<QDemonRenderContext> ctx, const char *name, const char *vertShader, const char *fragShader)
{
    QDemonRenderVertFragCompilationResult compResult =
            ctx->compileSource(name, toRef(vertShader), toRef(fragShader));
    dumpShaderOutput(ctx, compResult);
    return compResult.m_shader;
}
}

QDemonRef<QDemonRenderShaderProgram> QDemonRenderExampleTools::createSimpleShader(QDemonRef<QDemonRenderContext> ctx)
{
    return compileAndDump(ctx, "SimpleShader", getSimpleVertShader(), getSimpleFragShader());
}

QDemonRef<QDemonRenderShaderProgram> QDemonRenderExampleTools::createSimpleShaderTex(QDemonRef<QDemonRenderContext> ctx)
{
    return compileAndDump(ctx, "SimpleShader", getSimpleVertShader(), getSimpleFragShaderTex());
}
