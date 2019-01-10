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
#ifndef QDEMON_RENDER_LIGHT_H
#define QDEMON_RENDER_LIGHT_H
#include <QtDemonRuntimeRender/qdemonrendernode.h>

QT_BEGIN_NAMESPACE

struct RenderLightTypes
{
    enum Enum {
        Unknown = 0,
        Directional,
        Point,
        Area,
    };
};

struct SImage;

struct Q_DEMONRUNTIMERENDER_EXPORT SLight : public SNode
{
    RenderLightTypes::Enum m_LightType; // Directional
    SNode *m_Scope;
    QVector3D m_DiffuseColor; // colors are 0-1 normalized
    QVector3D m_SpecularColor; // colors are 0-1 normalized
    QVector3D m_AmbientColor; // colors are 0-1 normalized

    // The variables below are in the same range as Studio
    // Only valid if node is a point light
    float m_Brightness; // 0-200
    float m_LinearFade; // 0-200
    float m_ExponentialFade; // 0-200

    float m_AreaWidth; // 0.01-inf
    float m_AreaHeight; // 0.01-inf

    bool m_CastShadow; // true if this light produce shadows
    float m_ShadowBias; // depth shift to avoid self-shadowing artifacts
    float m_ShadowFactor; // Darkening factor for ESMs
    quint32 m_ShadowMapRes; // Resolution of shadow map
    float m_ShadowMapFar; // Far clip plane for the shadow map
    float m_ShadowMapFov; // Field of View for the shadow map
    float m_ShadowFilter; // Shadow map filter step size

    // Defaults to directional light
    SLight();

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SNode::Remap(inRemapper);
        inRemapper.Remap(m_Scope);
    }
};
QT_END_NAMESPACE

#endif
