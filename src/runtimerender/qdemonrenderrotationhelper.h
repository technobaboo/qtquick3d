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
#ifndef QDEMON_RENDER_ROTATION_HELPER_H
#define QDEMON_RENDER_ROTATION_HELPER_H

#include <QtDemonRuntimeRender/qdemonrendernode.h>

QT_BEGIN_NAMESPACE
/**
     *	Unfortunately we still use an XYZ-Euler rotation system.  This means that identical
     *rotations
     *	can be represented in various ways.  We need to ensure that the value that we write in the
     *	inspector palette are reasonable, however, and to do this we need a least-distance function
     *	from two different xyz tuples.
     */

struct SRotationHelper
{

    // Attempt to go for negative values intead of large positive ones
    // Goal is to keep the fabs of the angle low.
    static float ToMinimalAngle(float value)
    {
        float epsilon = (float)M_PI + .001f;
        while (fabs(value) > epsilon) {
            float tpi = (float)(2.0f * M_PI);
            if (value > 0.0f)
                value -= tpi;
            else
                value += tpi;
        }
        return value;
    }

    /**
         *	Convert an angle to a canonical form.  Return this canonical form.
         *
         *	The canonical form is defined as:
         *	1. XYZ all positive.
         *	2. XYZ all less than 360.
         *
         *	To do this we rely on two identities, the first is that given an angle, adding
         *	(or subtracting) pi from all three components does not change the angle.
         *
         *	The second is the obvious one that adding or subtracting 2*pi from any single
         *	component does not change the angle.
         *
         *	Note that this function works in radian space.
         */
    static QVector3D ToCanonicalFormStaticAxis(const QVector3D &inSrcAngle, quint32 inRotOrder)
    {
        // step 1 - reduce all components to less than 2*pi but greater than 0
        QVector3D retval(inSrcAngle);
        retval.x = ToMinimalAngle(retval.x);
        retval.y = ToMinimalAngle(retval.y);
        retval.z = ToMinimalAngle(retval.z);

        // step 2 - if any two components are equal to or greater than pi
        // then subtract pi from all three, then run two pi reduce again.

        quint32 greaterThanPiSum = 0;
        float pi = (float)M_PI;
        for (quint32 idx = 0; idx < 3; ++idx)
            if (fabs(retval[idx]) >= pi)
                greaterThanPiSum++;

        if (greaterThanPiSum > 1) {
            // For this identity to work, the middle axis angle needs to be subtracted from
            // 180 instead of added to 180 because the previous axis *reversed* it.
            quint32 theMiddleAxis = 0;

            switch (inRotOrder) {
            case EulOrdXYZs:
                theMiddleAxis = 1;
                break;
            case EulOrdXZYs:
                theMiddleAxis = 2;
                break;
            case EulOrdYXZs:
                theMiddleAxis = 0;
                break;
            case EulOrdYZXs:
                theMiddleAxis = 2;
                break;
            case EulOrdZYXs:
                theMiddleAxis = 1;
                break;
            case EulOrdZXYs:
                theMiddleAxis = 0;
                break;
            case EulOrdXYZr:
                theMiddleAxis = 1;
                break;
            case EulOrdXZYr:
                theMiddleAxis = 2;
                break;
            case EulOrdYXZr:
                theMiddleAxis = 0;
                break;
            case EulOrdYZXr:
                theMiddleAxis = 2;
                break;
            case EulOrdZYXr:
                theMiddleAxis = 1;
                break;
            case EulOrdZXYr:
                theMiddleAxis = 0;
                break;
            default:
                Q_ASSERT(false);
                return inSrcAngle;
            }
            for (quint32 idx = 0; idx < 3; ++idx) {
                if (idx == theMiddleAxis)
                    retval[idx] = pi - retval[idx];
                else
                    retval[idx] = retval[idx] > 0.0f ? retval[idx] - pi : retval[idx] + pi;
            }
        }
        return retval;
    }

    static QVector3D ToMinimalAngleDiff(const QVector3D inDiff)
    {
        return QVector3D(ToMinimalAngle(inDiff.x), ToMinimalAngle(inDiff.y),
                         ToMinimalAngle(inDiff.z));
    }

    /**
         *	Given an old angle and a new angle, return an angle has the same rotational value
         *	as the new angle *but* is as close to the old angle as possible.
         *	Works in radian space.  This function doesn't currently work for euler angles or
         *	with Euler angles with repeating axis.
         */
    static QVector3D ToNearestAngle(const QVector3D &inOldAngle, const QVector3D &inNewAngle,
                                    quint32 inRotOrder)
    {
        switch (inRotOrder) {
        case EulOrdXYZs:
        case EulOrdXZYs:
        case EulOrdYXZs:
        case EulOrdYZXs:
        case EulOrdZYXs:
        case EulOrdZXYs:
        case EulOrdXYZr:
        case EulOrdXZYr:
        case EulOrdYXZr:
        case EulOrdYZXr:
        case EulOrdZYXr:
        case EulOrdZXYr: {
            QVector3D oldA = ToCanonicalFormStaticAxis(inOldAngle, inRotOrder);
            QVector3D newA = ToCanonicalFormStaticAxis(inNewAngle, inRotOrder);
            QVector3D diff = newA - oldA;
            return inOldAngle + ToMinimalAngleDiff(diff);
        } break;
        default:
            return inNewAngle;
        }
    }
};
QT_END_NAMESPACE
#endif
