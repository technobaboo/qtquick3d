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
#ifndef QDEMON_RENDER_PROGRAM_PIPLINE_H
#define QDEMON_RENDER_PROGRAM_PIPLINE_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderShaderProgram;

///< A program pipeline is a collection of a multiple programs (vertex, fragment, geometry,....)
class Q_DEMONRENDER_EXPORT QDemonRenderProgramPipeline
{
public:
    QAtomicInt ref;
protected:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend

public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to render context
         * @param[in] fnd			Pointer to foundation
         *
         * @return No return.
         */
    QDemonRenderProgramPipeline(const QDemonRef<QDemonRenderContextImpl> &context);

    /// @brief destructor
    ~QDemonRenderProgramPipeline();

    /**
         * @brief Query if pipeline is valid
         *
         * @return True if valid.
         */
    bool isValid();

    /**
         * @brief enable / disable a program stage in the pipeline
         *
         * @param[in] pProgram	Pointer to program. If nullptr stage will be disabled
         * @param[in] flags		Flags to which stage this program is bound to. Can more than
         * one stage
         *
         * @return no return.
         */
    void setProgramStages(const QDemonRef<QDemonRenderShaderProgram> &pProgram, QDemonRenderShaderTypeFlags flags);

    /**
         * @brief Make the program pipeline active
         *
         * @return True if valid.
         */
    void bind();

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendProgramPipeline getShaderHandle()
    {
        return m_programPipelineHandle;
    }

    /**
         * @brief get the vertex stage program
         *
         * @return the backend object handle.
         */
    QDemonRef<QDemonRenderShaderProgram> getVertexStage();

private:
    QDemonRenderBackend::QDemonRenderBackendProgramPipeline m_programPipelineHandle; ///< opaque backend handle

    QDemonRef<QDemonRenderShaderProgram> m_program; ///< for non separable programs this contains the entire program
    QDemonRef<QDemonRenderShaderProgram> m_vertexProgram; ///< for separable programs this contains the vertex program
    QDemonRef<QDemonRenderShaderProgram> m_fragmentProgram; ///< for separable programs this contains the fragment program
    QDemonRef<QDemonRenderShaderProgram> m_tessControlProgram; ///< for separable programs this contains the
    ///tessellation control program
    QDemonRef<QDemonRenderShaderProgram> m_tessEvalProgram; ///< for separable programs this contains the
    ///tessellation evaluation program
    QDemonRef<QDemonRenderShaderProgram> m_geometryProgram; ///< for separable programs this contains the geometry program
    QDemonRef<QDemonRenderShaderProgram> m_computProgram; ///< for separable programs this contains the compute program
};

QT_END_NAMESPACE

#endif
