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
#ifndef QDEMON_RENDER_LAYER_H
#define QDEMON_RENDER_LAYER_H

#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>

QT_BEGIN_NAMESPACE
class QDemonRenderContextInterface;
struct QDemonPresentation;
struct QDemonRenderEffect;
struct SRenderPlugin; // TODO: ???
struct QDemonRenderImage;

struct AAModeValues
{
    enum Enum { NoAA = 0, SSAA = 1, X2 = 2, X4 = 4, X8 = 8 };
};

struct HorizontalFieldValues
{
    enum Enum { LeftWidth = 0, LeftRight, WidthRight };
};

struct VerticalFieldValues
{
    enum Enum { TopHeight = 0, TopBottom, HeightBottom };
};

struct LayerUnitTypes
{
    enum Enum { Percent = 0, Pixels };
};

struct LayerBackground
{
    enum Enum { Transparent = 0, Unspecified, Color };
};

struct LayerBlendTypes
{
    enum Enum { Normal = 0, Screen, Multiply, Add, Subtract, Overlay, ColorBurn, ColorDodge };
};

// A layer is a special node.  It *always* presents its global transform
// to children as the identity.  It also can optionally have a width or height
// different than the overlying context.  You can think of layers as the transformation
// between a 3d scene graph and a 2D texture.
struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderLayer : public QDemonGraphNode
{
    QDemonRenderScene *scene;

    // First effect in a list of effects.
    QDemonRenderEffect *firstEffect;

    // If a layer has a valid texture path (one that resolves to either a
    // an on-disk image or a offscreen renderer), then it does not render its
    // own source path.  Instead, it renders the offscreen renderer.  Used in this manner,
    // offscreen renderer's also have the option (if they support it) to render directly to the
    // render target given a specific viewport (that is also scissored if necessary).
    QString texturePath;

    SRenderPlugin *renderPlugin; // Overrides texture path if available.

    AAModeValues::Enum progressiveAAMode;
    AAModeValues::Enum multisampleAAMode;
    LayerBackground::Enum background;
    QVector3D clearColor;

    LayerBlendTypes::Enum blendType;

    HorizontalFieldValues::Enum horizontalFieldValues;
    float m_left;
    LayerUnitTypes::Enum leftUnits;
    float m_width;
    LayerUnitTypes::Enum widthUnits;
    float m_right;
    LayerUnitTypes::Enum rightUnits;

    VerticalFieldValues::Enum verticalFieldValues;
    float m_top;
    LayerUnitTypes::Enum topUnits;
    float m_height;
    LayerUnitTypes::Enum heightUnits;
    float m_bottom;
    LayerUnitTypes::Enum bottomUnits;

    // Ambient occlusion
    float aoStrength;
    float aoDistance;
    float aoSoftness;
    float aoBias;
    qint32 aoSamplerate;
    bool aoDither;

    // Direct occlusion
    float shadowStrength;
    float shadowDist;
    float shadowSoftness;
    float shadowBias;

    // IBL
    QDemonRenderImage *lightProbe;
    float probeBright;
    bool fastIbl;
    float probeHorizon;
    float probeFov;
    QDemonRenderImage *lightProbe2;
    float probe2Fade;
    float probe2Window;
    float probe2Pos;

    bool temporalAAEnabled;

    QDemonRenderLayer();

    void addEffect(QDemonRenderEffect &inEffect);

    QDemonRenderEffect *getLastEffect();

    LayerBlendTypes::Enum getLayerBlend() { return blendType; }
};
QT_END_NAMESPACE

#endif
