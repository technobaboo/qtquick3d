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

#include <QtDemonRender/qdemonrendertimerquery.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

QDemonRenderTimerQuery::QDemonRenderTimerQuery(const QDemonRef<QDemonRenderContextImpl> &context)
    : QDemonRenderQueryBase(context)
{
}

QDemonRenderTimerQuery::~QDemonRenderTimerQuery() = default;

void QDemonRenderTimerQuery::begin()
{
    m_backend->beginQuery(m_queryHandle, QDemonRenderQueryType::Timer);
}

void QDemonRenderTimerQuery::end()
{
    m_backend->endQuery(m_queryHandle, QDemonRenderQueryType::Timer);
}

void QDemonRenderTimerQuery::getResult(quint32 *params)
{
    m_backend->getQueryResult(m_queryHandle, QDemonRenderQueryResultType::Result, params);
}

void QDemonRenderTimerQuery::getResult(quint64 *params)
{
    m_backend->getQueryResult(m_queryHandle, QDemonRenderQueryResultType::Result, params);
}

void QDemonRenderTimerQuery::SetTimerQuery()
{
    m_backend->setQueryTimer(m_queryHandle);
}

QDemonRef<QDemonRenderTimerQuery> QDemonRenderTimerQuery::create(const QDemonRef<QDemonRenderContextImpl> &context)
{
    if (!context->isTimerQuerySupported())
        return nullptr;

    return QDemonRef<QDemonRenderTimerQuery>(new QDemonRenderTimerQuery(context));
}
QT_END_NAMESPACE
