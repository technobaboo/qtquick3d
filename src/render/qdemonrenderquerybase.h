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
#ifndef QDEMON_RENDER_QUERY_BASE_H
#define QDEMON_RENDER_QUERY_BASE_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContext;
class QDemonRenderBackend;

///< Base class
class QDemonRenderQueryBase
{
public:
    QAtomicInt ref;

protected:
    QDemonRef<QDemonRenderContext> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBackend::QDemonRenderBackendQueryObject m_queryHandle; ///< opaque backend handle

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] fnd			Pointer to foundation
     *
     * @return No return.
     */
    QDemonRenderQueryBase(const QDemonRef<QDemonRenderContext> &context);

    virtual ~QDemonRenderQueryBase();

    /**
     * @brief Get query type
     *
     * @return Return query type
     */
    virtual QDemonRenderQueryType getQueryType() const = 0;

    /**
     * @brief begin a query
     *
     * @return no return.
     */
    virtual void begin() = 0;

    /**
     * @brief end a query
     *
     * @return no return.
     */
    virtual void end() = 0;

    /**
     * @brief Get the result of a query
     *
     * @param[out] params	Contains result of query regarding query type
     *
     * @return no return.
     */
    virtual void getResult(quint32 *params) = 0;

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    virtual QDemonRenderBackend::QDemonRenderBackendQueryObject getQuerytHandle() const { return m_queryHandle; }
};

QT_END_NAMESPACE

#endif
