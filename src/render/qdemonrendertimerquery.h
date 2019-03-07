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
#ifndef QDEMON_RENDER_TIMER_QUERY_H
#define QDEMON_RENDER_TIMER_QUERY_H

#include <QtDemonRender/qdemonrenderquerybase.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContext;

class QDemonRenderTimerQuery : public QDemonRenderQueryBase
{
public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] fnd			Pointer to foundation
     *
     * @return No return.
     */
    explicit QDemonRenderTimerQuery(const QDemonRef<QDemonRenderContext> &context);

    ///< destructor
    ~QDemonRenderTimerQuery() override;

    /**
     * @brief Get query type
     *
     * @return Return query type
     */
    QDemonRenderQueryType getQueryType() const override { return QDemonRenderQueryType::Timer; }

    /**
     * @brief begin a query
     *
     * @return no return.
     */
    void begin() override;

    /**
     * @brief end a query
     *
     * @return no return.
     */
    void end() override;

    /**
     * @brief Get the result of a query
     *
     * @param[out] params	Contains result of query regarding query type
     *
     * @return no return.
     */
    void getResult(quint32 *params) override;

    /**
     * @brief Get the result of a query
     *
     * @param[out] params	Contains result of query regarding query type
     *
     * @return no return.
     */
    virtual void getResult(quint64 *params);

    /**
     * @brief Places an absolute timer query into the render queue
     *		  The result can be queried with GetResult
     *
     * @return no return.
     */
    virtual void SetTimerQuery();

    /*
     * @brief static creation function
     *
     * * @return a timer query object on success
     */
    static QDemonRef<QDemonRenderTimerQuery> create(const QDemonRef<QDemonRenderContext> &context);
};

QT_END_NAMESPACE

#endif
