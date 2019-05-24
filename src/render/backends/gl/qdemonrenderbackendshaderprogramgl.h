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
#ifndef QDEMON_RENDER_BACKEND_SHADER_PROGRAM_GL_H
#define QDEMON_RENDER_BACKEND_SHADER_PROGRAM_GL_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemon/qdemonutils.h>
#include <QtDemon/QDemonOption>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

struct QDemonRenderBackendShaderInputEntryGL
{
    QByteArray m_attribName; ///< must be the same name as used in the vertex shader
    quint32 m_attribLocation; ///< attribute index
    quint32 m_type; ///< GL vertex format type @sa GL_FLOAT, GL_INT
    quint32 m_numComponents; ///< component count. max 4
};

///< this class handles the shader input variables
class QDemonRenderBackendShaderInputGL
{
public:
    ///< constructor
    QDemonRenderBackendShaderInputGL(QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> entries)
        : m_shaderInputEntries(entries)
    {
    }
    ///< destructor
    ~QDemonRenderBackendShaderInputGL() {}

    QDemonRenderBackendShaderInputEntryGL *getEntryByName(const QString &entryName) const
    {
        for (int idx = 0; idx != m_shaderInputEntries.size(); ++idx) {
            if (m_shaderInputEntries[idx].m_attribName == entryName)
                return &m_shaderInputEntries.mData[idx];
        }
        return nullptr;
    }

    QDemonOption<QDemonRenderBackendShaderInputEntryGL> getEntryByAttribLocation(quint32 attribLocation) const
    {
        for (int idx = 0; idx != m_shaderInputEntries.size(); ++idx) {
            if (m_shaderInputEntries[idx].m_attribLocation == attribLocation)
                return m_shaderInputEntries[idx];
        }
        return QDemonEmpty();
    }

    QDemonDataRef<QDemonRenderBackendShaderInputEntryGL> m_shaderInputEntries; ///< shader input entries
};

///< this class represents the internals of a GL program
class QDemonRenderBackendShaderProgramGL
{
public:
    ///< constructor
    QDemonRenderBackendShaderProgramGL(quint32 programID) : m_programID(programID), m_shaderInput(nullptr) {}

    ///< destructor
    ~QDemonRenderBackendShaderProgramGL() {}

    quint32 m_programID; ///< this is the OpenGL object ID
    QDemonRenderBackendShaderInputGL *m_shaderInput; ///< pointer to shader input object
};

QT_END_NAMESPACE

#endif
