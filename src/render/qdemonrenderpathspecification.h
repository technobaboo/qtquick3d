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
#ifndef QDEMON_RENDER_PATH_SPECIFICATION_H
#define QDEMON_RENDER_PATH_SPECIFICATION_H

#include <QtGui/QVector2D>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;

class QDemonRenderPathSpecification
{
    QSharedPointer<QDemonRenderContextImpl> m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend

public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to render context
         * @param[in] fnd			Pointer to foundation
         *
         * @return No return.
         */
    QDemonRenderPathSpecification(QSharedPointer<QDemonRenderContextImpl> context);

    /// @QDemonRenderPathSpecification destructor
    ~QDemonRenderPathSpecification();


    /**
         * @brief reset commands and coordiantes
         *
         * @return No return.
         */
    virtual void Reset();

    /**
         * @brief add new move to command
         *
         * @param[in] inPoint		Coordinate
         *
         * @return No return.
         */
    virtual void MoveTo(QVector2D inPoint);

    /**
         * @brief add new cubic curve command
         *
         * @param[in] inC1		control point 1
         * @param[in] inC2		control point 2
         * @param[in] inDest	final point
         *
         * @return No return.
         */
    virtual void CubicCurveTo(QVector2D inC1, QVector2D inC2, QVector2D inDest);

    /**
         * @brief add new close command
         *
         *
         * @return No return.
         */
    virtual void ClosePath();

    /**
         * @brief Get path command list
         *
         *
         * @return path commands
         */
    virtual QVector<quint8> GetPathCommands() { return m_PathCommands; }

    /**
         * @brief Get path coordinates list
         *
         *
         * @return path coordinates
         */
    virtual QVector<float> GetPathCoords() { return m_PathCoords; }

private:
    QVector<quint8> m_PathCommands;
    QVector<float> m_PathCoords;

    /**
         * @brief add a new point to the coordinates
         *
         * @param[in] inPoint		Coordinate
         *
         * @return No return.
         */
    void P(QVector2D inData);

public:
    static QSharedPointer<QDemonRenderPathSpecification> CreatePathSpecification(QSharedPointer<QDemonRenderContextImpl> context);
};

QT_END_NAMESPACE

#endif
