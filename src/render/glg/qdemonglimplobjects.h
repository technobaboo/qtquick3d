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
#pragma once
#ifndef QDEMON_RENDER_GL_IMPL_OBJECTS_H
#define QDEMON_RENDER_GL_IMPL_OBJECTS_H

#include <QtDemonRender/qdemonopenglutil.h>
#include <QtDemonRender/qdemonrendertexture2d.h>

QT_BEGIN_NAMESPACE

// The set of all properties as they are currently set in hardware.
struct QDemonGLHardPropertyContext
{
    QDemonRenderFrameBuffer *m_FrameBuffer;
    QSharedPointer<QDemonRenderShaderProgram> m_ActiveShader;
    QDemonRenderProgramPipeline *m_ActiveProgramPipeline;
    QDemonRenderInputAssembler *m_InputAssembler;
    QDemonRenderBlendFunctionArgument m_BlendFunction;
    QDemonRenderBlendEquationArgument m_BlendEquation;
    bool m_CullingEnabled;
    QDemonRenderBoolOp::Enum m_DepthFunction;
    bool m_BlendingEnabled;
    bool m_DepthWriteEnabled;
    bool m_DepthTestEnabled;
    bool m_StencilTestEnabled;
    bool m_ScissorTestEnabled;
    bool m_ColorWritesEnabled;
    bool m_MultisampleEnabled;
    QDemonRenderRect m_ScissorRect;
    QDemonRenderRect m_Viewport;
    QVector4D m_ClearColor;

    QDemonGLHardPropertyContext()
        : m_FrameBuffer(nullptr)
        , m_ActiveShader(nullptr)
        , m_ActiveProgramPipeline(nullptr)
        , m_InputAssembler(nullptr)
        , m_CullingEnabled(true)
        , m_DepthFunction(QDemonRenderBoolOp::Less)
        , m_BlendingEnabled(true)
        , m_DepthWriteEnabled(true)
        , m_DepthTestEnabled(true)
        , m_StencilTestEnabled(false)
        , m_ScissorTestEnabled(true)
        , m_ColorWritesEnabled(true)
        , m_MultisampleEnabled(false)
        , m_ClearColor(0, 0, 0, 1)
    {
    }
};
QT_END_NAMESPACE
#endif
