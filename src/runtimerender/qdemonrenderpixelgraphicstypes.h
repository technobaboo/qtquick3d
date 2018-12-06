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
#ifndef QDEMON_RENDER_PIXEL_GRAPHICS_TYPES_H
#define QDEMON_RENDER_PIXEL_GRAPHICS_TYPES_H
#include <qdemonrender.h>
#include <QVector2D.h>
#include <QVector4D.h>
#include <QMatrix3x3.h>
#include <Qt3DSOption.h>

QT_BEGIN_NAMESPACE

// Vector graphics with no scaling are pixel aligned with 0,0 being the bottom,left of the
// screen
// with coordinates increasing to the right and up.  This is opposite most window systems but it
// preserves the normal openGL assumptions about viewports and positive Y going up in general.
struct SGTypes
{
    enum Enum {
        UnknownVGType = 0,
        Layer,
        Rect,
        VertLine,
        HorzLine,
    };
};

struct SPGGraphObject
{
    SGTypes::Enum m_Type;
    SPGGraphObject(SGTypes::Enum inType);
};

struct SPGRect : public SPGGraphObject
{
    float m_Left;
    float m_Top;
    float m_Right;
    float m_Bottom;

    QVector4D m_FillColor;

    SPGRect();
};

struct SPGVertLine : public SPGGraphObject
{
    float m_X;
    float m_Top;
    float m_Bottom;
    QVector4D m_LineColor;
    void SetPosition(float val) { m_X = val; }
    void SetStart(float val) { m_Bottom = val; }
    void SetStop(float val) { m_Top = val; }

    SPGVertLine();
};

struct SPGHorzLine : public SPGGraphObject
{
    float m_Y;
    float m_Left;
    float m_Right;
    QVector4D m_LineColor;
    void SetPosition(float val) { m_Y = val; }
    void SetStart(float val) { m_Left = val; }
    void SetStop(float val) { m_Right = val; }

    SPGHorzLine();
};
QT_END_NAMESPACE

#endif
