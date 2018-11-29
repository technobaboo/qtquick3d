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
#include <Qt3DSRenderPixelGraphicsTypes.h>

using namespace qt3ds;
using namespace render;

SPGGraphObject::SPGGraphObject(SGTypes::Enum inType)
    : m_Type(inType)
{
}

SPGRect::SPGRect()
    : SPGGraphObject(SGTypes::Rect)
    , m_Left(0)
    , m_Top(0)
    , m_Right(0)
    , m_Bottom(0)
    , m_FillColor(0, 0, 0, 0)
{
}

SPGVertLine::SPGVertLine()
    : SPGGraphObject(SGTypes::VertLine)
    , m_X(0)
    , m_Top(0)
    , m_Bottom(0)
    , m_LineColor(0, 0, 0, 0)
{
}

SPGHorzLine::SPGHorzLine()
    : SPGGraphObject(SGTypes::HorzLine)
    , m_Y(0)
    , m_Left(0)
    , m_Right(0)
    , m_LineColor(0, 0, 0, 0)
{
}
