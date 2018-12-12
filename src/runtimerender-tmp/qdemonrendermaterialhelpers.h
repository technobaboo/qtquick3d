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
#ifndef QDEMON_RENDER_MATERIAL_HELPERS_H
#define QDEMON_RENDER_MATERIAL_HELPERS_H
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>

QT_BEGIN_NAMESPACE

inline bool IsMaterial(SGraphObject &obj)
{
    return obj.m_Type == GraphObjectTypes::CustomMaterial
            || obj.m_Type == GraphObjectTypes::DefaultMaterial
            || obj.m_Type == GraphObjectTypes::ReferencedMaterial;
}

inline bool IsMaterial(SGraphObject *obj)
{
    if (obj)
        return IsMaterial(*obj);
    return false;
}

inline bool IsImage(SGraphObject &obj) { return obj.m_Type == GraphObjectTypes::Image; }

inline bool IsImage(SGraphObject *obj)
{
    if (obj)
        return IsImage(*obj);
    return false;
}

inline SGraphObject *GetNextMaterialSibling(SGraphObject *obj)
{
    if (obj == nullptr)
        return nullptr;
    if (IsMaterial(obj) == false) {
        Q_ASSERT(false);
        return nullptr;
    }
    if (obj->m_Type == GraphObjectTypes::CustomMaterial)
        return static_cast<SCustomMaterial *>(obj)->m_NextSibling;
    else if (obj->m_Type == GraphObjectTypes::DefaultMaterial)
        return static_cast<SDefaultMaterial *>(obj)->m_NextSibling;
    else
        return static_cast<SReferencedMaterial *>(obj)->m_NextSibling;
}

inline void SetNextMaterialSibling(SGraphObject &obj, SGraphObject *sibling)
{
    if (IsMaterial(obj) == false) {
        Q_ASSERT(false);
        return;
    }
    if (obj.m_Type == GraphObjectTypes::CustomMaterial)
        static_cast<SCustomMaterial *>(&obj)->m_NextSibling = sibling;
    else if (obj.m_Type == GraphObjectTypes::DefaultMaterial)
        static_cast<SDefaultMaterial *>(&obj)->m_NextSibling = sibling;
    else
        static_cast<SReferencedMaterial *>(&obj)->m_NextSibling = sibling;
}

QT_END_NAMESPACE

#endif
