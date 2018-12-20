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
#include <QtDemonRuntimeRender/qdemonrenderlight.h>

QT_BEGIN_NAMESPACE

SLight::SLight()
    : SNode(GraphObjectTypes::Light)
    , m_LightType(RenderLightTypes::Directional)
    , m_Scope(nullptr)
    , m_DiffuseColor(1, 1, 1)
    , m_SpecularColor(1, 1, 1)
    , m_AmbientColor(0, 0, 0)
    , m_Brightness(100)
    , m_LinearFade(0)
    , m_ExponentialFade(0)
    , m_AreaWidth(0)
    , m_AreaHeight(0)
    , m_CastShadow(false)
    , m_ShadowBias(0.0f)
    , m_ShadowFactor(5.0f)
    , m_ShadowMapRes(9)
    , m_ShadowMapFar(5000.0f)
    , m_ShadowMapFov(90.0f)
    , m_ShadowFilter(35.0f)
{
    m_Flags.SetPointLight(0);
}

QT_END_NAMESPACE
