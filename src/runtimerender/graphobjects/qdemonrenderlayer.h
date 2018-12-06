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
#ifndef QDEMON_RENDER_LAYER_H
#define QDEMON_RENDER_LAYER_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>

QT_BEGIN_NAMESPACE
class IQt3DSRenderContext;
struct SPresentation;
struct SScene;
struct SEffect;

struct AAModeValues
{
    enum Enum {
        NoAA = 0,
        SSAA = 1,
        X2 = 2,
        X4 = 4,
        X8 = 8
    };
};

struct HorizontalFieldValues
{
    enum Enum {
        LeftWidth = 0,
        LeftRight,
        WidthRight
    };
};

struct VerticalFieldValues
{
    enum Enum {
        TopHeight = 0,
        TopBottom,
        HeightBottom
    };
};

struct LayerUnitTypes
{
    enum Enum {
        Percent = 0,
        Pixels
    };
};

struct LayerBackground
{
    enum Enum {
        Transparent = 0,
        Unspecified,
        Color
    };
};

struct LayerBlendTypes
{
    enum Enum {
        Normal = 0,
        Screen,
        Multiply,
        Add,
        Subtract,
        Overlay,
        ColorBurn,
        ColorDodge
    };
};

// A layer is a special node.  It *always* presents its global transform
// to children as the identity.  It also can optionally have a width or height
// different than the overlying context.  You can think of layers as the transformation
// between a 3d scene graph and a 2D texture.
struct QDEMON_AUTOTEST_EXPORT SLayer : public SNode
{
    SScene *m_Scene;

    // First effect in a list of effects.
    SEffect *m_FirstEffect;

    // If a layer has a valid texture path (one that resolves to either a
    // an on-disk image or a offscreen renderer), then it does not render its
    // own source path.  Instead, it renders the offscreen renderer.  Used in this manner,
    // offscreen renderer's also have the option (if they support it) to render directly to the
    // render target given a specific viewport (that is also scissored if necessary).
    CRegisteredString m_TexturePath;

    SRenderPlugin *m_RenderPlugin; // Overrides texture path if available.

    AAModeValues::Enum m_ProgressiveAAMode;
    AAModeValues::Enum m_MultisampleAAMode;
    LayerBackground::Enum m_Background;
    QVector3D m_ClearColor;

    LayerBlendTypes::Enum m_BlendType;

    HorizontalFieldValues::Enum m_HorizontalFieldValues;
    float m_Left;
    LayerUnitTypes::Enum m_LeftUnits;
    float m_Width;
    LayerUnitTypes::Enum m_WidthUnits;
    float m_Right;
    LayerUnitTypes::Enum m_RightUnits;

    VerticalFieldValues::Enum m_VerticalFieldValues;
    float m_Top;
    LayerUnitTypes::Enum m_TopUnits;
    float m_Height;
    LayerUnitTypes::Enum m_HeightUnits;
    float m_Bottom;
    LayerUnitTypes::Enum m_BottomUnits;

    // Ambient occlusion
    float m_AoStrength;
    float m_AoDistance;
    float m_AoSoftness;
    float m_AoBias;
    qint32 m_AoSamplerate;
    bool m_AoDither;

    // Direct occlusion
    float m_ShadowStrength;
    float m_ShadowDist;
    float m_ShadowSoftness;
    float m_ShadowBias;

    // IBL
    SImage *m_LightProbe;
    float m_ProbeBright;
    bool m_FastIbl;
    float m_ProbeHorizon;
    float m_ProbeFov;
    SImage *m_LightProbe2;
    float m_Probe2Fade;
    float m_Probe2Window;
    float m_Probe2Pos;

    bool m_TemporalAAEnabled;

    SLayer();

    void AddEffect(SEffect &inEffect);

    SEffect *GetLastEffect();

    LayerBlendTypes::Enum GetLayerBlend()
    {
        return m_BlendType;
    }

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SNode::Remap(inRemapper);
        inRemapper.Remap(m_Scene);
        inRemapper.Remap(m_FirstEffect);
        inRemapper.Remap(m_TexturePath);
        inRemapper.Remap(m_RenderPlugin);
        inRemapper.Remap(m_LightProbe);
        inRemapper.Remap(m_LightProbe2);
    }
};
QT_END_NAMESPACE

#endif
