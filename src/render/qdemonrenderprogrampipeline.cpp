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

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderprogrampipeline.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

QT_BEGIN_NAMESPACE

QDemonRenderProgramPipeline::QDemonRenderProgramPipeline(const QDemonRef<QDemonRenderContext> &context)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_program(nullptr)
    , m_vertexProgram(nullptr)
    , m_fragmentProgram(nullptr)
    , m_tessControlProgram(nullptr)
    , m_tessEvalProgram(nullptr)
    , m_geometryProgram(nullptr)
    , m_computProgram(nullptr)
{
    m_programPipelineHandle = m_backend->createProgramPipeline();
}

QDemonRenderProgramPipeline::~QDemonRenderProgramPipeline()
{
    if (m_programPipelineHandle) {
        m_backend->releaseProgramPipeline(m_programPipelineHandle);
    }

    if (m_vertexProgram)
        m_vertexProgram.clear();
    if (m_fragmentProgram)
        m_fragmentProgram.clear();
    if (m_tessControlProgram)
        m_tessControlProgram.clear();
    if (m_tessEvalProgram)
        m_tessEvalProgram.clear();
    if (m_geometryProgram)
        m_geometryProgram.clear();
}

bool QDemonRenderProgramPipeline::isValid()
{
    return (m_programPipelineHandle != nullptr);
}

void QDemonRenderProgramPipeline::setProgramStages(const QDemonRef<QDemonRenderShaderProgram> &inProgram, QDemonRenderShaderTypeFlags flags)
{
    bool bDirty = false;

    if (flags & QDemonRenderShaderTypeValue::Vertex && inProgram != m_vertexProgram) {
        m_vertexProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::Fragment && inProgram != m_fragmentProgram) {
        m_fragmentProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::TessControl && inProgram != m_tessControlProgram) {
        m_tessControlProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::TessEvaluation && inProgram != m_tessEvalProgram) {
        m_tessEvalProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::Geometry && inProgram != m_geometryProgram) {
        m_geometryProgram = inProgram;
        bDirty = true;
    }

    if (bDirty) {
        m_backend->setProgramStages(m_programPipelineHandle, flags, (inProgram) ? inProgram->handle() : nullptr);
    }
}

void QDemonRenderProgramPipeline::bind()
{
    m_backend->setActiveProgramPipeline(m_programPipelineHandle);
}

QDemonRef<QDemonRenderShaderProgram> QDemonRenderProgramPipeline::getVertexStage()
{
    return m_vertexProgram;
}

QT_END_NAMESPACE
