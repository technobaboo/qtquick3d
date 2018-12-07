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
#ifndef QDEMON_RENDER_EFFECT_H
#define QDEMON_RENDER_EFFECT_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobject.h>

QT_BEGIN_NAMESPACE
struct SLayer;
struct SEffectContext;

// Effects are post-render effect applied to the layer.  There can be more than one of
// them and they have completely variable properties.
// see IEffectManager in order to create these effects.
// The data for the effect immediately follows the effect
struct SEffect : public SDynamicObject
{
private:
    // These objects are only created via the dynamic object system.
    SEffect(const SEffect &);
    SEffect &operator=(const SEffect &);
    SEffect();

public:
    SLayer *m_Layer;
    SEffect *m_NextEffect;
    // Opaque pointer to context type implemented by the effect system.
    // May be null in which case the effect system will generate a new context
    // the first time it needs to render this effect.
    SEffectContext *m_Context;

    void Initialize();

    // If our active flag value changes, then we ask the effect manager
    // to reset our context.
    void SetActive(bool inActive, IEffectSystem &inSystem);

    void Reset(IEffectSystem &inSystem);

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SDynamicObject::Remap(inRemapper);
        inRemapper.Remap(m_Layer);
        inRemapper.Remap(m_NextEffect);
        inRemapper.NullPtr(m_Context);
    }
};
QT_END_NAMESPACE

#endif
