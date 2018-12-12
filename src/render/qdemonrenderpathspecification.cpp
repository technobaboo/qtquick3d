/****************************************************************************
**
** Copyright (C) 2015 NVIDIA Corporation.
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

#include <qdemonrenderpathspecification.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>

QT_BEGIN_NAMESPACE

QDemonRenderPathSpecification::QDemonRenderPathSpecification(QDemonRenderContextImpl &context)
    : m_Context(context)
    , m_Backend(context.GetBackend())
{
}

QDemonRenderPathSpecification::~QDemonRenderPathSpecification() {}

void QDemonRenderPathSpecification::Reset()
{
    m_PathCommands.clear();
    m_PathCoords.clear();
}

void QDemonRenderPathSpecification::P(QVector2D inData)
{
    m_PathCoords.push_back(inData.x());
    m_PathCoords.push_back(inData.y());
}

void QDemonRenderPathSpecification::MoveTo(QVector2D inPoint)
{
    m_PathCommands.push_back(QDemonRenderPathCommands::MoveTo);
    P(inPoint);
}

void QDemonRenderPathSpecification::CubicCurveTo(QVector2D inC1, QVector2D inC2, QVector2D inDest)
{
    m_PathCommands.push_back(QDemonRenderPathCommands::CubicCurveTo);
    P(inC1);
    P(inC2);
    P(inDest);
}

void QDemonRenderPathSpecification::ClosePath()
{
    m_PathCommands.push_back(QDemonRenderPathCommands::Close);
}

QSharedPointer<QDemonRenderPathSpecification> QDemonRenderPathSpecification::CreatePathSpecification(QDemonRenderContextImpl &context)
{
    Q_ASSERT(context.IsPathRenderingSupported());

    return QSharedPointer<QDemonRenderPathSpecification>(new QDemonRenderPathSpecification(context));
}
QT_END_NAMESPACE
