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
#include <QtDemonRuntimeRender/qdemonrendertext.h>

QT_BEGIN_NAMESPACE

STextRenderInfo::STextRenderInfo()
    : m_FontSize(24)
    , m_HorizontalAlignment(TextHorizontalAlignment::Center)
    , m_VerticalAlignment(TextVerticalAlignment::Middle)
    , m_Leading(0)
    , m_Tracking(0)
    , m_DropShadow(false)
    , m_DropShadowStrength(80)
    , m_DropShadowOffset(10)
    , m_DropShadowHorizontalAlignment(TextHorizontalAlignment::Right)
    , m_DropShadowVerticalAlignment(TextVerticalAlignment::Bottom)
    , m_ScaleX(0)
    , m_ScaleY(0)
    , m_EnableAcceleratedFont(false)
{
}

STextRenderInfo::~STextRenderInfo()
{
}

SText::SText()
    : SNode(GraphObjectTypes::Text)
    , m_TextColor(1, 1, 1)
    , m_TextTexture(nullptr)
{
    m_Bounds.setEmpty();
}

NVBounds3 SText::GetTextBounds() const
{
    NVBounds3 retval;
    retval.setEmpty();
    if (m_TextTexture != nullptr) {
        retval.include(m_Bounds);
    }
    return retval;
}

QT_END_NAMESPACE
