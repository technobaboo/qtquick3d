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
#ifndef QDEMON_RENDER_PROFILER_H
#define QDEMON_RENDER_PROFILER_H

#include <QtDemon/qdemonrefcounted.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

QT_BEGIN_NAMESPACE

/**
     *	Opaque profiling system for rendering.
     */
class IRenderProfiler : public QDemonRefCounted
{
public:
    typedef QVector<QString> TStrIDVec;

protected:
    virtual ~IRenderProfiler() {}

public:
    /**
         * @brief start a timer query
         *
         * @param[in] nameID			Timer ID for tracking
         * @param[in] absoluteTime		If true the absolute GPU is queried
         * @param[in] sync				Do a sync before starting the timer
         *
         * @return no return
         */
    virtual void StartTimer(QString &nameID, bool absoluteTime, bool sync) = 0;

    /**
         * @brief stop a timer query
         *
         * @param[in] nameID			Timer ID for tracking
         *
         * @return no return
         */
    virtual void EndTimer(QString &nameID) = 0;

    /**
         * @brief Get elapsed timer value. Not this is an averaged time over several frames
         *
         * @param[in] nameID			Timer ID for tracking
         *
         * @return no return
         */
    virtual double GetElapsedTime(const QString &nameID) const = 0;

    /**
         * @brief Get ID list of tracked timers
         *
         * @return ID list
         */
    virtual const TStrIDVec &GetTimerIDs() const = 0;

    /**
         * @brief add vertex count to current counter
         *
         * @return
         */
    virtual void AddVertexCount(quint32 count) = 0;

    /**
         * @brief get current vertex count and reset
         *
         * @return
         */
    virtual quint32 GetAndResetTriangleCount() const = 0;

    static IRenderProfiler &CreateGpuProfiler(NVFoundationBase &inFoundation,
                                              IQDemonRenderContext &inContext,
                                              QDemonRenderContext &inRenderContext);
};
QT_END_NAMESPACE

#endif
