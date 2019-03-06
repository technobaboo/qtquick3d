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

QDemonRenderPathSpecification::QDemonRenderPathSpecification(const QDemonRef<QDemonRenderContextImpl> &context)
    : m_context(context), m_backend(context->getBackend())
{
}

QDemonRenderPathSpecification::~QDemonRenderPathSpecification() = default;

void QDemonRenderPathSpecification::reset()
{
    m_pathCommands.clear();
    m_pathCoords.clear();
}

void QDemonRenderPathSpecification::addPoint(QVector2D inData)
{
    m_pathCoords.push_back(inData.x());
    m_pathCoords.push_back(inData.y());
}

void QDemonRenderPathSpecification::moveTo(QVector2D inPoint)
{
    m_pathCommands.push_back(static_cast<uchar>(QDemonRenderPathCommands::MoveTo));
    addPoint(inPoint);
}

void QDemonRenderPathSpecification::cubicCurveTo(QVector2D inC1, QVector2D inC2, QVector2D inDest)
{
    m_pathCommands.push_back(static_cast<uchar>(QDemonRenderPathCommands::CubicCurveTo));
    addPoint(inC1);
    addPoint(inC2);
    addPoint(inDest);
}

void QDemonRenderPathSpecification::closePath()
{
    m_pathCommands.push_back(static_cast<uchar>(QDemonRenderPathCommands::Close));
}

QDemonRef<QDemonRenderPathSpecification> QDemonRenderPathSpecification::createPathSpecification(const QDemonRef<QDemonRenderContextImpl> &context)
{
    Q_ASSERT(context->isPathRenderingSupported());

    return QDemonRef<QDemonRenderPathSpecification>(new QDemonRenderPathSpecification(context));
}
QT_END_NAMESPACE
