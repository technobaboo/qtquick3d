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
#ifndef QDEMON_RENDER_PIXEL_GRAPHICS_TYPES_H
#define QDEMON_RENDER_PIXEL_GRAPHICS_TYPES_H

#include <QtGui/QVector2D>
#include <QtGui/QVector4D>
#include <QtGui/QMatrix3x3>

#include <QtDemon/QDemonOption>

QT_BEGIN_NAMESPACE

// Vector graphics with no scaling are pixel aligned with 0,0 being the bottom,left of the
// screen
// with coordinates increasing to the right and up.  This is opposite most window systems but it
// preserves the normal openGL assumptions about viewports and positive Y going up in general.
enum class QDemonGTypes
{
    UnknownVGType = 0,
    Layer,
    Rect,
    VertLine,
    HorzLine,
};

struct QDemonPGGraphObject
{
    QDemonGTypes type;
    constexpr QDemonPGGraphObject(QDemonGTypes inType) : type(inType) {}
};

struct QDemonPGRect : public QDemonPGGraphObject
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    QVector4D fillColor;

    constexpr QDemonPGRect() : QDemonPGGraphObject(QDemonGTypes::Rect) {}
};

struct QDemonPGVertLine : public QDemonPGGraphObject
{
    float x = 0.0f;
    float top = 0.0f;
    float bottom = 0.0f;
    QVector4D lineColor;
    void setPosition(float val) { x = val; }
    void setStart(float val) { bottom = val; }
    void setStop(float val) { top = val; }

    constexpr QDemonPGVertLine() : QDemonPGGraphObject(QDemonGTypes::VertLine) {}
};

struct QDemonPGHorzLine : public QDemonPGGraphObject
{
    float y = 0.0f;
    float left = 0.0f;
    float right = 0.0f;
    QVector4D lineColor;
    void setPosition(float val) { y = val; }
    void setStart(float val) { left = val; }
    void setStop(float val) { right = val; }

    constexpr QDemonPGHorzLine() : QDemonPGGraphObject(QDemonGTypes::HorzLine) {}
};
QT_END_NAMESPACE

#endif
