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

QDemonRenderLayer::QDemonRenderLayer()
    : QDemonGraphNode(QDemonGraphObject::Type::Layer)
    , scene(nullptr)
    , firstEffect(nullptr)
    , renderPlugin(nullptr)
    , progressiveAAMode(AAModeValues::NoAA)
    , multisampleAAMode(AAModeValues::NoAA)
    , background(LayerBackground::Transparent)
    , blendType(LayerBlendTypes::Normal)
    , horizontalFieldValues(HorizontalFieldValues::LeftWidth)
    , m_left(0)
    , leftUnits(LayerUnitTypes::Percent)
    , m_width(100.0f)
    , widthUnits(LayerUnitTypes::Percent)
    , m_right(0)
    , rightUnits(LayerUnitTypes::Percent)
    , verticalFieldValues(VerticalFieldValues::TopHeight)
    , m_top(0)
    , topUnits(LayerUnitTypes::Percent)
    , m_height(100.0f)
    , heightUnits(LayerUnitTypes::Percent)
    , m_bottom(0)
    , bottomUnits(LayerUnitTypes::Percent)
    , aoStrength(0)
    , aoDistance(5.0f)
    , aoSoftness(50.0f)
    , aoBias(0)
    , aoSamplerate(2)
    , aoDither(false)
    , shadowStrength(0)
    , shadowDist(10)
    , shadowSoftness(100.0f)
    , shadowBias(0)
    , lightProbe(nullptr)
    , probeBright(100.0f)
    , fastIbl(false)
    , probeHorizon(-1.0f)
    , probeFov(180.0f)
    , lightProbe2(nullptr)
    , probe2Fade(1.0f)
    , probe2Window(1.0f)
    , probe2Pos(0.5f)
    , temporalAAEnabled(false)
{
    flags.setLayerRenderToTarget(true);
    flags.setLayerEnableDepthTest(true);
    flags.setLayerEnableDepthPrepass(true);
}

void QDemonRenderLayer::addEffect(QDemonRenderEffect &inEffect)
{
    // Effects need to be rendered in reverse order as described in the file.
    inEffect.m_nextEffect = firstEffect;
    firstEffect = &inEffect;
    inEffect.m_layer = this;
}

QDemonRenderEffect *QDemonRenderLayer::getLastEffect()
{
    if (firstEffect) {
        QDemonRenderEffect *theEffect = firstEffect;
        // Empty loop intentional
        for (; theEffect->m_nextEffect; theEffect = theEffect->m_nextEffect) {
        }
        Q_ASSERT(theEffect->m_nextEffect == nullptr);
        return theEffect;
    }
    return nullptr;
}

QT_END_NAMESPACE
