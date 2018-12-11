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
#ifndef QDEMON_RENDER_SHADER_H
#define QDEMON_RENDER_SHADER_H

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

///< A shader program is an object composed of a multiple shaders (vertex, fragment,
///geometry,....)
class QDemonRenderShader
{
protected:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    QDemonConstDataRef<qint8> m_Source; ///< shader source code
    bool m_Binary; ///< true for binary programs
    QString m_ErrorMessage; ///< contains the error message if linking fails

public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to render context
         * @param[in] fnd			Pointer to foundation
         *
         * @return No return.
         */
    QDemonRenderShader(QDemonRenderContextImpl &context,
                       QDemonConstDataRef<qint8> source, bool binaryProgram)
        : m_Context(context)
        , m_Backend(context.GetBackend())
        , m_Source(source)
        , m_Binary(binaryProgram)
    {
    }

    /// @brief destructor
    ~QDemonRenderShader(){}

    /**
         * @brief Query if shader compiled succesfuly
         *
         * @return True if shader is valid.
         */
    virtual bool IsValid() = 0;

    /**
         * @brief Get Error Message
         *
         * @param[out] messageLength	Pointer to error string
         * @param[out] messageLength	Size of error meesage
         *
         * @return no return
         */
    virtual void GetErrorMessage(qint32 *messageLength, const char *errorMessage)
    {
        // Since we do not have any error message just generate a generic one
        if (m_Binary)
            m_ErrorMessage = QStringLiteral("Binary shader compilation failed");

        *messageLength = m_ErrorMessage.size();
        errorMessage = qPrintable(m_ErrorMessage);
    }

    /**
         * @brief Get Error Message
         *
         *
         * @return error message.
         */
    virtual const char *GetErrorMessage()
    {
        if (m_Binary)
            m_ErrorMessage = QStringLiteral("Binary shader compilation failed");

        return qPrintable(m_ErrorMessage);
    }
};

QT_END_NAMESPACE

#endif
