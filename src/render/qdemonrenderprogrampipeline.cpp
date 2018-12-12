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

QDemonRenderProgramPipeline::QDemonRenderProgramPipeline(QSharedPointer<QDemonRenderContextImpl> context)
    : m_Context(context)
    , m_Backend(context->GetBackend())
    , m_Program(nullptr)
    , m_VertexProgram(nullptr)
    , m_FragmentProgram(nullptr)
    , m_TessControlProgram(nullptr)
    , m_TessEvalProgram(nullptr)
    , m_GeometryProgram(nullptr)
    , m_ComputProgram(nullptr)
{
    m_ProgramPipelineHandle = m_Backend->CreateProgramPipeline();
}

QDemonRenderProgramPipeline::~QDemonRenderProgramPipeline()
{
    if (m_ProgramPipelineHandle) {
        m_Backend->ReleaseProgramPipeline(m_ProgramPipelineHandle);
    }

    if (m_VertexProgram)
        m_VertexProgram.clear();
    if (m_FragmentProgram)
        m_FragmentProgram.clear();
    if (m_TessControlProgram)
        m_TessControlProgram.clear();
    if (m_TessEvalProgram)
        m_TessEvalProgram.clear();
    if (m_GeometryProgram)
        m_GeometryProgram.clear();
}

bool QDemonRenderProgramPipeline::IsValid() { return (m_ProgramPipelineHandle != nullptr); }

void QDemonRenderProgramPipeline::SetProgramStages(QSharedPointer<QDemonRenderShaderProgram> inProgram,
                                                   QDemonRenderShaderTypeFlags flags)
{
    bool bDirty = false;

    if (flags & QDemonRenderShaderTypeValue::Vertex && inProgram != m_VertexProgram) {
        m_VertexProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::Fragment && inProgram != m_FragmentProgram) {
        m_FragmentProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::TessControl && inProgram != m_TessControlProgram) {
        m_TessControlProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::TessEvaluation && inProgram != m_TessEvalProgram) {
        m_TessEvalProgram = inProgram;
        bDirty = true;
    }
    if (flags & QDemonRenderShaderTypeValue::Geometry && inProgram != m_GeometryProgram) {
        m_GeometryProgram = inProgram;
        bDirty = true;
    }

    if (bDirty) {
        m_Backend->SetProgramStages(m_ProgramPipelineHandle, flags,
                                    (inProgram) ? inProgram->GetShaderProgramHandle() : nullptr);
    }
}

void QDemonRenderProgramPipeline::Bind()
{
    m_Backend->SetActiveProgramPipeline(m_ProgramPipelineHandle);
}
QT_END_NAMESPACE
