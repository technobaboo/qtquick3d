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

#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>

QT_BEGIN_NAMESPACE

SLayer::SLayer()
    : SNode(GraphObjectTypes::Layer)
    , m_Scene(nullptr)
    , m_FirstEffect(nullptr)
    , m_RenderPlugin(nullptr)
    , m_ProgressiveAAMode(AAModeValues::NoAA)
    , m_MultisampleAAMode(AAModeValues::NoAA)
    , m_Background(LayerBackground::Transparent)
    , m_BlendType(LayerBlendTypes::Normal)
    , m_HorizontalFieldValues(HorizontalFieldValues::LeftWidth)
    , m_Left(0)
    , m_LeftUnits(LayerUnitTypes::Percent)
    , m_Width(100.0f)
    , m_WidthUnits(LayerUnitTypes::Percent)
    , m_Right(0)
    , m_RightUnits(LayerUnitTypes::Percent)
    , m_VerticalFieldValues(VerticalFieldValues::TopHeight)
    , m_Top(0)
    , m_TopUnits(LayerUnitTypes::Percent)
    , m_Height(100.0f)
    , m_HeightUnits(LayerUnitTypes::Percent)
    , m_Bottom(0)
    , m_BottomUnits(LayerUnitTypes::Percent)
    , m_AoStrength(0)
    , m_AoDistance(5.0f)
    , m_AoSoftness(50.0f)
    , m_AoBias(0)
    , m_AoSamplerate(2)
    , m_AoDither(false)
    , m_ShadowStrength(0)
    , m_ShadowDist(10)
    , m_ShadowSoftness(100.0f)
    , m_ShadowBias(0)
    , m_LightProbe(nullptr)
    , m_ProbeBright(100.0f)
    , m_FastIbl(false)
    , m_ProbeHorizon(-1.0f)
    , m_ProbeFov(180.0f)
    , m_LightProbe2(nullptr)
    , m_Probe2Fade(1.0f)
    , m_Probe2Window(1.0f)
    , m_Probe2Pos(0.5f)
    , m_TemporalAAEnabled(false)
{
    m_Flags.SetLayerRenderToTarget(true);
    m_Flags.SetLayerEnableDepthTest(true);
    m_Flags.SetLayerEnableDepthPrepass(true);
}

void SLayer::AddEffect(SEffect &inEffect)
{
    // Effects need to be rendered in reverse order as described in the file.
    inEffect.m_NextEffect = m_FirstEffect;
    m_FirstEffect = &inEffect;
    inEffect.m_Layer = this;
}

SEffect *SLayer::GetLastEffect()
{
    if (m_FirstEffect) {
        SEffect *theEffect = m_FirstEffect;
        // Empty loop intentional
        for (; theEffect->m_NextEffect; theEffect = theEffect->m_NextEffect) {
        }
        Q_ASSERT(theEffect->m_NextEffect == nullptr);
        return theEffect;
    }
    return nullptr;
}

QT_END_NAMESPACE
